#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/mman.h>
#include <linux/random.h>
#include <linux/init.h>
#include <linux/raw.h>
#include <linux/tty.h>
#include <linux/capability.h>
#include <linux/ptrace.h>
#include <linux/device.h>
#include <linux/highmem.h>
#include <linux/crash_dump.h>
#include <linux/backing-dev.h>
#include <linux/bootmem.h>
#include <linux/splice.h>
#include <linux/pfn.h>
#include <linux/export.h>
#include <linux/io.h>
#include <linux/aio.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <asm/uaccess.h>


#define GPIO_SPI_DEBUG 1
#define GPIO_SPI_ERROR 1
#define USING_IO_REMAP 1

/* Debug INTERFACE INFORMATION */
#if GPIO_SPI_DEBUG
#define _DEBUG(msg...)  \
do{ \
            printk("[DEBUG][%s: %d] ",__FUNCTION__,__LINE__); \
            printk(msg); \
            printk("\n"); \
}while(0);
#else
#define _DEBUG(msg...);
#endif

#if GPIO_SPI_ERROR
#define _ERROR(msg...) \
do{ \
            printk("[ERROR][%s: %d] ",__FUNCTION__,__LINE__); \
            printk(msg); \
            printk("\n"); \
}while(0);
#else
#define _ERROR(msg...);
#endif

#define rt5350_reg_read(addr) __raw_readl(ioremap((addr), 4))
#define rt5350_reg_write(addr, val) __raw_writel((val), ioremap((addr), 4))

#define GPIOMODE_BASE 0x10000000
#define GPIOMODE_OFFSET 0x0060

#define GPIO_REG_BASE 0x10000600
#define GPIO21_00_INT 0x0000
#define GPIO21_00_EDGE 0x0004
#define GPIO21_00_RENA 0x0008
#define GPIO21_00_FENA 0x000c
#define GPIO21_00_POL 0x0028
#define GPIO21_00_SET 0x002c
#define GPIO21_00_RESET 0x0030
#define GPIO21_00_TOG 0x0034
#define GPIO21_00_DIR 0x0024
#define GPIO21_00_DATA 0x0020

#define GPIO27_22_INT 0x0060
#define GPIO27_22_EDGE 0x0064
#define GPIO27_22_RENA 0x0068
#define GPIO27_22_FENA 0x006c
#define GPIO27_22_POL 0x0078
#define GPIO27_22_SET 0x007c
#define GPIO27_22_RESET 0x0080
#define GPIO27_22_TOG 0x0084
#define GPIO27_22_DIR 0x0074
#define GPIO27_22_DATA 0x0070


#define GPIO_DIR_OUTPUT 1
#define GPIO_DIR_INPUT 0
#define GPIO_VAL_HIGH 1
#define GPIO_VAL_LOW 0 

#define GPIO_SPI_CS (17)
#define GPIO_SPI_CLK (18)
#define GPIO_SPI_SDI (21)
#define GPIO_SPI_SDO (20)

#define GPIO_SPI_INT (10)
#define GPIO_SPI_RESET (8)



volatile unsigned long *MAP_GPIOMODE;
volatile unsigned long *MAP_GPIO21_00_DIR;
volatile unsigned long *MAP_GPIO21_00_DATA;
volatile unsigned long *MAP_GPIO27_22_DIR;
volatile unsigned long *MAP_GPIO27_22_DATA;




/////////////////////////////////////////////////////////////////////////
static void gpio_ioremap_init(void)
{
        MAP_GPIOMODE = (volatile unsigned long *)ioremap(GPIOMODE_BASE+GPIOMODE_OFFSET, 4);
        MAP_GPIO21_00_DIR = (volatile unsigned long *)ioremap(GPIO_REG_BASE+GPIO21_00_DIR, 4);
        MAP_GPIO21_00_DATA = (volatile unsigned long *)ioremap(GPIO_REG_BASE+GPIO21_00_DATA, 4);
        MAP_GPIO27_22_DIR = (volatile unsigned long *)ioremap(GPIO_REG_BASE+GPIO27_22_DIR, 4);
        MAP_GPIO27_22_DATA = (volatile unsigned long *)ioremap(GPIO_REG_BASE+GPIO27_22_DATA, 4);
}

