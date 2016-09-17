/***************************************************************************************************
**�ļ�:
**��д��:http://sunshengquan.taobao.com/
**��д����:2012��01��02��
**��Ҫ����:
**�޸���:
**�޸�����:
****************************************************************************************************/
#ifndef _DOORDVR_CONFIG_H_
#define _DOORDVR_CONFIG_H_


//#define      CPU_TYPE	HI3515_V100
#define      CPU_TYPE HI3520D_V100

#define      CFG_RTC
#define	     CFG_IMAGEADJUEST
#define      CFG_PREVIEW
#define      CFG_VIDEOLOST
#define		 CFG_SCREEN
#define      CFG_PTZ
#define      CFG_OSD
#define      CFG_USB_HOTPLUGCHECK
#define      CFG_UPDATE
#define      CFG_DATABASE
#define      CFG_DISKMANAGE
#define      CFG_BUZZER
#define      CFG_WATCHDOG
#define      APP_VERSION "3.1"
#define      HARDWARE    "3.1"
#define      RELEASE    "2010-10-10"

//----------------------------------------------------------------------------------------------------------------

#if 0
typedef struct __DVR_SYSTEM_INFO_T_
{
	pthread_rwlock_t	gSysInfo_rwlock; 	// Lock for this struct
	PTHREAD_HIFB_INIT *fbInfo;
	HI_BOOL bDebugOpen;
	
	HI_BOOL PreviewStart;
    HI_BOOL ChnDisplaystatus[8];   //��ʾͨ���Ƿ���ʾ��״̬
	RECT_S  PreviewPos[8];			//��ʾͨ��������

	HI_BOOL bShowChn[8];
	RECT_S  ChnNamePos[8];			//ͨ�����Ƶ�����

	HI_BOOL bShowFlag[8];
	RECT_S  FlrtopPos[8];			// LT RT TOP      FLT FRT FTOP     ������

    HI_BOOL DayNight_bShow;
    HI_BOOL Day_Night_Status;		//TRUE: ��ʾDAY    FALSE:��ʾNIGHT
	RECT_S  Day_Night_Pos;			//��ʾ����

    HI_BOOL IROnOff_bShow;
	HI_BOOL IR_OnOff_Status;
	RECT_S  IR_OnOff_Pos;

	HI_S32 Brightness_Value;
	HI_BOOL Brightness_Status;
	RECT_S  Brightness_Pos;

	HI_BOOL bShow_ParkingLine;
	HI_BOOL LT_ParkingLine_Status;
	RECT_S  LT_ParkingLine_Pos;

	HI_BOOL RT_ParkingLine_Status;
	RECT_S  RT_ParkingLine_Pos;

	HI_BOOL TOP_ParkingLine_Status;
	RECT_S  TOP_ParkingLine_Pos;	
	
} DVR_SYSTEM_INFO_T;
#endif
/**
 * ͨ������ʱ��ṹ.
 * @note ��Ӧ������
 * @see TIMECMD_TIME_SET
 * @see TIMECMD_TIME_GET
 */
typedef struct tagDateTimeDef
{
    U32_T      year;
    U32_T      month;
    U32_T      day;
    U32_T      hour;
    U32_T      minute;
    U32_T      second;
    U32_T      millisecond;
} DateTimeDef_t, *LPDateTimeDef_t;
/**
 * ������ʱ��ṹ.
 *  ʱ��Hour:bit31-16(0-23)
 *  Minute: bit 15-8 (0-59)
 *  Second: bit //7-0 (0-59)
 */

typedef union tagTime
{

    U32_T ulTime;

    struct
    {
        U16_T Hour;
        U8_T Minute;
        U8_T Second;
    } TimeFieldDef;

} TimeDef_t, *LPTimeDef_t;

//----------------------------------------------------------------------------------------------------------------
/**
 * ��̨����.
 * ��ͨ��
 * @note ��Ӧ������
 * @see PTZCMD_PARAM_GET
 * @see PTZCMD_PARAM_SET
 */
