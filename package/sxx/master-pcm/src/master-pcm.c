/*
 *  Copyright (C) 2010, Lars-Peter Clausen <lars@metafoo.de>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under  the terms of the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <linux/init.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

#include <linux/delay.h>

#include <linux/dma-mapping.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/initval.h>
#include <sound/dmaengine_pcm.h>

#include <ralink_regs.h>

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


//GLB_CFG: GLB_CFG Register (offset: 0x0000)
#define PCM_GLB_CFG 0x0000
#define PCM_GLB_CFG_EN BIT(31)                 //(0x1 << 31)
#define PCM_GLB_CFG_DMA_EN BIT(30)       //(0x1 << 30)
#define PCM_GLB_CFG_CH0_TX_EN BIT(8)   //(0x1 << 8)
#define PCM_GLB_CFG_CH0_RX_EN BIT(0)  //(0x1 << 0)

#define PCM_GLB_CFG_RFF_THRES 20
#define PCM_GLB_CFG_TFF_THRES 16
#define PCM_GLB_CFG_DFT_THRES	(4 << PCM_GLB_CFG_RFF_THRES) | \
					(4 << PCM_GLB_CFG_TFF_THRES)
					
 //PCM_CFG: PCM_CFG Register (offset: 0x0004)
#define PCM_PCM_CFG 0x0004
#define PCM_PCM_CFG_CLKOUT_EN BIT(30)
#define PCM_PCM_CFG_EXT_FSYNC BIT(27)
#define PCM_PCM_CFG_LONG_FSYNC BIT(26)
#define PCM_PCM_CFG_FSYNC_POL BIT(25)
#define PCM_PCM_CFG_SLOT_MODE (0x30000000 >> 28)



#define PCM_INT_STATUS 0x08
#define PCM_INT_EN 0x0C
#define PCM_FF_STATUS 0x10
#define PCM_CH0_CFG 0x20
#define PCM_CH1_CFG 0x24
#define PCM_FSYNC_CFG 0x30
#define PCM_CH_CFG2 0x34

#define PCM_DIVCOMP_CFG 0x50
#define PCM_DIVCOMP_CFG_CLK_EN BIT(31)

#define PCM_DIVINT_CFG 0x54
#define PCM_DIGDELAY_CFG 0x60
#define PCM_CH0_FIFO 0x80
#define PCM_CH1_FIFO 0x84


#define PCM_DMA_DEBUG 0

/* Debug INTERFACE INFORMATION */
#if PCM_DMA_DEBUG
#define _DEBUG(msg...)  \
do{ \
            printk("[DEBUG][%s: %d] ",__FUNCTION__,__LINE__); \
            printk(msg); \
            printk("\n"); \
}while(0)
#define _ERROR(msg...) \
do{ \
            printk("[ERROR][%s: %d] ",__FUNCTION__,__LINE__); \
            printk(msg); \
            printk("\n"); \
}while(0)
#else
#define _DEBUG(msg...)
#define _ERROR(msg...)
#endif



struct rt5350_pcm {
	struct resource *mem;
	void __iomem *base;
	dma_addr_t phys_base;

	struct snd_dmaengine_dai_dma_data playback_dma_data;
	struct snd_dmaengine_dai_dma_data capture_dma_data;
};

static inline uint32_t rt5350_reg_read(unsigned int reg_addr)
{
        return __raw_readl(ioremap(reg_addr, 4));
}

static inline void rt5350_reg_write(	unsigned int reg_addr, uint32_t value)
{
        __raw_writel(value, ioremap(reg_addr, 4));
}


static inline uint32_t rt5350_pcm_read(const struct rt5350_pcm *pcm,
	unsigned int reg)
{
	return readl(pcm->base + reg);
}

static inline void rt5350_pcm_write(const struct rt5350_pcm *pcm,
	unsigned int reg, uint32_t value)
{
	writel(value, pcm->base + reg);
}

