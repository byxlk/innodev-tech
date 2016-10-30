
#ifndef GPIO_SPI_H
#define GPIO_SPI_H

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



void gpio_spi_write(unsigned char reg, unsigned char val);
unsigned char gpio_spi_read(unsigned char reg);

inline int set_gpio_direction(const char *gpio_port, const char *gpio_dir);
inline int set_gpio_value(const char *gpio_port, const char *gpio_val);
inline int get_gpio_value(const char *gpio_port);




#endif

