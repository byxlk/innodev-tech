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


#include "include/xw_export.h"



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

void si3050_close_pcm_out(SPS_SYSTEM_INFO_T *sps)
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
void si3050_close_pcm_in(SPS_SYSTEM_INFO_T *sps)
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

void si3050_pcm_dev_drv_init(SPS_SYSTEM_INFO_T *sps)
{
    sps->si3050_pcm_out = si3050_get_pcm_out();
    sps->si3050_pcm_in = si3050_get_pcm_in();
    
    if (!sps->si3050_pcm_out || !sps->si3050_pcm_in)
        return;
    
    printf("buffer size:%d %d\n", 
        pcm_get_buffer_size(sps->si3050_pcm_out), 
        pcm_get_buffer_size(sps->si3050_pcm_in));    

}

void si3050_sw_reset(SPS_SYSTEM_INFO_T *sps)
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


void Si3050_Power_Up_Si3019(void)
{
        gpio_spi_write(6, 0x00);
}

/*------------------------------------------------------------------------------*/
/* FUNCTION:    si3050_hw_init                                                  */
/*------------------------------------------------------------------------------*/
/* DESCRIPTION:    Perform hardware initialization of TID analog interface.     */
/*------------------------------------------------------------------------------*/

bool Si3050_Hw_Init(unsigned short timeslot)
{

	unsigned char device_id;
	//int i,j;

	/* Initialize and power up DAA */
	if(Si3050_Hw_Reset() == FALSE)
	{
		return FALSE;
	}


	device_id = gpio_spi_read(SI3050_REG_CHIP_A_REV);//reg11

	if((device_id>>4) > 3)
	{
		device_id &=0xF;
		//j=sprintf(str,"\nSI3050 dev id error %x on tcid %u\n",device_id,tcid);
		//ser_Write(str,j);
		return FALSE;
	}


	/* This is a simple method of verifying if the deivce is alive */
	if(device_id == 0)
	{
		//j=sprintf(str,"\nDetected SI3050 revision %u and a %s on tcid %u\n", ((device_id)&0xF), line_device[(device_id>>4)&0xF],tcid);
		//ser_Write(str,j);
		return FALSE;
	}

	/* enable PCM and assign timeslot for PCM */

	si3050_pcm_init(timeslot);

	/* Enable intterupt */
	gpio_spi_write(SI3050_REG_CONTROL2, 0x83); //0x87
	//j=sprintf(str,"\nFXO_ProSLIC device %d initialized", tcid);
	//ser_Write(str,j);

	return TRUE;
}

/*******************************************************************************
* FUNCTION:     si3050_hw_reset
*
********************************************************************************
* DESCRIPTION:    Power up the device
*
*******************************************************************************/