void getAllRegValue(const struct rt5350_pcm *pcm)
{
        _DEBUG("++");
        _DEBUG("+++++++++++++++++++++++++++++++++++++++");
        _DEBUG("[Offset: %04X] PCM_GLB_CFG = 0x%08X",PCM_GLB_CFG,rt5350_pcm_read(pcm, PCM_GLB_CFG));
        _DEBUG("[Offset: %04X] PCM_PCM_CFG = 0x%08X",PCM_PCM_CFG,rt5350_pcm_read(pcm, PCM_PCM_CFG));
        _DEBUG("[Offset: %04X] PCM_INT_STATUS = 0x%08X",PCM_INT_STATUS,rt5350_pcm_read(pcm, PCM_INT_STATUS));
        _DEBUG("[Offset: %04X] PCM_INT_EN = 0x%08X",PCM_INT_EN,rt5350_pcm_read(pcm, PCM_INT_EN));
        _DEBUG("[Offset: %04X] PCM_FF_STATUS = 0x%08X",PCM_FF_STATUS,rt5350_pcm_read(pcm, PCM_FF_STATUS));
        _DEBUG("[Offset: %04X] PCM_CH0_CFG = 0x%08X",PCM_CH0_CFG,rt5350_pcm_read(pcm, PCM_CH0_CFG));
        _DEBUG("[Offset: %04X] PCM_CH1_CFG = 0x%08X",PCM_CH1_CFG,rt5350_pcm_read(pcm, PCM_CH1_CFG));
        _DEBUG("[Offset: %04X] PCM_FSYNC_CFG = 0x%08X",PCM_FSYNC_CFG,rt5350_pcm_read(pcm, PCM_FSYNC_CFG));
        _DEBUG("[Offset: %04X] PCM_CH_CFG2 = 0x%08X",PCM_CH_CFG2,rt5350_pcm_read(pcm, PCM_CH_CFG2));
        _DEBUG("[Offset: %04X] PCM_DIVCOMP_CFG = 0x%08X",PCM_DIVCOMP_CFG,rt5350_pcm_read(pcm, PCM_DIVCOMP_CFG));
        _DEBUG("[Offset: %04X] PCM_DIVINT_CFG = 0x%08X",PCM_DIVINT_CFG,rt5350_pcm_read(pcm, PCM_DIVINT_CFG));
        _DEBUG("[Offset: %04X] PCM_DIGDELAY_CFG = 0x%08X",PCM_DIGDELAY_CFG,rt5350_pcm_read(pcm, PCM_DIGDELAY_CFG));
        _DEBUG("[Offset: %04X] PCM_CH0_FIFO = 0x%08X",PCM_CH0_FIFO,rt5350_pcm_read(pcm, PCM_CH0_FIFO));
        _DEBUG("[Offset: %04X] PCM_CH1_FIFO = 0x%08X",PCM_CH1_FIFO,rt5350_pcm_read(pcm, PCM_CH1_FIFO));
        _DEBUG("+++++++++++++++++++++++++++++++++++++++");
        _DEBUG("--");
}

static int rt5350_pcm_startup(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	struct rt5350_pcm *pcm = snd_soc_dai_get_drvdata(dai);
	uint32_t cfg;

        _DEBUG("++");

	if (dai->active) {
                _DEBUG("--");
		return 0;
	}

	cfg = rt5350_pcm_read(pcm, PCM_GLB_CFG);
        cfg |= PCM_GLB_CFG_EN;
        _DEBUG("PCM_ENABLE");
	rt5350_pcm_write(pcm, PCM_GLB_CFG, cfg);
        
        _DEBUG("--");
	return 0;
}

static void rt5350_pcm_shutdown(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	struct rt5350_pcm *pcm = snd_soc_dai_get_drvdata(dai);
	uint32_t cfg;

        _DEBUG("++");

	if (dai->active) {
                _DEBUG("--");
		return;
	}

	cfg = rt5350_pcm_read(pcm, PCM_GLB_CFG);
	cfg &= ~PCM_GLB_CFG_EN;
        _DEBUG("PCM_DISABLE");
	rt5350_pcm_write(pcm, PCM_GLB_CFG, cfg);
        
        _DEBUG("--");
}

static int rt5350_pcm_trigger(struct snd_pcm_substream *substream, int cmd,
	struct snd_soc_dai *dai)
{
	struct rt5350_pcm *pcm = snd_soc_dai_get_drvdata(dai);

	uint32_t cfg;
	uint32_t mask;

