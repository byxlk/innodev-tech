#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>

#include "include/xw_export.h"

#if USING_GPIO_SPI_DRIVER
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

void set_spiclk_pin_high(void)
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
        if(ioctl(gpio_fd, IOCTL_SET_SPICLK_PIN_HIGH, &regVal) < 0)
        {
                _ERROR("IOCTL_SET_RESET_PIN_HIGH error reg=0x%0x val=0x%0x",regVal.addr,regVal.val);
                close(gpio_fd);
                return ;
        }
        
        close(gpio_fd);
        return ;
}

void set_spiclk_pin_low(void)
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
        if(ioctl(gpio_fd, IOCTL_SET_SPICLK_PIN_LOW, &regVal) < 0)
        {
                _ERROR("IOCTL_SET_RESET_PIN_LOW error reg=0x%0x val=0x%0x",regVal.addr,regVal.val);
                close(gpio_fd);
                return ;
        }
        
        close(gpio_fd);
        return ;
}

void set_spics_pin_high(void)
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
        if(ioctl(gpio_fd, IOCTL_SET_SPICS_PIN_HIGH, &regVal) < 0)
        {
                _ERROR("IOCTL_SET_RESET_PIN_HIGH error reg=0x%0x val=0x%0x",regVal.addr,regVal.val);
                close(gpio_fd);
                return ;
        }
        
        close(gpio_fd);
        return ;
}

void set_spics_pin_low(void)
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
        if(ioctl(gpio_fd, IOCTL_SET_SPICS_PIN_LOW, &regVal) < 0)
        {
                _ERROR("IOCTL_SET_RESET_PIN_LOW error reg=0x%0x val=0x%0x",regVal.addr,regVal.val);
                close(gpio_fd);
                return ;
        }
        
        close(gpio_fd);
        return ;
}

void set_spisdi_pin_high(void)
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
        if(ioctl(gpio_fd, IOCTL_SET_SPISDI_PIN_HIGH, &regVal) < 0)
        {
                _ERROR("IOCTL_SET_RESET_PIN_HIGH error reg=0x%0x val=0x%0x",regVal.addr,regVal.val);
                close(gpio_fd);
                return ;
        }
        
        close(gpio_fd);
        return ;
}

void set_spisdi_pin_low(void)
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
        if(ioctl(gpio_fd, IOCTL_SET_SPISDI_PIN_LOW, &regVal) < 0)
        {
                _ERROR("IOCTL_SET_RESET_PIN_LOW error reg=0x%0x val=0x%0x",regVal.addr,regVal.val);
                close(gpio_fd);
                return ;
        }
        
        close(gpio_fd);
        return ;
}
void gpio_spi_port_dir_init(void)
{
    return ;
}
#else
int set_gpio_direction(const char *gpio_port, const char *gpio_dir)
{
        int gpio_fd = -1;

        gpio_fd = open(gpio_port, O_WRONLY);
        if(gpio_fd < 0)
        {
                _ERROR("open %s faild, return err value: %d",gpio_port, gpio_fd);
                return -1;
        }

        if(write(gpio_fd,   gpio_dir,  sizeof(gpio_dir)) < 0)
        {
                _ERROR("set [%s] as [%s] mode faild",gpio_port,gpio_dir);
                return -1;
        }

        close(gpio_fd);

        return 0;
}

int set_gpio_value(const char *gpio_port, const char *gpio_val)
{
        int gpio_fd = -1;

        gpio_fd = open(gpio_port, O_WRONLY);
        if(gpio_fd < 0)
        {
                _ERROR("open %s faild, return err value: %d",gpio_port,gpio_fd);
                return -1;
        }
        //_DEBUG("open gpio port ok");
        if(write(gpio_fd,   gpio_val,  sizeof(gpio_val)) < 0)
        {
                _ERROR("set port [%s] value as [%s] faild",gpio_port, gpio_val);
                return -1;
        }
        //_DEBUG("write gpio port ok");
        
        close(gpio_fd);
    return 0;
}

