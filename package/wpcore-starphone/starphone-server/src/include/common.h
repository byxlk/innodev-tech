#ifndef COMMON_H
#define COMMON_H

#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <linux/fb.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/un.h>
#include <signal.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <sys/mount.h>
#include <sys/vfs.h>
#include <sys/msg.h>
#include <termios.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <math.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <net/if.h>
#include <netinet/in.h>
#include <net/if_arp.h>
#include <dirent.h>
#include <net/route.h>
#include <netpacket/packet.h>
#include <linux/if_ether.h>
#include <linux/rtc.h>
#include <ctype.h>

#include "pcm.h"


#define SPS_EXT_DEBUG_FLAG 1
#define SPS_EXT_ERROR_FLAG 1


#if SPS_EXT_DEBUG_FLAG
#define _DEBUG(msg...) do{ printf("[%11lds][DEBUG][%s: %d] ",(long int)time(NULL),__FUNCTION__,__LINE__);\
        printf(msg); \
        printf("\r\n"); \
}while(0);
#else
#define _DEBUG(msg...)
#endif

#if SPS_EXT_ERROR_FLAG
#define _ERROR(msg...) do{  printf("\033[31m\033[1m[%11lds][ERROR][%s: %d]\033[0m ",(long int)time(NULL),__FUNCTION__,__LINE__);  \
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


#define PROC_PARTITIONS "/proc/partitions"
#define PROCESS_RUNING_FILE  "/var/IsProcessExist.txt"
#define SYSTEM_CONFIG_FILE  "/mnt/img/systemconfig.dat"
#define P2P_NATPORT 31000


typedef enum
{
    NVP1104A_CHIP = 0,
    NVP1108B_CHIP,
    TW2868_CHIP,
    NVP1104b_CHIP,
    VIDEO_CHIP_TYPE = NVP1108B_CHIP
} VIDEO_CHIP_T;


#define NVP1104B_ADDR 0x60
#define VIDEOCHIP_DEVNAME  "/dev/nc_vdec"
//#define VIDEOCHIP_DEVNAME  "/dev/tw286xdev"

#define   DOORDVR_CHANNEL_NUM         4
#define   ViDevTotal       2


#define   RTPLOING 99
#define   ViChnPerDev     4

#define   ALARM_OUTPUT_CHANNEL_NUM     2
#define   SYSTEM_MAX_USER_COUNT            16
#define   SYSTEM_MAX_DISKNUM                   2
#define   DVR_SET_TIMER(t)                       do{t=time(NULL);}while(0)
#define   DVR_JUDGE_COND(t, sec)                 (time(NULL)-t > sec )

#define  DVR_MAX_STREAM_NUM              2
#define  DVR_ALL_CH_NUM  (DOORDVR_CHANNEL_NUM*DVR_MAX_STREAM_NUM)
#define  MAIN_STREAM      0
#define  SUB_STREAM        1
#define  NETMAIN_STREAM   1
#define  NETSUB_STREAM    0

typedef enum
{
    DISK_ISNOT_VALID = 0,
    DISK_IS_VALID
} DISK_VALID_T;

//----------------------------------------------------------------------------------------------------------------

#define dvr_free(pval)  		free(pval);pval = NULL;
//----------------------------------------------------------------------------------------------------------------
//×Ö·ûÀàÐÍ¶šÒå
typedef uint64_t         U64_T;
typedef uint32_t         U32_T;
typedef uint16_t         U16_T;
typedef uint8_t          U8_T;
typedef unsigned char    UCHAR_T;
typedef void             VOID_T;

typedef int64_t     I64_T;
typedef int32_t     I32_T;
typedef int16_t     I16_T;
typedef int8_t      I8_T;
typedef char            CHAR_T;
typedef unsigned long   ULONG_T;

typedef pthread_t              DVR_PTH_T;
typedef pthread_mutex_t DVR_MUTEX_T;
typedef pthread_cond_t  DVR_COND_T;
typedef sem_t           DVR_SEM_T;
typedef time_t          DVR_TIME_T;
typedef struct tm           DVR_TM_T;
typedef struct timeval      DVR_TIMEV_T;

//----------------------------------------------------------------------------------------------------------------
#define DVR_CREATE_THREAD(Func, Args)   do{                 \
        pthread_t       __pth__;                                    \
        pthread_create(&__pth__, NULL, (void *)Func, (void *)Args); \
    }while(0)

/* ŽŽœš·ÖÀëÏß³Ì£¬·ÀÖ¹×èÈûÏµÍ³ */
#define DVR_PTHREAD_DETACH          do{ pthread_detach(pthread_self()); }while(0)

typedef enum
{
    SPS_STATE_INIT = 0,
    SPS_STATE_RUNNING = 1,
    SPS_STATE_PAUSE = 2,
    SPS_STATE_ERROR = 3,
    SPS_STATE_EXIT = 4,
    SPS_STATE_STOP = 5
} SPS_SYSTEM_STATE_T;

