#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <math.h>


#include "common.h"
#include "si3050_api.h"
#include "pcm.h"
#include "gpio-spi.h"



///////////////////////////////////////////////////////////////////
//  PCM INTERFACE
///////////////////////////////////////////////////////////////////

unsigned short buffer[BUFFER_SIZE];


void si3050_generate_sine(int freq, int volume)
{
    int i = 0;
    for (i = 0; i < BUFFER_SIZE / CHANNEL; i++) {
        double x = i * 2 * 3.1415926 / (SAMPLE_RATE / freq);
        
        unsigned short data = (unsigned short) (volume * sin(x) + 32768);
        if (CHANNEL == 1) {
            buffer[i] = data;
        } else {
            buffer[2 * i] = data;
            buffer[2 * i + 1] = data;
        }
    }
}

void si3050_pcm_init_config(struct pcm_config* config)
{
    unsigned int channels = 2;
    unsigned int rate = SAMPLE_RATE;
    //unsigned int bits = 16;
    unsigned int period_size = 128;
    unsigned int period_count = 2;
    
    config->channels = channels;
    config->rate = rate;
    config->period_size = period_size;
    config->period_count = period_count;
    config->format = PCM_FORMAT_S16_LE;
    config->start_threshold = 0;
    config->stop_threshold = 0;
    config->silence_threshold = 0;   
}

struct pcm* si3050_get_pcm_out(void)
{
    unsigned int card = 0;
    unsigned int device = 0;
    
    struct pcm_config config;
    struct pcm *pcm_out;

    si3050_pcm_init_config(&config);
    
    pcm_out = pcm_open(card, device, PCM_OUT, &config);
   	if (!pcm_out || !pcm_is_ready(pcm_out)) {
        printf("Unable to open PCM device %u (%s)\n",
                device, pcm_get_error(pcm_out));
        return NULL;
    }
    
    return pcm_out;
}

void si3050_close_pcm_out(starphone_server *sps)
{
    close(sps->si3050_pcm_out->fd);
    free(sps->si3050_pcm_out);
}

struct pcm* si3050_get_pcm_in(void)
{
    unsigned int card = 0;
    unsigned int device = 0;
    
    struct pcm_config config;
    struct pcm *pcm_in;

    si3050_pcm_init_config(&config);
    
    pcm_in = pcm_open(card, device, PCM_IN, &config);
   	if (!pcm_in || !pcm_is_ready(pcm_in)) {
        printf("Unable to open PCM device %u (%s)\n",
                device, pcm_get_error(pcm_in));
        return NULL;
    }
    
    return pcm_in;
}
void si3050_close_pcm_in(starphone_server *sps)
{
    close(sps->si3050_pcm_in->fd);
    free(sps->si3050_pcm_in);
}

int si3050_play_sine(void)
{
    struct pcm *pcm_out = si3050_get_pcm_out();
       
    si3050_generate_sine(2000, 10000);
    
    while (1) {
        if (!pcm_write(pcm_out, buffer, BUFFER_SIZE)) {
            //printf("sucess in to out: %d\n", BUFFER_SIZE);	
        } else {
            printf("failed\n");
            break;
        }
    }
    
    return 0;
}

void si3050_pcm_loopback(void)
{
    struct pcm *pcm_out = si3050_get_pcm_out();
    struct pcm *pcm_in = si3050_get_pcm_in();
    unsigned char buff[128];
    
    if (!pcm_out || !pcm_in)
        return;
    
    printf("buffer size:%d %d\n", pcm_get_buffer_size(pcm_out), pcm_get_buffer_size(pcm_in));
    
    while(1) {
        int ret = pcm_read(pcm_in, buff, sizeof(buff)); 
        if (!ret) {
                ret = pcm_write(pcm_out, buff, sizeof(buff));
       		if (!ret) {
                        continue;
       			//printf("sucess in to out: %d\n", sizeof(buff));	
       		} else {
       		        printf("pcm_write return value: %d\n",ret);
       			break;
       		}
       	} else {
       	    printf("pcm_read return value: %d\n",ret);
            break;
        }
    }
    
    printf("exit!\n");
}

