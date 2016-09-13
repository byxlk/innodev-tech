#ifndef COMMON_H
#define COMMON_H

#include <pthread.h>

#include "pcm.h"



#define DEBUG_FLAG 1
#define ERROR_FLAG 1



#if DEBUG_FLAG
#define _DEBUG(msg...) do{ printf("[DEBUG][%s: %d] ",__FUNCTION__,__LINE__); \
        printf(msg); \
        printf("\r\n"); \
}while(0);
#else
#define _DEBUG(msg...)
#endif

#if ERROR_FLAG
#define _ERROR(msg...) do{ printf("[ERROR][%s: %d] ",__FUNCTION__,__LINE__); \
        printf(msg); \
        printf("\r\n"); \
}while(0);
#else
#define _ERROR(msg...)
#endif


typedef struct  _STARPHONE_SERVER{
    //Thread para
    pthread_t thread_id_upd;
    pthread_t thread_id_tcp;
    
    struct pcm *si3050_pcm_out;
    struct pcm *si3050_pcm_in;
    unsigned char *pcm_dat_buff;
    
    unsigned char phone_status;
    unsigned char ring_count;
}starphone_server;


#endif