static void gpio_mode_init(void)
{
#if USING_IO_REMAP
        *MAP_GPIOMODE &= ~((0x1 << 2) | (0x1 << 3));
        *MAP_GPIOMODE |= (0x1 << 4);// JTAG_GPIO_MODE     
        *MAP_GPIOMODE |= (0x1 << 6);// UARTF_SHARE_MODE  PCM + GPIO = b100
#else
    unsigned int gpio_cfg = 0;
    
    gpio_cfg = rt5350_reg_read(GPIOMODE_BASE + GPIOMODE_OFFSET);   
    gpio_cfg |= (0x1 << 6); // JTAG_GPIO_MODE     
    gpio_cfg |= (0x4 << 2);// UARTF_SHARE_MODE  PCM + GPIO = b100
    rt5350_reg_write(GPIOMODE_BASE + GPIOMODE_OFFSET, gpio_cfg);
#endif
}

static inline int set_gpio_direction(unsigned char gpio_port, unsigned char gpio_dir)
{
#if USING_IO_REMAP
         if(gpio_port < 22 && gpio_port > 0)
        {
                 if(gpio_dir == GPIO_DIR_INPUT)
                        *MAP_GPIO21_00_DIR  &= ~(0x1 << gpio_port);
                 else                        
                        *MAP_GPIO21_00_DIR  |= (0x1 << gpio_port);
        }
        else if(gpio_port < 28 && gpio_port > 21)
        {
                 if(gpio_dir == GPIO_DIR_INPUT)
                        *MAP_GPIO27_22_DIR  &= ~(0x1 << gpio_port);
                 else                        
                        *MAP_GPIO27_22_DIR  |= (0x1 << gpio_port);
        }
        else
        {
                _ERROR("GPIO Port Number[%d] invaild. ",gpio_port);
                return -1;
        }        
#else
        unsigned int gpio_cfg = 0;
        unsigned int gpio_dir_offset = 0;
        //Config GPIO DIR register

        if(gpio_port < 22 && gpio_port > 0)
                gpio_dir_offset = GPIO21_00_DIR;
        else if(gpio_port < 28 && gpio_port > 21)
                gpio_dir_offset = GPIO27_22_DIR;
        else
        {
                _ERROR("GPIO Port Number[%d] invaild. ",gpio_port);
                return -1;
        }
        
        gpio_cfg = rt5350_reg_read(GPIO_REG_BASE + gpio_dir_offset); 
        if(gpio_dir == GPIO_DIR_INPUT)
                gpio_cfg &= ~(0x1 << gpio_port); // Set GPIO as input mode
        else
                gpio_cfg |= (0x1 << gpio_port); //Set GPIO as output mode
          rt5350_reg_write(GPIO_REG_BASE + gpio_dir_offset, gpio_cfg);
#endif

        return 0;
}