bool Si3050_Hw_Reset(void)
{
	unsigned char ac_termination_data;
	unsigned char dc_termination_data;
	unsigned char international_control1_data;
	unsigned char daa_control5_data;
	unsigned char loop_cnt = 0;
	unsigned char data;
	unsigned char interrupt_mask;
	//char i,j;


	/* UINT8 intterupt_mask; */
	/* Enable link + power up line side device */
	gpio_spi_write(SI3050_REG_DAA_CONTROL2, 0x00);

	/* Read FTD */
	data=gpio_spi_read(SI3050_REG_LINE_STATUS);//reg12
	while((!(data & 0x40) | data==0xff)
		&& (loop_cnt < SI3050_MAX_FTD_RETRY))
	{
		wait(200);
		loop_cnt++;
		data=gpio_spi_read(SI3050_REG_LINE_STATUS);
	}
	if(loop_cnt >= SI3050_MAX_FTD_RETRY)
	{
		//j=sprintf(str,"\nFailed to get FTD register sync\n");
		//ser_Write(str,j);
		return(FALSE);
	}


		ac_termination_data = 0x00;
		dc_termination_data = 0xc0;
		international_control1_data = 0x00;
		daa_control5_data = 0x20;


	/* Set AC termination */
	gpio_spi_write(SI3050_REG_AC_TERMIATION_CONTROL, ac_termination_data);//reg30
	/* Set DC termination */
	gpio_spi_write(SI3050_REG_DC_TERMINATION_CONTROL, dc_termination_data);//reg26
	/* Set International Control: set ring impedance, ring detection threshold, on-hook speed , enable/disable iir filter */
	gpio_spi_write(SI3050_REG_INTERNATIONAL_CONTROL1, international_control1_data);//reg16 /* OHS IIRE RZ RT */
	/* Set off-hook speed , enable full scale */
	daa_control5_data |= 0x80 | 0x02;//0x80 | 0x02; /* Move the Pole of the built in filter from 5 Hz to 200 Hz this may affect voice quality */
	gpio_spi_write(SI3050_REG_DAA_CONTROL5, daa_control5_data);//reg31 /* OHS IIRE RZ RT */

	/* Enable Ring Validation */
	/* fmin 10Hz */
	/* fmax 83Hz */
	/* ring confirmation count 150ms */
	/* ring time out 640ms */
	/* ring delay 0ms */
	gpio_spi_write(SI3050_REG_RING_VALIDATION_CONTROL1, 0x16);//reg22 //0x56
	gpio_spi_write(SI3050_REG_RING_VALIDATION_CONTROL2, 0x29);//reg23//0x2b
	gpio_spi_write(SI3050_REG_RING_VALIDATION_CONTROL3, 0x99);//reg24

	/* Set interrupt mask */
	/* interrupt_mask = SI3050_RDT_INT | SI3050_ROV_INT | SI3050_FDT_INT | SI3050_BTD_INT | SI3050_DOD_INT |SI3050_LCSO_INT | SI3050_TGD_INT | SI3050_POL_INT; */
	/* si3050_write_reg(SI3050_REG_INTERRUPT_MASK, interrupt_register, tcid); */
	/* Clear intterupt register */
	/* si3050_write_reg(SI3050_REG_INTERRUPT_SOURCE, 0x00, tcid); */

	//si3050_write_reg(43 ,0x03, tcid);
	//si3050_write_reg(44 ,0x07, tcid);

	interrupt_mask = SI3050_RDT_INT | SI3050_ROV_INT | SI3050_FDT_INT | SI3050_BTD_INT | SI3050_DOD_INT |SI3050_LCSO_INT | SI3050_TGD_INT | SI3050_POL_INT;
	interrupt_mask=0x80;
	gpio_spi_write(SI3050_REG_INTERRUPT_MASK, interrupt_mask);
	/* Clear intterupt register */
	gpio_spi_write(SI3050_REG_INTERRUPT_SOURCE, 0x00);



	/* Set international control registers */
	gpio_spi_write(SI3050_REG_INTERNATIONAL_CONTROL2, 0x00); //reg17 /* RT2 OPE BTE ROV BTD */
	gpio_spi_write(SI3050_REG_INTERNATIONAL_CONTROL3, 0x00); //reg18 /* RFWE */
	gpio_spi_write(SI3050_REG_INTERNATIONAL_CONTROL4, 0x00); //reg19 /* OVL DOD OPE */

	return(TRUE);
}

/*******************************************************************************
* FUNCTION:     pcm_init
*
********************************************************************************
* DESCRIPTION:    Perform hardware initialization of PCM interface.
*
*******************************************************************************/
void Si3050_Pcm_Init(unsigned short timeslot)
{
    unsigned short  pcm_offset;
    unsigned char   pcm_mode;


    pcm_offset = (timeslot)*8;
    pcm_mode = SI3050_PCM_ENABLE ;//| SI3050_PCM_TRI ;//| 1<<1;//PHCF

	gpio_spi_write(SI3050_REG_PCM_TX_LOW, (pcm_offset & 0x0ff));
    gpio_spi_write(SI3050_REG_PCM_TX_HIGH, (pcm_offset >> 8) & 0x0003);
    gpio_spi_write(SI3050_REG_PCM_RX_LOW, pcm_offset & 0x0ff);
    gpio_spi_write(SI3050_REG_PCM_RX_HIGH, (pcm_offset >> 8) & 0x0003);
    gpio_spi_write(SI3050_REG_PCM_SPI_MODE_SELECT, pcm_mode);//reg33

}




/*******************************************************************************
* FUNCTION:     si3050_set_hook
*
********************************************************************************
* DESCRIPTION:    Toggle the hook switch
*
*******************************************************************************/
void Si3050_Set_Hook(bool si3050_off_hook)
{
    unsigned char data = 0;
    if(si3050_off_hook)
    {
        data = 0x1;
	}
	gpio_spi_write(SI3050_REG_DAA_CONTROL1, data);
}