        //_DEBUG("++");

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		mask = PCM_GLB_CFG_CH0_TX_EN;
                //_DEBUG("CH0_TX_EN");
	 }
	else {
		mask = PCM_GLB_CFG_CH0_RX_EN;
                //_DEBUG("CH0_RX_EN");
	}

	cfg = rt5350_pcm_read(pcm, PCM_GLB_CFG);

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		cfg |= mask;
		break;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		cfg &= ~mask;
		break;
	default:
		return -EINVAL;
	}

	if (cfg & (PCM_GLB_CFG_CH0_TX_EN | PCM_GLB_CFG_CH0_RX_EN)) {
		cfg |= PCM_GLB_CFG_DMA_EN;
                //_DEBUG("DMA_EN = 1");
	    }
	else {
		cfg &= ~PCM_GLB_CFG_DMA_EN;
                //_DEBUG("DMA_EN = 0");
	    }

	rt5350_pcm_write(pcm, PCM_GLB_CFG, cfg);

        //_DEBUG("--");
	return 0;
}

static int rt5350_pcm_set_fmt(struct snd_soc_dai *dai, unsigned int fmt)
{
	struct rt5350_pcm *pcm = snd_soc_dai_get_drvdata(dai);
	uint32_t cfg;

        _DEBUG("++");

	cfg = rt5350_pcm_read(pcm, PCM_PCM_CFG);

	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	    case SND_SOC_DAIFMT_CBS_CFS:
		cfg &= ~PCM_PCM_CFG_CLKOUT_EN; // pcm clock from external
		_DEBUG("EXT_CLK_EN");
                cfg |= PCM_PCM_CFG_EXT_FSYNC; // pcm sync from external
                _DEBUG("EXT_FSYNC");
		break;
	    case SND_SOC_DAIFMT_CBM_CFM:
		cfg |= PCM_PCM_CFG_CLKOUT_EN; // pcm clock from internal
		_DEBUG("INT_CLK_EN");
                cfg &= ~PCM_PCM_CFG_EXT_FSYNC; // pcm sync from internal
                _DEBUG("INT_FSYNC");
		break;
	    case SND_SOC_DAIFMT_CBM_CFS:
	    default:
		return -EINVAL;
	}
    
	rt5350_pcm_write(pcm, PCM_PCM_CFG, cfg);

        _DEBUG("--");
	return 0;
}

static int rt5350_pcm_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{

	return 0;
}

//#define PCMCLOCK_OUT 0 // 256KHz
//#define PCMCLOCK_OUT 3 // 512khz  --- ok
#define PCMCLOCK_OUT 9 // 512khz
//#define PCMCLOCK_OUT 5 // 512khz


unsigned long i2sMaster_inclk_int[11]  = {78, 56, 52, 39, 28, 26, 19, 14, 13, 9, 6};
unsigned long i2sMaster_inclk_comp[11] = {64, 352, 42, 32, 176, 21, 272, 88, 10, 392/*455*/, 261};

//FreqOut = FreqIn *(1/2) *{1 / [DIVINT+DIVCOMP/(512)]}

static int rt5350_pcm_set_sysclk(struct snd_soc_dai *dai, int clk_id,
	unsigned int freq, int dir)
{
        struct rt5350_pcm *pcm = snd_soc_dai_get_drvdata(dai);

        _DEBUG("++");
	_DEBUG("Internal REFCLK with fractional division");

        //When using the external clock, the frequency clock
        //should be equal to the PCM_clock out. Otherwise, the
        //PCM_CLKin should be 8.192 MHz.
	rt5350_pcm_write(pcm, PCM_DIVINT_CFG, i2sMaster_inclk_int[PCMCLOCK_OUT]);
        _DEBUG("DIVINT = %d",i2sMaster_inclk_int[PCMCLOCK_OUT]);
	rt5350_pcm_write(pcm, PCM_DIVCOMP_CFG, i2sMaster_inclk_comp[PCMCLOCK_OUT] | PCM_DIVCOMP_CFG_CLK_EN);
        _DEBUG("DIVCOMP = %d",i2sMaster_inclk_comp[PCMCLOCK_OUT]);
	//_DEBUG("Clk_Out = %f KHz",(40000.0*(1/2)*(1/(i2sMaster_inclk_int[PCMCLOCK_OUT]+i2sMaster_inclk_comp[PCMCLOCK_OUT]/512.0))));
        
	_DEBUG("--");
        return 0;
}