int get_gpio_value(const char *gpio_port)
{
        int gpio_fd = -1;
        int read_size = 0;
        char gpio_val[3] ;
        
        //_DEBUG("get gpio [%s] value start",*gpio_port);
        gpio_fd = open(gpio_port, O_RDONLY);
        if(gpio_fd < 0) 
        {
                _ERROR("open %s faild, return err value: %d",gpio_port,gpio_fd);
                return -1;
        }
        
        //_DEBUG("open gpio ok, start read value ...");
         if((read_size = read(gpio_fd,   gpio_val,  3)) < 0)
        {
                _ERROR("read value from port [%s] faild",gpio_port);
                return -1;
        }
        //_DEBUG("Read size: %d content: %c ",read_size, gpio_val[0]);

        close(gpio_fd);

        if(gpio_val[0] == '0')
                return 0;
        else if(gpio_val[0] == '1')
                return 1;
        else
        {
                _ERROR("Read gpio[%s] value error",gpio_port);
                return 1;
        }
        //return (atoi(gpio_val[0]));
}

void gpio_spi_port_dir_init(void)
{
    set_gpio_direction(GPIO_INT_DIR, GPIO_DIR_INPUT); // INT
    set_gpio_direction(GPIO_RESET_DIR, GPIO_DIR_OUTPUT); //RESET
    set_gpio_direction(GPIO_SPI_CLK_DIR, GPIO_DIR_OUTPUT); // CLK
    set_gpio_direction(GPIO_SPI_CS_DIR, GPIO_DIR_OUTPUT); // CS
    set_gpio_direction(GPIO_SPI_SDI_DIR, GPIO_DIR_OUTPUT); // SDI
    set_gpio_direction(GPIO_SPI_SDO_DIR, GPIO_DIR_INPUT); // SDO

        set_gpio_value(GPIO_SPI_CLK_VAL, GPIO_VAL_HIGH); // CLK
        set_gpio_value(GPIO_SPI_CS_VAL, GPIO_VAL_HIGH); // CS
        set_gpio_value(GPIO_SPI_SDI_VAL, GPIO_VAL_HIGH); // SDI
        set_gpio_value(GPIO_RESET_VAL, GPIO_VAL_HIGH); // RESET
}

unsigned char gpio_spi_read(unsigned char reg)
{
        int i = 0;
        unsigned char regVal = 0x00;
        
        //_DEBUG("step 1: send 0x60");
        
        // first: send 0x60
        usleep(5*1000);
        set_gpio_value(GPIO_SPI_CS_VAL, GPIO_VAL_LOW); // CS       
        usleep(100);
        for(i =0; i < 8; i++)
        {
                set_gpio_value(GPIO_SPI_CLK_VAL, GPIO_VAL_LOW); // CLK
                usleep(100);
                set_gpio_value(GPIO_SPI_SDI_VAL, GPIO_BIT((7-i), 0x60)); 
                usleep(9*100);
                set_gpio_value(GPIO_SPI_CLK_VAL, GPIO_VAL_HIGH); // CLK
                usleep(1*1000);
        }
        usleep(100);
        set_gpio_value(GPIO_SPI_CS_VAL, GPIO_VAL_HIGH); // CS
        usleep(5*1000);

         //_DEBUG("step 2: send address 0x%x",reg);
         
        //second: send address
        set_gpio_value(GPIO_SPI_CS_VAL, GPIO_VAL_LOW); // CS
        usleep(100);
        for(i =0; i < 8; i++)
        {
                set_gpio_value(GPIO_SPI_CLK_VAL, GPIO_VAL_LOW); // CLK
                usleep(100);
                set_gpio_value(GPIO_SPI_SDI_VAL, GPIO_BIT((7-i), reg)); 
                usleep(9*100);
                set_gpio_value(GPIO_SPI_CLK_VAL, GPIO_VAL_HIGH); // CLK
                usleep(1*1000);
        }
        usleep(100);
        set_gpio_value(GPIO_SPI_CS_VAL, GPIO_VAL_HIGH); // CS
        usleep(5*1000);
        
        //_DEBUG("step 3: read data");
        
        set_gpio_value(GPIO_SPI_CS_VAL, GPIO_VAL_LOW); // CS
        usleep(100);
        for(i =0; i < 8; i++)
        {
                set_gpio_value(GPIO_SPI_CLK_VAL, GPIO_VAL_LOW); // CLK
                usleep(100);
                regVal |= ((get_gpio_value(GPIO_SPI_SDO_VAL) == 0)? 0x00 : (0x01 << (7-i))); 
                usleep(9*100);
                set_gpio_value(GPIO_SPI_CLK_VAL, GPIO_VAL_HIGH); // CLK
                usleep(1*1000);
        }
        usleep(100);
        set_gpio_value(GPIO_SPI_CS_VAL, GPIO_VAL_HIGH); // CS
        usleep(5*1000);
        
        return regVal;
}