static inline int set_gpio_value(unsigned char gpio_port, unsigned char gpio_val)
{
#if USING_IO_REMAP
        if(gpio_port < 22 && gpio_port > 0)
        {
                 if(gpio_val == GPIO_DIR_INPUT)
                        *MAP_GPIO21_00_DATA &= ~(0x1 << gpio_port);
                 else                        
                        *MAP_GPIO21_00_DATA |= (0x1 << gpio_port);
        }
        else if(gpio_port < 28 && gpio_port > 21)
        {
                 if(gpio_val == GPIO_DIR_INPUT)
                        *MAP_GPIO27_22_DATA  &= ~(0x1 << gpio_port);
                 else                        
                        *MAP_GPIO27_22_DATA |= (0x1 << gpio_port);
        }
        else
        {
                _ERROR("GPIO Port Number[%d] invaild. ",gpio_port);
                return -1;
        }        
#else
            unsigned int gpio_cfg = 0;
            unsigned int gpio_dir_offset = 0;
            //Config GPIO DIR register
    
            if(gpio_port < 22 && gpio_port > 0)
                    gpio_dir_offset = GPIO21_00_DATA;
            else if(gpio_port < 28 && gpio_port > 21)
                    gpio_dir_offset = GPIO27_22_DATA;
            else
            {
                    _ERROR("GPIO Port Number[%d] invaild. ",gpio_port);
                    return -1;
            }
            
            gpio_cfg = rt5350_reg_read(GPIO_REG_BASE + gpio_dir_offset); 
            if(gpio_val == GPIO_VAL_LOW)
                    gpio_cfg &= ~(0x1 << gpio_port); // Set GPIO as input mode
            else
                    gpio_cfg |= (0x1 << gpio_port); //Set GPIO as output mode
              rt5350_reg_write(GPIO_REG_BASE + gpio_dir_offset, gpio_cfg);
#endif

        return 0;
}
static inline int get_gpio_value(unsigned char gpio_port)
{
#if USING_IO_REMAP
        if(gpio_port < 22 && gpio_port > 0)
        {
                if(*MAP_GPIO21_00_DATA & (0x1 << gpio_port))
                        return GPIO_VAL_HIGH;
                else
                        return GPIO_VAL_LOW;                 
        }
        else if(gpio_port < 28 && gpio_port > 21)
        {
                 if(*MAP_GPIO27_22_DATA & (0x1 << gpio_port))
                        return GPIO_VAL_HIGH;
                else
                        return GPIO_VAL_LOW;
        }
        else
        {
                _ERROR("GPIO Port Number[%d] invaild. ",gpio_port);
                return -1;
        }        
#else
            unsigned int gpio_cfg = 0;
            unsigned int gpio_dir_offset = 0;
            //Config GPIO DIR register
    
            if(gpio_port < 22 && gpio_port > 0)
                    gpio_dir_offset = GPIO21_00_DIR;
            else if(gpio_port < 28 && gpio_port > 21)
                    gpio_dir_offset = GPIO27_22_DIR;
            else
            {
                    _ERROR("GPIO Port Number[%d] invaild. ",gpio_port);
                    return -1;
            }
            
            gpio_cfg = rt5350_reg_read(GPIO_REG_BASE + gpio_dir_offset); 
            if(gpio_cfg & (0x1 << gpio_port))
                    return GPIO_VAL_HIGH;
            else
                    return GPIO_VAL_LOW;
#endif

        return 0;
}


static void gpio_spi_byte_write(unsigned char reg, unsigned char val);
static unsigned char gpio_spi_byte_read(unsigned char reg);



/////////////////////////////////////////////////////////////////////////



static struct class *gpio_spi_class;

static int gpio_spi_open(struct inode *inode, struct file *file)
{
#if USING_IO_REMAP
    /* ÅäÖÃÏàÓŠµÄÒýœÅÓÃÓÚGPIO */
    /*
        MAP_GPIOMODE:
            bit6:   JTAG_GPIO_MODE 1:GPIO Mode
            bit4:2: UARTF 111:GPIO Mode
    */
    *MAP_GPIOMODE |= (0x7<<2)|(0x1<<6);

    /* œ«GPIO#7¡¢GPIO#8¡¢GPIO#9¡¢GPIO#10¡¢GPIO#17¡¢GPIO#18ÉèÖÃÎªÊä³ö */
    *MAP_GPIO21_00_DIR |= (1<<7)|(1<<8)|(1<<9)|(1<<10)|(1<<17)|(1<<18);
#else


#endif
    return 0;
}

static int gpio_spi_close(struct inode *inode, struct file *file)
{

    return 0;
}


static ssize_t gpio_spi_write(struct file *file, const char __user *buf, 
size_t size, loff_t *ppos)
{
    unsigned int val;

    copy_from_user(&val, buf, 4);
#if USING_IO_REMAP

    if(val & 0x1)
    {
        *MAP_GPIO21_00_DATA |= (1<<17)|(1<<18);
    }
    else
    {
        *MAP_GPIO21_00_DATA &= ~((1<<17)|(1<<18));
    }
#else

#endif

    return 1;
}

