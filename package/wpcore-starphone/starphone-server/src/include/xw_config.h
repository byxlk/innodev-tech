/***************************************************************************************************
**文件:
**编写者:http://sunshengquan.taobao.com/
**编写日期:2012年01月02号
**简要描述:
**修改者:
**修改日期:
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
    HI_BOOL ChnDisplaystatus[8];   //显示通道是否显示的状态
	RECT_S  PreviewPos[8];			//显示通道的坐标

	HI_BOOL bShowChn[8];
	RECT_S  ChnNamePos[8];			//通道名称的坐标

	HI_BOOL bShowFlag[8];
	RECT_S  FlrtopPos[8];			// LT RT TOP      FLT FRT FTOP     的坐标

    HI_BOOL DayNight_bShow;
    HI_BOOL Day_Night_Status;		//TRUE: 显示DAY    FALSE:显示NIGHT
	RECT_S  Day_Night_Pos;			//显示坐标

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
 * 通用日期时间结构.
 * @note 对应命令字
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
 * 联合体时间结构.
 *  时间Hour:bit31-16(0-23)
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
 * 云台参数.
 * 单通道
 * @note 对应命令字
 * @see PTZCMD_PARAM_GET
 * @see PTZCMD_PARAM_SET
 */
#define MAX_PRESET 16
typedef struct tagPTZParam
{
    U32_T      BaudRate;           //波特率 1200 2400 4800 9600
    U8_T       DataBits;           //数据位 5-8
    U8_T       StopBits;           //停止位 0:1  1:1.5  2:2
    U8_T       ParityBits;         //奇偶校验位 0-None 1-ODD 2-EVEN 3-MARK
    U8_T       FlowControl;        //流控0：无，1：软流控2：硬流控
    U16_T      ProtocolIdx;        //协议在内置支持的所有协议中的位置,对应下标
    //0为透传
    U16_T      Address;            //地址码
    U8_T       Preset[MAX_PRESET]; //预置位MAX_PRESET=128
    U8_T       Cruise[MAX_PRESET]; //巡航MAX_PRESET=128
    U8_T       Track[MAX_PRESET];  //轨迹MAX_PRESET=128
    U16_T      Speed;              //云台速度
    U16_T      Reserved;
} PTZParam_t, *LPPTZParam_t;
/**
* 云台命令结构
*
* @note 对应命令字
* @see PTZCMD_CMDSEND
*/
typedef struct
{
    U16_T     channel;            //通道号
    U16_T     cmd;                //命令字
    U32_T     speed;              //速度
    U32_T     preset;             //预置位值
    U32_T     len;                //透传数据长度
    U8_T      data[64];           //透传数据
} PTZCmdParam_t, *LPPTZCmdParam;
/**
*云台设置
**/
typedef struct tagPTZSet_T
{
    PTZParam_t ptzparam[DOORDVR_CHANNEL_NUM];
} PTZSet_t, *pPTZSet_t;
//----------------------------------------------------------------------------------------------------------------

typedef struct tagCommonSet_T
{
    DateTimeDef_t system_date;      //系统时间
    U8_T      machine_num[32];  //机器编号； 最长31位
    U8_T      date_format;      //日期格式 ；0----年:月:日,   1 ----月:日:年 ; 2-----日:月:年
    U8_T      time_format;     //0---24小时；1---12小时
    U8_T      language;         //语言选择   0----中，1----英。。。。。。
    U8_T      video_format;     //视频制式   0---NTSC; 1----PAL
    U32_T      reserve;          //保留
} CommonSet_t, *pCommonSet_t;

/**
*版本信息
*/
typedef struct tagVersionInfo_T
{
    U8_T software_version[8];   //软件版本-------最长7位
    U8_T mcu_version[8];        // MCU版本-------最长7位
    U8_T hardware_version[8];   //硬件版本-------最长7位
    U8_T release_date[32];      //发布日期-------最长31位
    U8_T extern_info[32];       //扩展信息-------最长31位

} VersionInfo_t, *pVersionInfo_t;


typedef struct tagSystemInfo_T
{
    VersionInfo_t versioninfo;
} SystemInfo_t, *pSystemInfo_t;

//----------------------------------------------------------------------------------------------------------------
/**
*输出调节
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

//本地显示
typedef  struct
{
    U8_T channel_name[DOORDVR_CHANNEL_NUM][32];
} LocalDisplay_T;
//----------------------------------------------------------------------------------------------------------------

#endif