static int rt5350_pcm_suspend(struct snd_soc_dai *dai)
{
	struct rt5350_pcm *pcm = snd_soc_dai_get_drvdata(dai);
	uint32_t cfg;

        _DEBUG("++");
        
	if (dai->active) {
		cfg = rt5350_pcm_read(pcm, PCM_GLB_CFG);
		cfg &= ~PCM_GLB_CFG_CH0_TX_EN;                
                _DEBUG("CH0_TX_EN = 0");
		rt5350_pcm_write(pcm, PCM_GLB_CFG, cfg);
	}

        _DEBUG("--");
	return 0;
}

static int rt5350_pcm_resume(struct snd_soc_dai *dai)
{
	struct rt5350_pcm *pcm = snd_soc_dai_get_drvdata(dai);
	uint32_t cfg;

        _DEBUG("++");

	if (dai->active) {
		cfg = rt5350_pcm_read(pcm, PCM_GLB_CFG);
		cfg |= PCM_GLB_CFG_CH0_TX_EN;
                _DEBUG("GLB_CFG_CH0_TX_EN = 1");
		rt5350_pcm_write(pcm, PCM_GLB_CFG, cfg);
	}

        _DEBUG("--");
	return 0;
}

static void rt5350_init_pcm_config(struct rt5350_pcm *pcm)
{
	struct snd_dmaengine_dai_dma_data *dma_data;

        _DEBUG("++");

	/* Playback */
	dma_data = &pcm->playback_dma_data;
	dma_data->maxburst = 16;
	dma_data->slave_id = 6;
	dma_data->addr = pcm->phys_base + PCM_CH0_FIFO; //only use channel 0

	/* Capture */
	dma_data = &pcm->capture_dma_data;
	dma_data->maxburst = 16;
	dma_data->slave_id = 4;
	dma_data->addr = pcm->phys_base + PCM_CH0_FIFO;
    
        _DEBUG("--");
}

static int rt5350_pcm_dai_probe(struct snd_soc_dai *dai)
{
	struct rt5350_pcm *pcm = snd_soc_dai_get_drvdata(dai);
	uint32_t cfg;
    
        _DEBUG("++");
        
	rt5350_init_pcm_config(pcm);
	dai->playback_dma_data = &pcm->playback_dma_data;
	dai->capture_dma_data = &pcm->capture_dma_data;

	_DEBUG("Internal REFCLK with fractional division");

        _DEBUG("DFT_THERES = 4 ");
	rt5350_pcm_write(pcm, PCM_GLB_CFG, PCM_GLB_CFG_DFT_THRES);
	rt5350_pcm_write(pcm, PCM_INT_EN, 0);
        _DEBUG("INT_EN = 0x00");
    
        ///////////// pcm general config
        cfg = rt5350_pcm_read(pcm, PCM_PCM_CFG);
        _DEBUG("LONG_FSYNC = 1");
        //cfg |= PCM_PCM_CFG_LONG_FSYNC; //long sync mode
        cfg &= ~PCM_PCM_CFG_LONG_FSYNC; //short sync mode    
        _DEBUG("FSYNC_POL = 1");
        cfg |= PCM_PCM_CFG_FSYNC_POL; // sync high active
    
        //slot mode, pcm clock = 256KHz
        //cfg &= ~(0x07);
        //cfg = 0; // 4 slots
        cfg |= PCM_PCM_CFG_SLOT_MODE; // 32 slot, pcm clock = 2.048MHz   
        _DEBUG("SLOT_MODE = 3");
        
        rt5350_pcm_write(pcm, PCM_PCM_CFG, cfg);
    
        ///////////// pcm sync config
        //cfg = rt5350_pcm_read(pcm, PCM_FSYNC_CFG);
        // pol, etc.
    
        //rt5350_pcm_write(pcm, PCM_FSYNC_CFG, cfg);
    
        //When using the external clock, the frequency clock
        //should be equal to the PCM_clock out. Otherwise, the
        //PCM_CLKin should be 8.192 MHz.
	rt5350_pcm_write(pcm, PCM_DIVINT_CFG, i2sMaster_inclk_int[PCMCLOCK_OUT]);        
        _DEBUG("DIVINT = %d",i2sMaster_inclk_int[PCMCLOCK_OUT]);
	rt5350_pcm_write(pcm, PCM_DIVCOMP_CFG, i2sMaster_inclk_comp[PCMCLOCK_OUT] | PCM_DIVCOMP_CFG_CLK_EN);
        _DEBUG("DIVCOMP = %d",i2sMaster_inclk_comp[PCMCLOCK_OUT]);        
        //_DEBUG("Clk_Out = %f KHz",(40000.0*(1/2)*(1/(i2sMaster_inclk_int[PCMCLOCK_OUT]+i2sMaster_inclk_comp[PCMCLOCK_OUT]/512.0))));

        _DEBUG("--");
        getAllRegValue(pcm);
	return 0;
}