typedef enum
{
    IDLE = 0,
    BUSY,
    ALIVE,
    PAUSE,
    EXIT
} MAIN_STATE;

typedef enum
{
    PTHREAD_POWER_ON = 0,
    PTHREAD_POWER_OFF
} PTHREAD_POWER;

typedef enum
{
    PTHREAD_MAIN_ID = 0,
    PTHREAD_UDP_BROADCAST_ID,            //Ô€ÀÀ
    PTHREAD_CLIENT_CONNECT_ID, 		   //¿ŽÃÇ¹·
    PTHREAD_CLIENT_MANAGE_ID,             //Ž®¿Ú
    PTHREAD_MODEM_CTRL_ID,          //ÊÓÆµ¶ªÊ§Œì²é
    PTHREAD_USBHOTPLUG_ID,      //USB hotPlug
    PTHREAD_DISKM_ID,
    PTHREAD_UPDATE_ID,
    PTHREAD_MAX_ID
} PTHREAD_ID;


#define NOTNEEND_CH 0
typedef enum
{
    //ÏÖ³¡
    L_CH1_FULL_SCREEN,//µ¥»­Ãæ
    L_CH2_FULL_SCREEN,
    L_CH3_FULL_SCREEN,
    L_CH4_FULL_SCREEN,
    L_CH5_FULL_SCREEN,
    L_CH6_FULL_SCREEN,
    L_CH7_FULL_SCREEN,
    L_CH8_FULL_SCREEN,
    L_TWOSPLIT_CH1_CH2,///2·Öžî
    L_TWOSPLIT_CH5_CH6,
    L_THRIDSPLIT_CH1_CH2_CH3,/// 3·Öžî
    L_THRIDSPLIT_CH5_CH6_CH7,

	// DAY/NIGHT
	UPPER_LEFT_SHOW_DAY_3SEC,
	UPPER_LEFT_SHOW_NIGHT_3SEC,
	
	//BRIGHTNESS
	UPPER_LEFT_INC_BRIGHTNESS_3SEC,
	UPPER_LEFT_DEC_BRIGHTNESS_3SEC,
	
	//IR_ON/IROFF
	UPPER_RIGHT_SHOW_IR_ON_3SEC,
	UPPER_RIGHT_SHOW_IR_OFF_3SEC,

	//PARKING LINE
	SHOW_PARKING_LINE_ON,
	SHOW_PARKING_LINE_OFF
	
} PREVIEW_MODE;


typedef enum
{
    INIT_FLAG = 0,
    WRITE_FLAG  ,
    READ_FLAG,
} PTHREAD_SIG_COND;

typedef struct __PTHREAD_BUF__
{
    PTHREAD_ID      start_id;
    U16_T       m_signal;
    U32_T       m_value;
    U16_T       m_channel;
    U32_T       m_time;
    U8_T        m_buffer[80];
} PTHREAD_BUF;

typedef struct
{
    MAIN_STATE      state;
    PTHREAD_POWER   power;
} STATE_PTHREAD;

typedef struct __STATE_MAIN__
{
    MAIN_STATE      state;
    PTHREAD_POWER   power;
} STATE_T;

typedef struct __STATE_PREVIEW__
{
    MAIN_STATE  state;
    PTHREAD_POWER   power;
} STATE_PREVIEW;

typedef struct __STATE_DISKM__
{
    MAIN_STATE  state;
    PTHREAD_POWER   power;
} STATE_DISKM;


typedef struct __PTHREAD_STATE__
{
    STATE_T              state_main;
    STATE_T              state_live;//ÏÖ³¡
    STATE_T          state_schedule;//µ÷¶È
    STATE_T            state_serial;//Ž®¿Ú
    STATE_T            state_serial2;//usb serial 0
    STATE_T            state_serial3; //usb serial 1
    STATE_T         state_videolost;//ÊÓÆµ¶ªÊ§
    STATE_T               state_ptz; //ÔÆÌš
    STATE_T        state_osddisplay;//OSD
    STATE_T         state_usbhotplug;
    STATE_T         state_diskm;
    
} PTHREAD_STATE;


typedef enum
{
    false = 0,
    true  = 1
} bool;


typedef enum
{
    SUCCESS = 0,
    FAILURE,
    ENDING,
    PASS
} RETURN;

typedef enum
{
    HOUR_24 = 0,
    HOUR_12,
    isPM,
    isAM
} TIME_MODE_T;

typedef struct tagDateTimeDef_
{
    unsigned int year;
    unsigned int month;
    unsigned int day;
    unsigned int hour;
    unsigned int minute;
    unsigned int second;
    unsigned int millisecond;
} DateTimeDef;

