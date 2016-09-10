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


inline int set_gpio_direction(const char *gpio_port, const char *gpio_dir)
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

inline int set_gpio_value(const char *gpio_port, const char *gpio_val)
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

inline int get_gpio_value(const char *gpio_port)
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
        _DEBUG("Read size: %d content: %c ",read_size, gpio_val[0]);

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

unsigned char gpio_spi_read(unsigned char reg)
{
        int i = 0;
        unsigned char regVal = 0x00;
        
        //_DEBUG("step 1: send 0x60");
        
        // first: send 0x60
        usleep(DAT_DELAY);
        set_gpio_value(GPIO_SPI_CS_VAL, GPIO_VAL_LOW); // CS       
        usleep(CS_DELAY);
        for(i =0; i < 8; i++)
        {
                set_gpio_value(GPIO_SPI_CLK_VAL, GPIO_VAL_LOW); // CLK
                usleep(CLK_L_DELAY);
                set_gpio_value(GPIO_SPI_SDI_VAL, GPIO_BIT((7-i), 0x60)); 
                usleep(VAL_DELAY);
                set_gpio_value(GPIO_SPI_CLK_VAL, GPIO_VAL_HIGH); // CLK
                usleep(CLK_H_DELAY);
        }
        usleep(CS_DELAY);
        set_gpio_value(GPIO_SPI_CS_VAL, GPIO_VAL_HIGH); // CS
        usleep(DAT_DELAY);

         //_DEBUG("step 2: send address 0x%x",reg);
         
        //second: send address
        set_gpio_value(GPIO_SPI_CS_VAL, GPIO_VAL_LOW); // CS
        usleep(CS_DELAY);
        for(i =0; i < 8; i++)
        {
                set_gpio_value(GPIO_SPI_CLK_VAL, GPIO_VAL_LOW); // CLK
                usleep(CLK_L_DELAY);
                set_gpio_value(GPIO_SPI_SDI_VAL, GPIO_BIT((7-i), reg)); 
                usleep(VAL_DELAY);
                set_gpio_value(GPIO_SPI_CLK_VAL, GPIO_VAL_HIGH); // CLK
                usleep(CLK_H_DELAY);
        }
        usleep(CS_DELAY);
        set_gpio_value(GPIO_SPI_CS_VAL, GPIO_VAL_HIGH); // CS
        usleep(DAT_DELAY);
        
        //_DEBUG("step 3: read data");
        
        set_gpio_value(GPIO_SPI_CS_VAL, GPIO_VAL_LOW); // CS
        usleep(CS_DELAY);
        for(i =0; i < 8; i++)
        {
                set_gpio_value(GPIO_SPI_CLK_VAL, GPIO_VAL_LOW); // CLK
                usleep(CLK_L_DELAY);
                regVal |= ((get_gpio_value(GPIO_SPI_SDO_VAL) == 0)? 0x00 : (0x01 << (7-i))); 
                usleep(VAL_DELAY);
                set_gpio_value(GPIO_SPI_CLK_VAL, GPIO_VAL_HIGH); // CLK
                usleep(CLK_H_DELAY);
        }
        usleep(CS_DELAY);
        set_gpio_value(GPIO_SPI_CS_VAL, GPIO_VAL_HIGH); // CS
        usleep(DAT_DELAY);
        
        return regVal;
}

void gpio_spi_write(unsigned char reg, unsigned char val)
{
        int i = 0;
        
        // first: send 0x20
        usleep(DAT_DELAY);
        set_gpio_value(GPIO_SPI_CS_VAL, GPIO_VAL_LOW); // CS
        usleep(CS_DELAY);
        for(i =0; i < 8; i++)
        {
                set_gpio_value(GPIO_SPI_CLK_VAL, GPIO_VAL_LOW); // CLK
                usleep(CLK_L_DELAY);
                set_gpio_value(GPIO_SPI_SDI_VAL, GPIO_BIT((7-i), 0x20)); 
                usleep(VAL_DELAY);
                set_gpio_value(GPIO_SPI_CLK_VAL, GPIO_VAL_HIGH); // CLK
                usleep(CLK_H_DELAY);
        }
        usleep(CS_DELAY);
        set_gpio_value(GPIO_SPI_CS_VAL, GPIO_VAL_HIGH); // CS
        usleep(DAT_DELAY);

        //second: send address
        set_gpio_value(GPIO_SPI_CS_VAL, GPIO_VAL_LOW); // CS
        usleep(CS_DELAY);
        for(i =0; i < 8; i++)
        {
                set_gpio_value(GPIO_SPI_CLK_VAL, GPIO_VAL_LOW); // CLK
                usleep(CLK_L_DELAY);
                set_gpio_value(GPIO_SPI_SDI_VAL, GPIO_BIT((7-i), reg)); 
                usleep(VAL_DELAY);
                set_gpio_value(GPIO_SPI_CLK_VAL, GPIO_VAL_HIGH); // CLK
                usleep(CLK_H_DELAY);
        }
        usleep(CS_DELAY);
        set_gpio_value(GPIO_SPI_CS_VAL, GPIO_VAL_HIGH); // CS
        usleep(DAT_DELAY);

        set_gpio_value(GPIO_SPI_CS_VAL, GPIO_VAL_LOW); // CS
        usleep(CS_DELAY);
        for(i =0; i < 8; i++)
        {
                set_gpio_value(GPIO_SPI_CLK_VAL, GPIO_VAL_LOW); // CLK
                usleep(CLK_L_DELAY);
                set_gpio_value(GPIO_SPI_SDI_VAL, GPIO_BIT((7-i), val)); 
                usleep(VAL_DELAY);
                set_gpio_value(GPIO_SPI_CLK_VAL, GPIO_VAL_HIGH); // CLK
                usleep(CLK_H_DELAY);
        }
        usleep(CS_DELAY);
        set_gpio_value(GPIO_SPI_CS_VAL, GPIO_VAL_HIGH); // CS
        usleep(DAT_DELAY);
        
}