static int rt5350_pcm_dai_remove(struct snd_soc_dai *dai)
{
	return 0;
}

static const struct snd_soc_dai_ops rt5350_pcm_dai_ops = {
	.startup = rt5350_pcm_startup,
	.shutdown = rt5350_pcm_shutdown,
	.trigger = rt5350_pcm_trigger,
	.hw_params = rt5350_pcm_hw_params,
	.set_fmt = rt5350_pcm_set_fmt,
	.set_sysclk = rt5350_pcm_set_sysclk,
};

#define RT5350_PCM_FMTS (SNDRV_PCM_FMTBIT_S8 | \
		SNDRV_PCM_FMTBIT_S16_LE)

static struct snd_soc_dai_driver rt5350_pcm_dai = {
	.probe = rt5350_pcm_dai_probe,
	.remove = rt5350_pcm_dai_remove,
	.playback = {
		.channels_min = 1,
		.channels_max = 2,
		.rates = SNDRV_PCM_RATE_8000_48000,
		.formats = RT5350_PCM_FMTS,
	},
	.capture = {
		.channels_min = 2,
		.channels_max = 2,
		.rates = SNDRV_PCM_RATE_8000_48000,
		.formats = RT5350_PCM_FMTS,
	},
	.symmetric_rates = 1,
	.ops = &rt5350_pcm_dai_ops,
	.suspend = rt5350_pcm_suspend,
	.resume = rt5350_pcm_resume,
};

static const struct snd_pcm_hardware mt7620_pcm_hardware = {
	.info = SNDRV_PCM_INFO_MMAP |
		SNDRV_PCM_INFO_MMAP_VALID |
		SNDRV_PCM_INFO_INTERLEAVED |
		SNDRV_PCM_INFO_BLOCK_TRANSFER,
	.formats = SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S8,
	.period_bytes_min	= 128,//PAGE_SIZE,
	.period_bytes_max	= 64 * 1024,
	.periods_min		= 2,
	.periods_max		= 128,
	.buffer_bytes_max	= 128 * 1024,
	.fifo_size		= 32,
};

static const struct snd_dmaengine_pcm_config rt5350_dmaengine_pcm_config = {
	.prepare_slave_config = snd_dmaengine_pcm_prepare_slave_config,
	.pcm_hardware = &mt7620_pcm_hardware,
	.prealloc_buffer_size = 256 * PAGE_SIZE,
};

static const struct snd_soc_component_driver rt5350_pcm_component = {
	.name = "master-pcm",
};

