#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>

#include "gpio-spi.h"
#include "common.h"



int gpio_spi_write(unsigned char reg, unsigned char val)
{
      int gpio_fd = -1;
      struct si3050_reg regVal;

        gpio_fd = open(GPIO_SPI_DEVICE, O_WRONLY);
        if(gpio_fd < 0) 
        {
                _ERROR("open %s faild, return err value: %d",GPIO_SPI_DEVICE,gpio_fd);
                return -1;
        }

        regVal.addr = (unsigned int)reg;
        regVal.val = (unsigned int)val;
        if(ioctl(gpio_fd, IOCTL_GPIO_SPI_WRITE, &regVal) < 0)
        {
                _ERROR("IOCTL_GPIO_SPI_WRITE error reg=0x%0x val=0x%0x",regVal.addr,regVal.val);
                close(gpio_fd);
                return -1;
        }
        
        close(gpio_fd);
        return 0;
}
unsigned char gpio_spi_read(unsigned char reg)
{
        int gpio_fd = -1;
        struct si3050_reg regVal;
        
        gpio_fd = open(GPIO_SPI_DEVICE, O_RDONLY);
        if(gpio_fd < 0) 
        {
                _ERROR("open %s faild, return err value: %d",GPIO_SPI_DEVICE,gpio_fd);
                return -1;
        }

        regVal.addr = (unsigned int)reg;
        regVal.val = 0xFF;
        if(ioctl(gpio_fd, IOCTL_GPIO_SPI_READ, &regVal) < 0)
        {
                _ERROR("IOCTL_GPIO_SPI_READ error reg=0x%0x val=0x%0x",regVal.addr,regVal.val );
                close(gpio_fd);
                return -1;
        }

        close(gpio_fd);

        return ((unsigned char)regVal.val) ;
}

void set_reset_pin_high(void)
{
        int gpio_fd = -1;
        struct si3050_reg regVal;

        gpio_fd = open(GPIO_SPI_DEVICE, O_WRONLY);
        if(gpio_fd < 0) 
        {
                _ERROR("open %s faild, return err value: %d",GPIO_SPI_DEVICE,gpio_fd);
                return ;
        }

        regVal.addr = 0;
        regVal.val = 0;
        if(ioctl(gpio_fd, IOCTL_SET_RESET_PIN_HIGH, &regVal) < 0)
        {
                _ERROR("IOCTL_SET_RESET_PIN_HIGH error reg=0x%0x val=0x%0x",regVal.addr,regVal.val);
                close(gpio_fd);
                return ;
        }
        
        close(gpio_fd);
        return ;
}

void set_reset_pin_low(void)
{
        int gpio_fd = -1;
        struct si3050_reg regVal;

        gpio_fd = open(GPIO_SPI_DEVICE, O_WRONLY);
        if(gpio_fd < 0) 
        {
                _ERROR("open %s faild, return err value: %d",GPIO_SPI_DEVICE,gpio_fd);
                return ;
        }

        regVal.addr = 0;
        regVal.val = 0;
        if(ioctl(gpio_fd, IOCTL_SET_RESET_PIN_LOW, &regVal) < 0)
        {
                _ERROR("IOCTL_SET_RESET_PIN_LOW error reg=0x%0x val=0x%0x",regVal.addr,regVal.val);
                close(gpio_fd);
                return ;
        }
        
        close(gpio_fd);
        return ;
}