static long gpio_spi_unlocked_ioctl(struct file *file, unsigned int cmd, 
unsigned long arg)
{
//#if USING_IO_REMAP
    
//#else

//#endif

    return 0;
}

/* 1.·ÖÅä¡¢ÉèÖÃÒ»žöfile_operationsœá¹¹Ìå */
static struct file_operations gpio_spi_fops = {
    /* ÕâÊÇÒ»žöºê£¬ÍÆÏò±àÒëÄ£¿éÊ±×Ô¶¯ŽŽœšµÄ__this_module±äÁ¿ */
    .owner              = THIS_MODULE,                  
    .open               = gpio_spi_open,
    .write          = gpio_spi_write,
//    .read           = gpio_spi_read,
    .unlocked_ioctl = gpio_spi_unlocked_ioctl,
    .release            = gpio_spi_close,
};

int major;
static int __init gpio_spi_init(void)
{
    unsigned int gpio_cfg = 0;
    
    /* 2.×¢²á */
    major = register_chrdev(0, "gpio_spi", &gpio_spi_fops);

    /* 3.×Ô¶¯ŽŽœšÉè±žœÚµã */
    /* ŽŽœšÀà */
    gpio_spi_class = class_create(THIS_MODULE, "motor");
    /* ÀàÏÂÃæŽŽœšÉè±žœÚµã */
    device_create(gpio_spi_class, NULL, MKDEV(major, 0), NULL, "motor");       // /dev/motor

#if USING_IO_REMAP
    /* 4.Ó²ŒþÏà¹ØµÄ²Ù×÷ */
    /* Ó³ÉäŒÄŽæÆ÷µÄµØÖ· */
    gpio_ioremap_init();
#endif

    // Config GPIO MODE register
    gpio_mode_init();

    //Config GPIO DIR register
    set_gpio_direction(GPIO_SPI_INT, GPIO_DIR_INPUT);
    set_gpio_direction(GPIO_SPI_SDO, GPIO_DIR_INPUT);

    set_gpio_direction(GPIO_SPI_RESET, GPIO_DIR_OUTPUT);
    set_gpio_direction(GPIO_SPI_CS, GPIO_DIR_OUTPUT);
    set_gpio_direction(GPIO_SPI_CLK, GPIO_DIR_OUTPUT);
    set_gpio_direction(GPIO_SPI_SDI, GPIO_DIR_OUTPUT);
        
    /* RESET(GPIO#8) Default value is output 1, Others output 0 */
    set_gpio_value(GPIO_SPI_CS, GPIO_VAL_HIGH);
    set_gpio_value(GPIO_SPI_CLK, GPIO_VAL_HIGH);
    set_gpio_value(GPIO_SPI_SDI, GPIO_VAL_HIGH);
    set_gpio_value(GPIO_SPI_RESET, GPIO_VAL_HIGH);

    /* FOR TEST */
    mdelay(2);
    set_gpio_value(GPIO_SPI_RESET, GPIO_VAL_LOW);
    mdelay(250);
    set_gpio_value(GPIO_SPI_RESET, GPIO_VAL_HIGH);
    mdelay(250);
    
    return 0;
}

static void __exit gpio_spi_exit(void)
{
    unregister_chrdev(major, "gpio_spi");
    device_destroy(gpio_spi_class, MKDEV(major, 0));
    class_destroy(gpio_spi_class);
    
 #if USING_IO_REMAP
    iounmap(MAP_GPIOMODE);
    iounmap(MAP_GPIO21_00_DIR);
    iounmap(MAP_GPIO21_00_DATA);
    iounmap(MAP_GPIO27_22_DIR);
    iounmap(MAP_GPIO27_22_DATA);
 #endif
 
}


module_init(gpio_spi_init);
module_exit(gpio_spi_exit);

MODULE_DESCRIPTION("GPIO as SPI driver");
MODULE_AUTHOR("Shuliang Li");
MODULE_LICENSE("GPL");

