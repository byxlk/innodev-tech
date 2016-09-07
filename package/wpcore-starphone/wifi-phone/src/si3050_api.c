#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>

#include "si3050_api.h"
#include "gpio-spi.h"
#include "common.h"


void si3050_get_ver_info(void)
{
        unsigned char sys_ver_val = 0x00;
        unsigned char line_ver_val = 0x00;
        
        _DEBUG("Get si3050 infomation Start ...");        
        
        sys_ver_val = gpio_spi_read(11);        
        line_ver_val = gpio_spi_read(13);
        
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
        
}

void si3050_hw_reset(void)
{
        usleep(50*1000);
        //set_gpio_value(GPIO_RESET_VAL, GPIO_VAL_LOW); // RESET
        usleep(50*1000);
        usleep(50*1000);
        //usleep(50*1000);
        //usleep(50*1000);
        //usleep(50*1000);
        //usleep(50*1000);      
        //sleep(1);
        //set_gpio_value(GPIO_RESET_VAL, GPIO_VAL_HIGH); // RESET
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


void si3050_sys_init(void)
{

    //初始化gpio - SPI初始方向和值
    _DEBUG("Config gpio direction and value start...");
    set_gpio_direction(GPIO_INT_DIR, GPIO_DIR_INPUT); // INT
    set_gpio_direction(GPIO_RESET_DIR, GPIO_DIR_OUTPUT); //RESET 
    set_gpio_direction(GPIO_SPI_CLK_DIR, GPIO_DIR_OUTPUT); // CLK
    set_gpio_direction(GPIO_SPI_CS_DIR, GPIO_DIR_OUTPUT); // CS
    set_gpio_direction(GPIO_SPI_SDI_DIR, GPIO_DIR_OUTPUT); // SDI
    set_gpio_direction(GPIO_SPI_SDO_DIR, GPIO_DIR_INPUT); // SDO 
    _DEBUG("Config gpio direction complete...");
    
    set_gpio_value(GPIO_SPI_CLK_VAL, GPIO_VAL_HIGH); // CLK
    set_gpio_value(GPIO_SPI_CS_VAL, GPIO_VAL_HIGH); // CS
    set_gpio_value(GPIO_SPI_SDI_VAL, GPIO_VAL_HIGH); // SDI     
    set_gpio_value(GPIO_RESET_VAL, GPIO_VAL_HIGH); // RESET
    _DEBUG("Config gpio value complete...");
    
    //si3050_hw_reset(); //reset for power up 
    _DEBUG("Reset si3050 complete...");
    
    si3050_get_ver_info();
    
    return ;
}

