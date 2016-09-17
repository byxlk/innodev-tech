#ifndef SI3050_API_H
#define SI3050_API_H

#include "pcm.h"
#include "gpio-spi.h"
#include "common.h"



#define SAMPLE_RATE 8000
#define CHANNEL 2
#define BUFFER_SIZE (1024)

////////////////////////////////////////////////////////
void si3050_generate_sine(int freq, int volume);

void si3050_pcm_init_config(struct pcm_config* config);

struct pcm* si3050_get_pcm_out(void);

void si3050_close_pcm_out( starphone_server *sps);

struct pcm* si3050_get_pcm_in(void);

void si3050_close_pcm_in( starphone_server *sps);

int si3050_play_sine(void);

void si3050_pcm_loopback(void);


///////////////////////////////////////////////////////
void si3050_get_ver_info(void);

void si3050_hw_reset(void);

void si3050_sw_reset(starphone_server *sps);

void si3050_sys_init(void);

void si3050_power_up_si3019(void);

void si3050_pcm_dev_drv_init(starphone_server *sps);

void *XW_Pthread_ModemCtrlDeamon(void *args);


#endif