/*******************************************************************************
* FUNCTION:     si3050_hw_gain_control
*
********************************************************************************
* DESCRIPTION:    Modify the DAA gain - note, not used for normal operation.
*
*******************************************************************************/
void Si3050_Hw_Gain_Control(unsigned char gain_value_high, 
							unsigned char gain_value_low, bool f_is_tx_gain)
{
	if(f_is_tx_gain)
	{
		gpio_spi_write(SI3050_REG_TX_GAIN_CONTROL2, gain_value_high);
		gpio_spi_write(SI3050_REG_TX_GAIN_CONTROL3, gain_value_low);
	}
	else
	{
		gpio_spi_write(SI3050_REG_RX_GAIN_CONTROL2, gain_value_high);//r39
		gpio_spi_write(SI3050_REG_RX_GAIN_CONTROL3, gain_value_low);//r41
		/*
		reg 39
		7:5 Reserved Read returns zero.
		4 RGA2 Receive Gain or Attenuation 2.
		0 = Incrementing the RXG2[3:0] bits results in gaining up the receive path.
		1 = Incrementing the RXG2[3:0] bits results in attenuating the receive path.
		3:0 RXG2[3:0] Receive Gain 2.
		Each bit increment represents 1 dB of gain or attenuation, up to a maximum of +12 dB and
		?5 dB respectively.
		For example:
		RGA2 RXG2[3:0] Result
		X 0000 0 dB gain or attenuation is applied to the receive path.
		0 0001 1 dB gain is applied to the receive path.
		0 :
		0 11xx 12 dB gain is applied to the receive path.
		1 0001 1 dB attenuation is applied to the receive path.
		1 :
		1 1111 15 dB attenuation is applied to the receive path.
		*/

		/*
		reg 41
				7:5 Reserved Read returns zero.
				4 RGA3 Receive Gain or Attenuation 2.
				0 = Incrementing the RXG3[3:0] bits results in gaining up the receive path.
				1 = Incrementing the RXG3[3:0] bits results in attenuating the receive path.
				3:0 RXG3[3:0] Receive Gain 3.
				Each bit increment represents 0.1 dB of gain or attenuation, up to a maximum of 1.5 dB.
				For example:
				RGA3 RXG3[3:0] Result
				X 0000 0 dB gain or attenuation is applied to the receive path.
				0 0001 0.1 dB gain is applied to the receive path.
				0 :
				0 1111 1.5 dB gain is applied to the receive path.
				1 0001 0.1 dB attenuation is applied to the receive path.
				1 :
				1 1111 1.5 dB attenuation is applied to the receive path.
		*/
	}
}


/*******************************************************************************
* FUNCTION:     si3050_set_lowpwr_path
*
********************************************************************************
* DESCRIPTION:  set data path on onhook line monitor state to transfer CID
*
*******************************************************************************/
void Si3050_Set_Lowpwr_Path(void)
{
	gpio_spi_write(SI3050_REG_DAA_CONTROL1, 0x08);
}

/*******************************************************************************
* FUNCTION:     si3050_clear_lowpwr_path
*
********************************************************************************
* DESCRIPTION:  disable low power data path on onhook state
*
*******************************************************************************/
void Si3050_Clear_Lowpwr_Path(void)
{
	   gpio_spi_write(SI3050_REG_DAA_CONTROL1, 0x00);
}

void Si3050_DAA_System_Init(void)
{
    //unsigned char regCfg = 0;

    SPS_SYSTEM_INFO_T *DTSystemInfo = XW_Global_InitSystemInfo();
    //si3050_hw_reset(); //reset for power up 
    _DEBUG("Reset si3050 complete...");

    
    
    // Check the Version to make sure SPI Conmunication is OK
    si3050_get_ver_info();

    
    //Si3050_Power_Up_Si3019();

    return ;
}

void *XW_Pthread_ModemCtrlDeamon(void *args)
{
        SPS_SYSTEM_INFO_T *DTSystemInfo = XW_Global_InitSystemInfo();

        while(1)
        {

                sleep(10);
        }

        return 0;
} 

