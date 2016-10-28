/******************************************************************************
*********************
**文件:
**编写者:
**编写日期:2016年08月27号
**简要描述:
**修改者:
**修改日期:
*******************************************************************************
*********************/

#include "include/xw_export.h"



/******************************************************************************
****************************/

void XW_SignalDo_handleSignal(int s);
/******************************************************************************
*****************************
**函数:
**输入参数:
**功能:
**返回值:
*******************************************************************************
****************************/

void XW_SignalDo_InitSignal(void)
{
    signal(SIGINT, SIG_IGN);
    signal(SIGPIPE, XW_SignalDo_handleSignal);
}

/******************************************************************************
*****************************
**函数:
**输入参数:
**功能:
**返回值:
*******************************************************************************
****************************/
void XW_SignalDo_handleSignal(int s)
{
    printf("pipe:error\n");
    XW_SignalDo_InitSignal();
}

/******************************************************************************
*****************************
**函数:
**输入参数:
**功能:系统模块的初始化
**返回值:
*******************************************************************************
****************************/
int XW_SysModuleBoot(void)
{
    int retVal = 0;
    SPS_SYSTEM_INFO_T *DTSystemInfo = XW_Global_InitSystemInfo();
    /*初始化系统信息*/;
    //DTSystemInfo->

    // init modules
    XW_SignalDo_InitSignal();/*异常信号的处理*/
    XW_Tools_SetProcessPriority();/*设置线程的优先级和调度方式*/
    XW_ManagePthread_StateInit();/*线程管理的初始化*/

    XW_MsgQueue_Create();/*创建消息队列/bin/ls*/

    retVal = XW_Si3050_DAA_System_Init();/*设置Si3050系统初始化*/
    if(retVal < 0)
    {
        return retVal;
    }

    // Thread start run ...
    ManagePthread_UdpBroadcast();/*创建UDP 广播IP线程*/
    ManagePthread_ClientConnectManage();/*Client链接 管理线程*/
    ManagePthread_ClientApplicationManage();/*Client任务处理线程*/
    ManagePthread_ModemCtrlDeamon();/* Modem  处理线程*/
    
    //ManagePthread_SDCardManage();/*硬盘管理线程*/
    //ManagePthread_UsbDeviceHotplug();/*USB热插拔检测*/
    //XW_Update_CreateUpdatePthread();/* 升级处理线程 */

    return retVal;
}
/******************************************************************************
*****************************
**函数:
**功能:
**输入参数:系统模块的去初始化
**返回值:
*******************************************************************************
****************************/
void XW_SysModuleUnBoot(void)
{
    //U8_T  i = 0;
    PTHREAD_BUF m_signal;
    //DVR_SYSTEM_INFO_T *gSystemInfo = NULL;

    bzero(&m_signal, sizeof(PTHREAD_BUF));
    //gSystemInfo = XW_Global_GetSystemGlobalContext();/*获得系统的全局信息*/


    //XW_ManagePthread_SendExitSignal(PTHREAD_RECORD_SCHEDULE_ID, 0, false, true);/*停止录像调度线程*/

    _DEBUG("Stop UDP broadcast Thread...");
    XW_ManagePthread_SendExitSignal(PTHREAD_UDP_BROADCAST_ID, 0, true, true);
    
    _DEBUG("Stop Client Connection Thread...");
    XW_ManagePthread_SendExitSignal(PTHREAD_CLIENT_CONNECT_ID, 0, true, false);

    //_DEBUG("UnInit MPP System...");
    //XW_Global_SetSystemMPP_UnInit();/*设置MPP系统反初始化*/

    _DEBUG("Stop Client Manage Thread...");
    XW_ManagePthread_SendExitSignal(PTHREAD_CLIENT_MANAGE_ID, 0, true, false);

    _DEBUG("Stop Modem Control Thread...");
    XW_ManagePthread_SendExitSignal(PTHREAD_MODEM_CTRL_ID, 0, true, false);
    //XW_ManagePthread_SendExitSignal(PTHREAD_USBHOTPLUG_ID, 0, true, true);
    //XW_ManagePthread_SendExitSignal(PTHREAD_DISKM_ID, 0, true, true);
    //XW_ManagePthread_SendExitSignal(PTHREAD_UPDATE_ID, 0, true, true);

    printf("power off \n");
}



