
#ifndef GPIO_SPI_H
#define GPIO_SPI_H

#define GPIO_SPI_DEVICE "/dev/gpio-spi"

#define IOCTL_GPIO_SPI_WRITE 0
#define IOCTL_GPIO_SPI_READ 1
#define IOCTL_SET_RESET_PIN_HIGH 2
#define IOCTL_SET_RESET_PIN_LOW 3
#define IOCTL_SI3050_HW_RESET 4
#define IOCTL_SI3050_SW_RESET 5

struct si3050_reg {
    unsigned int addr;
    unsigned int val;
};


int gpio_spi_write(unsigned char reg, unsigned char val);

unsigned char gpio_spi_read(unsigned char reg);

void set_reset_pin_high(void);

void set_reset_pin_low(void);


#endif