static int rt5350_pcm_dev_probe(struct platform_device *pdev)
{
	struct rt5350_pcm *pcm;
	int ret;
        unsigned int gpio_cfg;

        _DEBUG("++");

        // Config GPIO MODE register
        //gpio_cfg = rt5350_reg_read(GPIOMODE_BASE + GPIOMODE_OFFSET);   
        //_DEBUG("GPIO MODE REG: 0x%0x\n",gpio_cfg);
        //gpio_cfg |= (0x1 << 6); // JTAG_GPIO_MODE     
        //gpio_cfg |= (0x4 << 2);// UARTF_SHARE_MODE  PCM + GPIO = b100
        //rt5350_reg_write(GPIOMODE_BASE + GPIOMODE_OFFSET, gpio_cfg);

        //Config GPIO DIR register
        //gpio_cfg = rt5350_reg_read(GPIO_REG_BASE + GPIO21_00_DIR);    
        //_DEBUG("GPIO DIR REG: 0x%0x\n",gpio_cfg);
        //gpio_cfg &= ~((0x1 << 10) | (0x1 << 21));// Set GPIO#10 & GPIO#21 as input
        //gpio_cfg |= (1 << 8)|(1 << 17)|(1 << 18); //Set GPIO#8,17,18 as output mode
        //gpio_cfg |= (1 << 19)|(1 << 20)|(1 << 21);//Set GPIO#19,20,21 as output mode
        //rt5350_reg_write(GPIO_REG_BASE + GPIO21_00_DIR, gpio_cfg);
    
        /* RESET(GPIO#8) Default value is output 1, Others output 0 */
        //gpio_cfg = rt5350_reg_read(GPIO_REG_BASE + GPIO21_00_DATA); 
        //_DEBUG("GPIO DATA REG: 0x%0x\n",gpio_cfg);
        //gpio_cfg |= (1 << 8)|(1 << 19);
        //gpio_cfg &= ~((1 << 17) | (1 << 18) | (1 << 20));
        //rt5350_reg_write(GPIO_REG_BASE + GPIO21_00_DIR, gpio_cfg);
        
	snd_dmaengine_pcm_register(&pdev->dev,
		&rt5350_dmaengine_pcm_config,
		SND_DMAENGINE_PCM_FLAG_COMPAT);

	pcm = kzalloc(sizeof(*pcm), GFP_KERNEL);
	if (!pcm) {
                _DEBUG("--");
		return -ENOMEM;
	}
    
	pcm->mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!pcm->mem) {
		ret = -ENOENT;
		goto err_free;
	}

	pcm->mem = request_mem_region(pcm->mem->start, resource_size(pcm->mem),
				pdev->name);
	if (!pcm->mem) {
		ret = -EBUSY;
		goto err_free;
	}

	pcm->base = ioremap_nocache(pcm->mem->start, resource_size(pcm->mem));
	if (!pcm->base) {
		ret = -EBUSY;
		goto err_release_mem_region;
	}

	pcm->phys_base = pcm->mem->start;

	platform_set_drvdata(pdev, pcm);
	ret = snd_soc_register_component(&pdev->dev, &rt5350_pcm_component,
					 &rt5350_pcm_dai, 1);

	if (!ret) {
		dev_err(&pdev->dev, "loaded");
                _DEBUG("--");
		return ret;
	}

	dev_err(&pdev->dev, "Failed to register DAI");
	iounmap(pcm->base);

err_release_mem_region:
	release_mem_region(pcm->mem->start, resource_size(pcm->mem));
err_free:
	kfree(pcm);

        _DEBUG("--");
	return ret;
}

static int rt5350_pcm_dev_remove(struct platform_device *pdev)
{
	struct rt5350_pcm *pcm = platform_get_drvdata(pdev);

        _DEBUG("++");

	snd_soc_unregister_component(&pdev->dev);

	iounmap(pcm->base);
	release_mem_region(pcm->mem->start, resource_size(pcm->mem));

	kfree(pcm);

	snd_dmaengine_pcm_unregister(&pdev->dev);

        _DEBUG("--");

	return 0;
}

static const struct of_device_id rt5350_pcm_match[] = {
	{ .compatible = "ralink,rt5350-pcm,master-pcm" },
	{},
};
MODULE_DEVICE_TABLE(of, rt5350_pcm_match);

static struct platform_driver rt5350_pcm_driver = {
	.probe = rt5350_pcm_dev_probe,
	.remove = rt5350_pcm_dev_remove,
	.driver = {
		.name = "master-pcm",
		.owner = THIS_MODULE,
		.of_match_table = rt5350_pcm_match,
	},
};

module_platform_driver(rt5350_pcm_driver);

MODULE_AUTHOR("Derek Quan <qdk0901@qq.com>");
MODULE_DESCRIPTION("RT5350 PCM Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:rt5350-pcm");
