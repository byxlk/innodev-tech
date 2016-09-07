#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>

#include "test.h"
#include "si3050_api.h"
#include "gpio-spi.h"
#include "common.h"


int si3050_test_main(int argc, char **argv)
{
        usleep(50*1000);
        _DEBUG("REG[2] Default= 0x%0x", gpio_spi_read(2)); //0x0000_0011
        gpio_spi_write(2, 0xc3);
        _DEBUG("REG[2] Read = 0x%0x", gpio_spi_read(2)); //0x0000_0011
        //gpio_spi_read(6); //0x0001_0000
        //gpio_spi_read(22); //0x1001_0110
        //gpio_spi_read(23);//0x0010_1101
        //gpio_spi_read(24);//0x0001_1001
        //gpio_spi_read(31);//0x0010_0000

        return 0;
}