#define MAX_PRESET 16
typedef struct tagPTZParam
{
    U32_T      BaudRate;           //������ 1200 2400 4800 9600
    U8_T       DataBits;           //����λ 5-8
    U8_T       StopBits;           //ֹͣλ 0:1  1:1.5  2:2
    U8_T       ParityBits;         //��żУ��λ 0-None 1-ODD 2-EVEN 3-MARK
    U8_T       FlowControl;        //����0���ޣ�1��������2��Ӳ����
    U16_T      ProtocolIdx;        //Э��������֧�ֵ�����Э���е�λ��,��Ӧ�±�
    //0Ϊ͸��
    U16_T      Address;            //��ַ��
    U8_T       Preset[MAX_PRESET]; //Ԥ��λMAX_PRESET=128
    U8_T       Cruise[MAX_PRESET]; //Ѳ��MAX_PRESET=128
    U8_T       Track[MAX_PRESET];  //�켣MAX_PRESET=128
    U16_T      Speed;              //��̨�ٶ�
    U16_T      Reserved;
} PTZParam_t, *LPPTZParam_t;
/**
* ��̨����ṹ
*
* @note ��Ӧ������
* @see PTZCMD_CMDSEND
*/
typedef struct
{
    U16_T     channel;            //ͨ����
    U16_T     cmd;                //������
    U32_T     speed;              //�ٶ�
    U32_T     preset;             //Ԥ��λֵ
    U32_T     len;                //͸�����ݳ���
    U8_T      data[64];           //͸������
} PTZCmdParam_t, *LPPTZCmdParam;
/**
*��̨����
**/
typedef struct tagPTZSet_T
{
    PTZParam_t ptzparam[DOORDVR_CHANNEL_NUM];
} PTZSet_t, *pPTZSet_t;
//----------------------------------------------------------------------------------------------------------------

typedef struct tagCommonSet_T
{
    DateTimeDef_t system_date;      //ϵͳʱ��
    U8_T      machine_num[32];  //������ţ� �31λ
    U8_T      date_format;      //���ڸ�ʽ ��0----��:��:��,   1 ----��:��:�� ; 2-----��:��:��
    U8_T      time_format;     //0---24Сʱ��1---12Сʱ
    U8_T      language;         //����ѡ��   0----�У�1----Ӣ������������
    U8_T      video_format;     //��Ƶ��ʽ   0---NTSC; 1----PAL
    U32_T      reserve;          //����
} CommonSet_t, *pCommonSet_t;

/**
*�汾��Ϣ
*/
typedef struct tagVersionInfo_T
{
    U8_T software_version[8];   //����汾-------�7λ
    U8_T mcu_version[8];        // MCU�汾-------�7λ
    U8_T hardware_version[8];   //Ӳ���汾-------�7λ
    U8_T release_date[32];      //��������-------�31λ
    U8_T extern_info[32];       //��չ��Ϣ-------�31λ

} VersionInfo_t, *pVersionInfo_t;


typedef struct tagSystemInfo_T
{
    VersionInfo_t versioninfo;
} SystemInfo_t, *pSystemInfo_t;

//----------------------------------------------------------------------------------------------------------------
/**
*�������
**/
typedef struct tagOutputadjust_T
{
    I8_T  vga_brightness;        //-127 ---- +127
    U8_T  vga_contrast;          //0----255
    U8_T  vga_saturation;        // 0 ---- 255
    U8_T  vga_resolution_ratio;  //0---800*600   1----- 1024*768  2------1280x1024  :default:1024*768
    U8_T  output_volume;         //0----15
    U8_T  reserve[3];
} Outputadjust_t, *pOutputadjust_t;

//������ʾ
typedef  struct
{
    U8_T channel_name[DOORDVR_CHANNEL_NUM][32];
} LocalDisplay_T;
//----------------------------------------------------------------------------------------------------------------

#endif