///////////////////////////////////////////////////////////////////
// SPI CONTROL
///////////////////////////////////////////////////////////////////
void si3050_get_ver_info(void)
{
        unsigned char sys_ver_val = 0x00;
        unsigned char line_ver_val = 0x00;
        
        _DEBUG("Get si3050 infomation Start ...");        
        
        sys_ver_val = gpio_spi_read(11);        
        //line_ver_val = gpio_spi_read(13);
        
        _DEBUG("\n");
        _DEBUG("Read reg[11] Value = 0x%0x",sys_ver_val);
        _DEBUG("Read reg[13] Value = 0x%0x",line_ver_val);
        
        switch((sys_ver_val & 0xF0) >> 4)
        {
                case 0x01:
                         _DEBUG("Line-Side ID[si3018]: 0x01");
                        break;
                case 0x03:
                        _DEBUG("Line-Side ID[si3019]: 0x03");
                        break;
                case 0x04:
                        _DEBUG("Line-Side ID[si3011]: 0x04");
                        break;
                default:
                        _DEBUG("Line-Side ID[Unknown]: 0x%0x ",(sys_ver_val & 0xF0) >> 4); 
                        break;
        }
        
        _DEBUG("System-Side Revision: 0x%0x",(sys_ver_val & 0x0F));       
        _DEBUG("Line-Side Device Revision: 0x%0x",((line_ver_val & 0x3C) >> 2));
        _DEBUG("\n");

        _DEBUG("REG[2] Default = 0x03, Read Value = 0x%0x", gpio_spi_read(2)); //0x0000_0011
        gpio_spi_write(2, 0xc5);
        _DEBUG("REG[2] Write Value = 0xc5, Read Value = 0x%0x", gpio_spi_read(2)); //0x0000_0011
        gpio_spi_write(2, 0xc3);
}

void si3050_pcm_dev_drv_init(starphone_server *sps)
{
    sps->si3050_pcm_out = si3050_get_pcm_out();
    sps->si3050_pcm_in = si3050_get_pcm_in();
    
    if (!sps->si3050_pcm_out || !sps->si3050_pcm_in)
        return;
    
    printf("buffer size:%d %d\n", 
        pcm_get_buffer_size(sps->si3050_pcm_out), 
        pcm_get_buffer_size(sps->si3050_pcm_in));    

}

void si3050_sw_reset(starphone_server *sps)
{
    unsigned char regCfg = 0;

    
    //Enable si3050 PCM interface 
    regCfg = gpio_spi_read(33);
    regCfg |= (0x1 << 3) | (0x1 << 5); // Enable PCM & u-Law
    regCfg &= ~(0x1 << 4);
    gpio_spi_write(33, regCfg);

    //Specific county Seting for Taiwan
    regCfg = gpio_spi_read(16);
    regCfg &= ~((0x1 << 0) & (0x1 << 1) & (0x1 << 4) &  (0x1 << 6)); // OHS RZ RT
    gpio_spi_write(16, regCfg);

    regCfg = gpio_spi_read(26);
    regCfg |= (0x1 << 6) | (0x1 << 7); // DCV[1:0] = 11
    regCfg &= ~((0x1 << 1) & (0x1 << 4) & (0X1 << 5));
    gpio_spi_write(26, regCfg);

    regCfg = gpio_spi_read(30);
    regCfg &= ~((0x1 << 0) & (0x1 << 1) & (0x1 << 2) & (0x1 << 3) & (0x1 << 4));
    gpio_spi_write(30, regCfg);

    regCfg = gpio_spi_read(31);
    regCfg &= ~(0x1 << 3); // OHS2 = 0
    gpio_spi_write(31, regCfg);    

}


void si3050_hw_reset(void)
{
        usleep(1000);
        set_reset_pin_low(); // RESET
        usleep(50*1000);
        usleep(50*1000);
        //usleep(50*1000);
        //usleep(50*1000);
        //usleep(50*1000);
        //usleep(50*1000);      
        //sleep(1);
        set_reset_pin_high(); // RESET
        usleep(50*1000);
        usleep(50*1000);
        //usleep(50*1000);
        //usleep(50*1000);
        //usleep(50*1000);
        //usleep(50*1000);
        //usleep(50*1000);
        //usleep(50*1000);
        //usleep(50*1000);
        //usleep(50*1000);
}


void si3050_power_up_si3019(void)
{
        gpio_spi_write(6, 0x00);
}

void si3050_sys_init(void)
{
    //unsigned char regCfg = 0;

     
    si3050_hw_reset(); //reset for power up 
    _DEBUG("Reset si3050 complete...");

    
    
    // Check the Version to make sure SPI Conmunication is OK
    si3050_get_ver_info();

    
    si3050_power_up_si3019();

    return ;
}