typedef struct tagRtc_time_t
{
    unsigned int second;
    unsigned int minute;
    unsigned int hour;
    unsigned int date;
    unsigned int month;
    unsigned int year;
    unsigned int weekday;
    unsigned int tm_yday;
    unsigned int tm_isdst;
} rtc_time_t, *prtc_time_t;

typedef enum
{
    SYSTEM_NTSC = 0,
    SYSTEM_PAL
} System_Format_T;


typedef enum
{
    REC_SW,
    STREAM_SW,
    MAX_SWITCH
} SWITCH_TYPE_T;

typedef enum hiSAMPLE_VO_DIV_MODE
{
    DIV_MODE_1  = 1,    /* 1-screen display */
    DIV_MODE_2 = 2,
    DIV_MODE_4  = 4,    /* 4-screen display */
    DIV_MODE_8 = 8,
    DIV_MODE_9  = 9,    /* 9-screen display */
    DIV_MODE_16 = 16,   /* 16-screen display */
    DIV_MODE_BUTT
} SAMPLE_VO_DIV_MODE;

#define BT656_WORKMODE VI_WORK_MODE_4D1
typedef struct RECT_tag
{
    int  x;
    int  y;
    int  width;
    int  height;
} RECT;

typedef enum
{
    MB_UNIT = 0,
    KB_UNIT,
    B_UNIT,
} FILEUINT;

#define    SERIALNUMMAX 1
#define    KEYSERIAL  0
#define    DOORSERIAL 1

typedef enum
{
    AVI_ONLY_VIDEO = 0,
    AVI_AUDIOVIDEO
} AVI_TYPE_T;

typedef  enum UDTOPT
{
    UDT_MSS,             // the Maximum Transfer Unit
    UDT_SNDSYN,          // if sending is blocking
    UDT_RCVSYN,          // if receiving is blocking
    UDT_CC,              // custom congestion control algorithm
    UDT_FC,              // deprecated, for compatibility only
    UDT_SNDBUF,          // maximum buffer in sending queue
    UDT_RCVBUF,          // UDT receiving buffer size
    UDT_LINGER,          // waiting for unsent data when closing
    UDP_SNDBUF,          // UDP sending buffer size
    UDP_RCVBUF,          // UDP receiving buffer size
    UDT_MAXMSG,          // maximum datagram message size
    UDT_MSGTTL,          // time-to-live of a datagram message
    UDT_RENDEZVOUS,      // rendezvous connection mode
    UDT_SNDTIMEO,    // send() timeout
    UDT_RCVTIMEO     // recv() timeout
} UDTOPT_T;

typedef enum
{
    AUDIO_LIVE_MODE,
    AUDIO_PLAYBACK_MODE,
    AUDIO_TALKBACK_MODE,
    AUDIO_TEXT_MODE
} AUDIO_MODE;

typedef enum
{
    ISLIVE = 0,
    ISPB,
} LIVEORPB_T;


typedef enum
{
    SEND_MESSAGE_TO_UI = 1,
    SEND_MESSAGE_TO_SLAVE,
    MESSAGE_TYPE_RecordCtr_FROM_UI = 3,
    MESSAGE_TYPE_AlarmCtr_FROM_UI,
    MESSAGE_TYPE_Advanced_FROM_UI,
    MESSAGE_TYPE_Common_FROM_UI,
    MESSAGE_TYPE_Encoder_FROM_UI,
    MESSAGE_TYPE_Recorder_FROM_UI,
    MESSAGE_TYPE_Ptz_Parma_FROM_UI,
    MESSAGE_TYPE_Network_Parma_FROM_UI,
    MESSAGE_TYPE_Alarm_Parma_FROM_UI,
    MESSAGE_TYPE_Videodetect_Parma_FROM_UI,
    MESSAGE_TYPE_Videolost_Parma_FROM_UI,
    MESSAGE_TYPE_Localdisplay_FROM_UI,
    MESSAGE_TYPE_MAX
}
MESSAGE_QUEUE_TUPE;