int gpio_spi_write(unsigned char reg, unsigned char val)
{
        int i = 0;
        
        // first: send 0x20
        usleep(5*1000);
        set_gpio_value(GPIO_SPI_CS_VAL, GPIO_VAL_LOW); // CS
        usleep(100);
        for(i =0; i < 8; i++)
        {
                set_gpio_value(GPIO_SPI_CLK_VAL, GPIO_VAL_LOW); // CLK
                usleep(100);
                set_gpio_value(GPIO_SPI_SDI_VAL, GPIO_BIT((7-i), 0x20)); 
                usleep(9*100);
                set_gpio_value(GPIO_SPI_CLK_VAL, GPIO_VAL_HIGH); // CLK
                usleep(1*1000);
        }
        usleep(100);
        set_gpio_value(GPIO_SPI_CS_VAL, GPIO_VAL_HIGH); // CS
        usleep(5*1000);

        //second: send address
        set_gpio_value(GPIO_SPI_CS_VAL, GPIO_VAL_LOW); // CS
        usleep(100);
        for(i =0; i < 8; i++)
        {
                set_gpio_value(GPIO_SPI_CLK_VAL, GPIO_VAL_LOW); // CLK
                usleep(100);
                set_gpio_value(GPIO_SPI_SDI_VAL, GPIO_BIT((7-i), reg)); 
                usleep(9*100);
                set_gpio_value(GPIO_SPI_CLK_VAL, GPIO_VAL_HIGH); // CLK
                usleep(1*1000);
        }
        usleep(100);
        set_gpio_value(GPIO_SPI_CS_VAL, GPIO_VAL_HIGH); // CS
        usleep(5*1000);

        set_gpio_value(GPIO_SPI_CS_VAL, GPIO_VAL_LOW); // CS
        usleep(100);
        for(i =0; i < 8; i++)
        {
                set_gpio_value(GPIO_SPI_CLK_VAL, GPIO_VAL_LOW); // CLK
                usleep(100);
                set_gpio_value(GPIO_SPI_SDI_VAL, GPIO_BIT((7-i), val)); 
                usleep(9*100);
                set_gpio_value(GPIO_SPI_CLK_VAL, GPIO_VAL_HIGH); // CLK
                usleep(1*1000);
        }
        usleep(100);
        set_gpio_value(GPIO_SPI_CS_VAL, GPIO_VAL_HIGH); // CS
        usleep(5*1000);

        return 0;
        
}


void set_reset_pin_high(void)
{
    set_gpio_value(GPIO_RESET_VAL, GPIO_VAL_HIGH); // CLK
}
void set_reset_pin_low(void)
{
    set_gpio_value(GPIO_RESET_VAL, GPIO_VAL_LOW); // CLK
}
void set_spiclk_pin_high(void)
{
     set_gpio_value(GPIO_SPI_CLK_VAL, GPIO_VAL_HIGH); // CLK
}

void set_spiclk_pin_low(void)
{
    set_gpio_value(GPIO_SPI_CLK_VAL, GPIO_VAL_LOW); // CLK
}

void set_spics_pin_high(void)
{
        set_gpio_value(GPIO_SPI_CS_VAL, GPIO_VAL_HIGH); // CS        
}
void set_spics_pin_low(void)
{
        set_gpio_value(GPIO_SPI_CS_VAL, GPIO_VAL_LOW); // CS
}
void set_spisdi_pin_high(void)
{
    set_gpio_value(GPIO_SPI_SDI_VAL, GPIO_VAL_HIGH); 
}
void set_spisdi_pin_low(void)
{
    set_gpio_value(GPIO_SPI_SDI_VAL, GPIO_VAL_LOW); 
}

#endif

