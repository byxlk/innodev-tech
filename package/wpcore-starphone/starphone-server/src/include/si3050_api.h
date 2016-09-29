#ifndef SI3050_API_H
#define SI3050_API_H

#include "pcm.h"
#include "gpio-spi.h"
#include "common.h"



#define SAMPLE_RATE 8000
#define CHANNEL 2
#define BUFFER_SIZE (1024)

/* Register addresses */

#define SI3050_REG_NOP			  			0x00  /* d00 */
#define SI3050_REG_CONTROL1			  		0x01  /* d01 */
#define SI3050_REG_CONTROL2					0x02  /* d02 */
#define SI3050_REG_INTERRUPT_MASK			0x03  /* d03 */
#define SI3050_REG_INTERRUPT_SOURCE			0x04  /* d04 */
#define SI3050_REG_DAA_CONTROL1				0x05  /* d05 */
#define SI3050_REG_DAA_CONTROL2				0x06  /* d06 */
#define SI3050_REG_SAMPLE_RATE		  		0x07  /* d07 */
#define SI3050_REG_RESERVED1		  		0x08  /* d08 */
#define SI3050_REG_RESERVED2				0x09  /* d09 */
#define SI3050_REG_DAA_CONTROL3		  		0x0A  /* d10 */
#define SI3050_REG_CHIP_A_REV				0x0B  /* d11 */
#define SI3050_REG_LINE_STATUS				0x0C  /* d12 */
#define SI3050_REG_CHIP_B_REV				0x0D  /* d13 */
#define SI3050_REG_DAA_CONTROL4		  		0x0E  /* d14 */
#define SI3050_REG_TX_RX_GAIN_CONTROL		0x0F  /* d15 */
#define SI3050_REG_INTERNATIONAL_CONTROL1	0x10  /* d16 */
#define SI3050_REG_INTERNATIONAL_CONTROL2	0x11  /* d17 */
#define SI3050_REG_INTERNATIONAL_CONTROL3	0x12  /* d18 */
#define SI3050_REG_INTERNATIONAL_CONTROL4	0x13  /* d19 */
#define SI3050_REG_CALL_PROGRESS_RX			0x14  /* d20 */
#define SI3050_REG_CALL_PROGRESS_TX			0x15  /* d21 */
#define SI3050_REG_RING_VALIDATION_CONTROL1	0x16  /* d22 */
#define SI3050_REG_RING_VALIDATION_CONTROL2	0x17  /* d23 */
#define SI3050_REG_RING_VALIDATION_CONTROL3	0x18  /* d24 */
#define SI3050_REG_RESISTOR_CALBRATION		0x19  /* d25 */
#define SI3050_REG_DC_TERMINATION_CONTROL	0x1A  /* d26 */
#define SI3050_REG_RESERVED3				0x1B  /* d27 */
#define SI3050_REG_LOOP_CURRENT_STATUS		0x1C  /* d28 */
#define SI3050_REG_LINE_VOLTAGE_STATUS		0x1D  /* d29 */
#define SI3050_REG_AC_TERMIATION_CONTROL	0x1E  /* d30 */
#define SI3050_REG_DAA_CONTROL5				0x1F  /* d31 */
#define SI3050_REG_GROUND_START_CONTROL		0x20  /* d32 */
#define SI3050_REG_PCM_SPI_MODE_SELECT		0x21  /* d33 */
#define SI3050_REG_PCM_TX_LOW				0x22  /* d34 */
#define SI3050_REG_PCM_TX_HIGH				0x23  /* d35 */
#define SI3050_REG_PCM_RX_LOW				0x24  /* d36 */
#define SI3050_REG_PCM_RX_HIGH				0x25  /* d37 */
#define SI3050_REG_TX_GAIN_CONTROL2			0x26  /* d38 */
#define SI3050_REG_RX_GAIN_CONTROL2			0x27  /* d39 */
#define SI3050_REG_TX_GAIN_CONTROL3			0x28  /* d40 */
#define SI3050_REG_RX_GAIN_CONTROL3			0x29  /* d41 */
#define SI3050_REG_GCI_CONTROL				0x2A  /* d42 */
#define SI3050_REG_LINE_IV_THRESHOLD_INTERRUPT			0x2B  /* d43 */
#define SI3050_REG_LINE_IV_THRESHOLD_INTERRUPT_CONTROL	0x2C  /* d44 */
#define SI3050_REG_PROGRAMMABLE_HYBRID1		0x2D  /* d45 */
#define SI3050_REG_PROGRAMMABLE_HYBRID2		0x2E  /* d46 */
#define SI3050_REG_PROGRAMMABLE_HYBRID3		0x2F  /* d47 */
#define SI3050_REG_PROGRAMMABLE_HYBRID4		0x30  /* d48 */
#define SI3050_REG_PROGRAMMABLE_HYBRID5		0x31  /* d49 */
#define SI3050_REG_PROGRAMMABLE_HYBRID6		0x32  /* d50 */
#define SI3050_REG_PROGRAMMABLE_HYBRID7		0x33  /* d51 */
#define SI3050_REG_PROGRAMMABLE_HYBRID8		0x34  /* d52 */
#define SI3050_REG_RESERVED4				0x35  /* d53 */
#define SI3050_REG_RESERVED5				0x36  /* d54 */
#define SI3050_REG_RESERVED6				0x37  /* d55 */
#define SI3050_REG_RESERVED7				0x38  /* d56 */
#define SI3050_REG_RESERVED8				0x39  /* d57 */
#define SI3050_REG_RESERVED9				0x3A  /* d58 */
#define SI3050_REG_SPARK_QUENCHING_CONTROL	0x3B  /* d59 */

/* PCM_MODE */
#define SI3050_PCM_TRI					0x01
#define SI3050_PCM_MULAW				0x08
#define SI3050_PCM_LINEAR				0x18
#define SI3050_PCM_ENABLE				0x20

/* INTERRUPT */
#define SI3050_RDT_INT					0x80
#define SI3050_ROV_INT					0x40
#define SI3050_FDT_INT					0x20
#define SI3050_BTD_INT					0x10
#define SI3050_DOD_INT					0x08
#define SI3050_LCSO_INT					0x04
#define SI3050_TGD_INT					0x02
#define SI3050_POL_INT					0x01

/* Broadcast/Read/Write */
#define SI3050_BRCT					0x80
#define SI3050_READ					0x60
#define SI3050_WRITE				0x20

#define SI3050_MAX_FTD_RETRY			25
#define SI3050_FTD_DELAY				100 /* In mSec */


////////////////////////////////////////////////////////
void si3050_generate_sine(int freq, int volume);

void si3050_pcm_init_config(struct pcm_config* config);

struct pcm* si3050_get_pcm_out(void);

void si3050_close_pcm_out( SPS_SYSTEM_INFO_T *sps);

struct pcm* si3050_get_pcm_in(void);

void si3050_close_pcm_in( SPS_SYSTEM_INFO_T *sps);

int si3050_play_sine(void);

void si3050_pcm_loopback(void);


///////////////////////////////////////////////////////
void Si3050_Get_VersionInfo(void);

void Si3050_Pin_Reset(void);

//void si3050_sw_reset(SPS_SYSTEM_INFO_T *sps);

void XW_Si3050_DAA_System_Init(void);

void Si3050_Pcm_DriverInit(SPS_SYSTEM_INFO_T *sps);

void *XW_Pthread_ModemCtrlDeamon(void *args);


#endif