typedef enum
{
    /**
    *ÉèÖÃÏµÍ³²ÎÊýÏûÏ¢¶ÓÁÐÃüÁî
    **/
    MSP_CMD_HIFB_INIT_OK,
	MSP_CMD_CH1_FULL_SCREEN,//µ¥»­Ãæ
    MSP_CMD_CH2_FULL_SCREEN,
    MSP_CMD_CH3_FULL_SCREEN,
    MSP_CMD_CH4_FULL_SCREEN,
    MSP_CMD_CH5_FULL_SCREEN,
    MSP_CMD_CH6_FULL_SCREEN,
    MSP_CMD_CH7_FULL_SCREEN,
    MSP_CMD_CH8_FULL_SCREEN,
    MSP_CMD_TWOSPLIT_CH1_CH2,///2·Öžî
    MSP_CMD_TWOSPLIT_CH5_CH6,
    MSP_CMD_THRIDSPLIT_CH1_CH2_CH3,/// 3·Öžî
    MSP_CMD_THRIDSPLIT_CH5_CH6_CH7,
	MSP_CMD_PREVIEW_CHANNEL_UPDATE,
	// DAY/NIGHT
	MSP_CMD_SHOW_DAY_3SEC,
	MSP_CMD_SHOW_NIGHT_3SEC,
	MSP_CMD_SHOW_DAYNIGHT_3SEC,
	//BRIGHTNESS
	MSP_CMD_BRIGHTNESS_INC_3SEC,
	MSP_CMD_BRIGHTNESS_DEC_3SEC,
	MSP_CMD_BRIGHTNESS_INCDEC_3SEC,
	//IR_ON/IROFF
	MSP_CMD_IR_ON_3SEC,
	MSP_CMD_IR_OFF_3SEC,
	MSP_CMD_IR_ONOFF_3SEC,
	
	//PARKING LINE
	MSP_CMD_PARKING_LINE_ON,
	MSP_CMD_PARKING_LINE_OFF,
	MSP_CMD_PARKING_LINE_ONOFF,

    //Updat
    MSP_CMD_SYSTEMUPGRADE,
    MSP_CMD_UPDATE_BEGIN,
    MSP_CMD_UPDATE_INITOK,
    MSP_CMD_UPDATE_BACKUP,
    MSP_CMD_UPDATE_DOING,
    MSP_CMD_UPDATE_END,
    MSP_CMD_UPDATE_LOGO,
    MSP_CMD_UPDATE_LOGOOK,
    MSP_CMD_UPDATE_LOGOERR,
    MSP_CMD_UPDATE_ERROR,

    /**
    *×ÜÃüÁîÊý
    **/
    MSP_CMD_NUM,
} MspCmdID;

typedef int MspId;

typedef struct MspSendCmd
{
	MspCmdID    ctrlModel;
    MspCmdID    cmdFunc;
    I32_T       bShowType;	//0 : not use datebuff and showPos  1:only show bord     2: image   3: text   
    CHAR_T      dataBuff[96];		//ÍŒÆ¬Â·Ÿ¶»òÕßÎÄ±ŸÄÚÈÝ
    RECT      showPos;		//ÏÔÊŸÎ»ÖÃ    
} MspSendCmd_t;


typedef struct MsptReturnStruct
{
    MspCmdID    ctrlModel;
    MspCmdID    cmdFunc;
    U32_T       cmdDataLength;	    
} MsptReturnStruct_t;

//ÎÄŒþ±ž·Ý

typedef struct
{
    CHAR_T    dev_typename[32];  //Éè±žÀàÐÍ
    U8_T    partition_num; //·ÖÇøºÅ: ŽÓ1¿ªÊŒ
    CHAR_T    backup_path[64];
    ULONG_T total_space;//×ÜÈÝÁ¿   µ¥Î»×ÖœÚ
    ULONG_T available_space;//¿ÉÀûÓÃÈÝÁ¿ µ¥Î»×ÖœÚ
    U8_T    exist;//0----²»ŽæÔÚ£»1---ŽæÔÚ
} BACKUPDEVSUB_T;

#define MAX_DEV_NUM 16
typedef struct
{
    BACKUPDEVSUB_T dev[MAX_DEV_NUM];
    U8_T       total_dev_number;
} BACKUPDEV_T;

typedef enum
{
    CURRENT_WORK_DISK_NUM1 = 1,
    CURRENT_WORK_DISK_NUM2,
    CURRENT_WORK_DISK_FULL,//Ó²ÅÌÂú
    CURRENT_WORK_DISK_NULL,//Ã»ÓÐÓ²ÅÌ

} tagWorkDiskNum;

/*----------------------------------------------*
 * The common data type, will be used in the whole project.*
 *----------------------------------------------*/

typedef unsigned char           HI_U8;
typedef unsigned short          HI_U16;
typedef unsigned int            HI_U32;

typedef signed char             HI_S8;
typedef short                   HI_S16;
typedef int                     HI_S32;

#ifndef _M_IX86
    typedef unsigned long long  HI_U64;
    typedef long long           HI_S64;
#else
    typedef __int64             HI_U64;
    typedef __int64             HI_S64;
#endif

typedef char                    HI_CHAR;
#define HI_VOID                 void

/*----------------------------------------------*
 * const defination                             *
 *----------------------------------------------*/
typedef enum {
    HI_FALSE = 0,
    HI_TRUE  = 1,
} HI_BOOL;

#ifndef NULL
    #define NULL    0L
#endif

#define HI_NULL     0L
#define HI_SUCCESS  0
#define HI_FAILURE  (-1)

#endif

