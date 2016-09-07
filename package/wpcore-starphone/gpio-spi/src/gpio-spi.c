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
#define _DEBUG(msg...)  do{ printk("[DEBUG][%s: %d] ",__FUNCTION__,__LINE__); \
            printk(msg); \
            printk("\n"); \
}while(0);
#else
#define _DEBUG(msg...);
#endif

#if GPIO_SPI_ERROR
#define _ERROR(msg...) do{ printk("[ERROR][%s: %d] ",__FUNCTION__,__LINE__); \
            printk(msg); \
            printk("\n"); \
}while(0);
#else
#define _ERROR(msg...);
#endif


#define DELAY_DAT (500)
#define DELAY_CS (10)

#define DELAY_CLK_L (100)
#define DELAY_VAL (100)
#define DELAY_CLK_H (200)

//#define DELAY_DAT (5)
//#define DELAY_CS (1)

//#define DELAY_CLK_L (1)
//#define DELAY_VAL (1)
//#define DELAY_CLK_H (2)


#define GPIO_BIT(x, y) ((((0x01 << (x)) & (y)) == 0)? GPIO_VAL_LOW : GPIO_VAL_HIGH)

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
                 if(gpio_val == GPIO_VAL_LOW)
                        *MAP_GPIO21_00_DATA &= ~(0x1 << gpio_port);
                 else                        
                        *MAP_GPIO21_00_DATA |= (0x1 << gpio_port);
        }
        else if(gpio_port < 28 && gpio_port > 21)
        {
                 if(gpio_val == GPIO_VAL_LOW)
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
                if((*MAP_GPIO21_00_DATA) & (0x1 << gpio_port))
                        return GPIO_VAL_HIGH;
                else
                        return GPIO_VAL_LOW;                 
        }
        else if(gpio_port < 28 && gpio_port > 21)
        {
                 if((*MAP_GPIO27_22_DATA) & (0x1 << gpio_port))
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


static int gpio_spi_byte_write(unsigned char reg, unsigned char val)
{
        int i = 0;
        
        // first: send 0x20
        udelay(DELAY_DAT);
        set_gpio_value(GPIO_SPI_CS, GPIO_VAL_LOW); // CS
        udelay(DELAY_CS);
        for(i =0; i < 8; i++)
        {
                set_gpio_value(GPIO_SPI_CLK, GPIO_VAL_LOW); // CLK
                udelay(DELAY_CLK_L);
                set_gpio_value(GPIO_SPI_SDI, GPIO_BIT((7-i), 0x20)); 
                udelay(DELAY_VAL);
                set_gpio_value(GPIO_SPI_CLK, GPIO_VAL_HIGH); // CLK
                udelay(DELAY_CLK_H);
        }
        udelay(DELAY_CS);
        set_gpio_value(GPIO_SPI_CS, GPIO_VAL_HIGH); // CS
        udelay(DELAY_DAT);

        //second: send address
        set_gpio_value(GPIO_SPI_CS, GPIO_VAL_LOW); // CS
        udelay(DELAY_CS);
        for(i =0; i < 8; i++)
        {
                set_gpio_value(GPIO_SPI_CLK, GPIO_VAL_LOW); // CLK
                udelay(DELAY_CLK_L);
                set_gpio_value(GPIO_SPI_SDI, GPIO_BIT((7-i), reg)); 
                udelay(DELAY_VAL);
                set_gpio_value(GPIO_SPI_CLK, GPIO_VAL_HIGH); // CLK
                udelay(DELAY_CLK_H);
        }
        udelay(DELAY_CS);
        set_gpio_value(GPIO_SPI_CS, GPIO_VAL_HIGH); // CS
        udelay(DELAY_DAT);

        set_gpio_value(GPIO_SPI_CS, GPIO_VAL_LOW); // CS
        udelay(DELAY_CS);
        for(i =0; i < 8; i++)
        {
                set_gpio_value(GPIO_SPI_CLK, GPIO_VAL_LOW); // CLK
                udelay(DELAY_CLK_L);
                set_gpio_value(GPIO_SPI_SDI, GPIO_BIT((7-i), val)); 
                udelay(DELAY_VAL);
                set_gpio_value(GPIO_SPI_CLK, GPIO_VAL_HIGH); // CLK
                udelay(DELAY_CLK_H);
        }
        udelay(DELAY_CS);
        set_gpio_value(GPIO_SPI_CS, GPIO_VAL_HIGH); // CS
        udelay(DELAY_DAT);

        return 0;
}
static unsigned char gpio_spi_byte_read(unsigned char reg)
{
        int i = 0;
        unsigned char regVal = 0x00;
        
        //_DEBUG("step 1: send 0x60");
        
        // first: send 0x60
        udelay(DELAY_DAT);
        set_gpio_value(GPIO_SPI_CS, GPIO_VAL_LOW); // CS       
        udelay(DELAY_CS);
        for(i =0; i < 8; i++)
        {
                set_gpio_value(GPIO_SPI_CLK, GPIO_VAL_LOW); // CLK
                udelay(DELAY_CLK_L);
                set_gpio_value(GPIO_SPI_SDI, GPIO_BIT((7-i), 0x60)); 
                udelay(DELAY_VAL);
                set_gpio_value(GPIO_SPI_CLK, GPIO_VAL_HIGH); // CLK
                udelay(DELAY_CLK_H);
        }
        udelay(DELAY_CS);
        set_gpio_value(GPIO_SPI_CS, GPIO_VAL_HIGH); // CS
        udelay(DELAY_DAT);

         //_DEBUG("step 2: send address 0x%x",reg);
         
        //second: send address
        set_gpio_value(GPIO_SPI_CS, GPIO_VAL_LOW); // CS
        udelay(DELAY_CS);
        for(i =0; i < 8; i++)
        {
                set_gpio_value(GPIO_SPI_CLK, GPIO_VAL_LOW); // CLK
                udelay(DELAY_CLK_L);
                set_gpio_value(GPIO_SPI_SDI, GPIO_BIT((7-i), reg)); 
                udelay(DELAY_VAL);
                set_gpio_value(GPIO_SPI_CLK, GPIO_VAL_HIGH); // CLK
                udelay(DELAY_CLK_H);
        }
        udelay(DELAY_CS);
        set_gpio_value(GPIO_SPI_CS, GPIO_VAL_HIGH); // CS
        udelay(DELAY_DAT);
        
        //_DEBUG("step 3: read data");
        
        set_gpio_value(GPIO_SPI_CS, GPIO_VAL_LOW); // CS
        udelay(DELAY_CS);
        for(i =0; i < 8; i++)
        {
                set_gpio_value(GPIO_SPI_CLK, GPIO_VAL_LOW); // CLK
                udelay(DELAY_CLK_L);
                regVal |= ((get_gpio_value(GPIO_SPI_SDO) == 0)? 0x00 : (0x01 << (7-i))); 
                udelay(DELAY_VAL);
                set_gpio_value(GPIO_SPI_CLK, GPIO_VAL_HIGH); // CLK
                udelay(DELAY_CLK_H);
        }
        udelay(DELAY_CS);
        set_gpio_value(GPIO_SPI_CS, GPIO_VAL_HIGH); // CS
        udelay(DELAY_DAT);

        return regVal;
}


/////////////////////////////////////////////////////////////////////////
static void si3050_get_ver_info(void)
{
        unsigned char sys_ver_val = 0x00;
        unsigned char line_ver_val = 0x00;
        
        _DEBUG("Get si3050 infomation Start ...");        
        
        sys_ver_val = gpio_spi_byte_read(11);        
        line_ver_val = gpio_spi_byte_read(13);
        
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

static void si3050_hw_reset(void)
{
        udelay(50*1000);
        set_gpio_value(GPIO_SPI_RESET, GPIO_VAL_LOW); // RESET
        udelay(50*1000);
        udelay(50*1000);
        udelay(50*1000);
        udelay(50*1000);
        //udelay(50*1000);
        //udelay(50*1000);      
        //sleep(1);
        set_gpio_value(GPIO_SPI_RESET, GPIO_VAL_HIGH); // RESET
        udelay(50*1000);
        udelay(50*1000);
        udelay(50*1000);
        udelay(50*1000);
        udelay(50*1000);
        udelay(50*1000);
        //usleep(50*1000);
        //usleep(50*1000);
        //usleep(50*1000);
        //usleep(50*1000);
}


void si3050_power_up_si3019(void)
{
        gpio_spi_byte_write(6, 0x00);
}


/////////////////////////////////////////////////////////////////////////
static struct class *gpio_spi_class;

static int gpio_spi_open(struct inode *inode, struct file *file)
{
#if USING_IO_REMAP
    /*
        MAP_GPIOMODE:
            bit6:   JTAG_GPIO_MODE 1:GPIO Mode
            bit4:2: UARTF 111:GPIO Mode
    */
    *MAP_GPIOMODE |= (0x7<<2)|(0x1<<6);

    /* Config GPIO#7,8,9,10,17,18 as output mode */
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

/* 1. malloc and config file_operations structer */
static struct file_operations gpio_spi_fops = {
    .owner              = THIS_MODULE,                  
    .open               = gpio_spi_open,
    .write          = gpio_spi_write,
//    .read           = gpio_spi_byte_read,
    .unlocked_ioctl = gpio_spi_unlocked_ioctl,
    .release            = gpio_spi_close,
};

int major;
static int __init gpio_spi_init(void)
{
    unsigned int gpio_cfg = 0;
    
    /* 2. Register Char device */
    major = register_chrdev(0, "gpio_spi", &gpio_spi_fops);

    /* 3. Automatic create device node */
    /* Create Class */
    gpio_spi_class = class_create(THIS_MODULE, "motor");
    
    /* subdevice node in class */
    device_create(gpio_spi_class, NULL, MKDEV(major, 0), NULL, "motor");       // /dev/motor

#if USING_IO_REMAP
    /* 4. Oprater with hardware related */
    /* map GPIO register */
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
    _DEBUG("Config gpio value complete...");
    
    /* FOR TEST */
    mdelay(2);
    set_gpio_value(GPIO_SPI_RESET, GPIO_VAL_LOW);
    mdelay(250);
    set_gpio_value(GPIO_SPI_RESET, GPIO_VAL_HIGH);
    mdelay(250);
    _DEBUG("Reset si3050 complete...");
    
        // Check the Version to make sure SPI Conmunication is OK
        si3050_get_ver_info();
    /*
        //Enable si3050 PCM interface 
        regCfg = gpio_spi_byte_read(33);
        regCfg |= (0x1 << 3) | (0x1 << 5); // Enable PCM & u-Law
        regCfg &= ~(0x1 << 4);
        gpio_spi_write(33, regCfg);
    
        //Specific county Seting for Taiwan
        regCfg = gpio_spi_byte_read(16);
        regCfg &= ~((0x1 << 0) & (0x1 << 1) & (0x1 << 4) &  (0x1 << 6)); // OHS RZ RT
        gpio_spi_write(16, regCfg);
    
        regCfg = gpio_spi_byte_read(26);
        regCfg |= (0x1 << 6) | (0x1 << 7); // DCV[1:0] = 11
        regCfg &= ~((0x1 << 1) & (0x1 << 4) & (0X1 << 5));
        gpio_spi_write(26, regCfg);
    
        regCfg = gpio_spi_byte_read(30);
        regCfg &= ~((0x1 << 0) & (0x1 << 1) & (0x1 << 2) & (0x1 << 3) & (0x1 << 4));
        gpio_spi_write(30, regCfg);
    
        regCfg = gpio_spi_byte_read(31);
        regCfg &= ~(0x1 << 3); // OHS2 = 0
        gpio_spi_write(31, regCfg);
    
        si3050_power_up_si3019();
    */        
    
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

