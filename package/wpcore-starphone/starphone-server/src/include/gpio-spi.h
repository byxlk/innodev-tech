
#ifndef GPIO_SPI_H
#define GPIO_SPI_H

#define USING_GPIO_SPI_DRIVER 0


#if USING_GPIO_SPI_DRIVER
#define GPIO_SPI_DEVICE "/dev/gpio-spi"

#define IOCTL_GPIO_SPI_WRITE 0
#define IOCTL_GPIO_SPI_READ 1
#define IOCTL_SET_RESET_PIN_HIGH 2
#define IOCTL_SET_RESET_PIN_LOW 3
#define IOCTL_SI3050_HW_RESET 4
#define IOCTL_SI3050_SW_RESET 5
#define IOCTL_SET_SPICLK_PIN_HIGH 6
#define IOCTL_SET_SPICLK_PIN_LOW 7
#define IOCTL_SET_SPICS_PIN_HIGH 8
#define IOCTL_SET_SPICS_PIN_LOW 9
#define IOCTL_SET_SPISDI_PIN_HIGH 10
#define IOCTL_SET_SPISDI_PIN_LOW 11

#else

#define GPIO_DIR_OUTPUT "out"
#define GPIO_DIR_INPUT "in"
#define  GPIO_VAL_HIGH "1"
#define GPIO_VAL_LOW "0"

#define GPIO_BIT(x, y) ((((0x01 << (x)) & (y)) == 0)? GPIO_VAL_LOW : GPIO_VAL_HIGH)

#define GPIO_SPI_CLK_DIR  "/sys/class/gpio/gpio18/direction"
#define GPIO_SPI_CS_DIR    "/sys/class/gpio/gpio17/direction"
#define GPIO_SPI_SDI_DIR  "/sys/class/gpio/gpio21/direction"
#define GPIO_SPI_SDO_DIR "/sys/class/gpio/gpio20/direction"
#define GPIO_RESET_DIR      "/sys/class/gpio/gpio8/direction"
#define GPIO_INT_DIR          "/sys/class/gpio/gpio10/direction"


#define GPIO_SPI_CLK_VAL "/sys/class/gpio/gpio18/value"
#define GPIO_SPI_CS_VAL "/sys/class/gpio/gpio17/value"
#define GPIO_SPI_SDI_VAL "/sys/class/gpio/gpio21/value"
#define GPIO_SPI_SDO_VAL "/sys/class/gpio/gpio20/value"
#define GPIO_RESET_VAL "/sys/class/gpio/gpio8/value"
#define GPIO_INT_VAL "/sys/class/gpio/gpio10/value"

#endif

struct si3050_reg {
    unsigned int addr;
    unsigned int val;
};


int gpio_spi_write(unsigned char reg, unsigned char val);
unsigned char gpio_spi_read(unsigned char reg);
void gpio_spi_port_dir_init(void);
void set_reset_pin_high(void);
void set_reset_pin_low(void);
void set_spiclk_pin_high(void);
void set_spiclk_pin_low(void);
void set_spics_pin_high(void);
void set_spics_pin_low(void);
void set_spisdi_pin_high(void);
void set_spisdi_pin_low(void);

#endif

