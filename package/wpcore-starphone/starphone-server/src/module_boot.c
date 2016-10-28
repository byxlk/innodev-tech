/******************************************************************************
*********************
**�ļ�:
**��д��:
**��д����:2016��08��27��
**��Ҫ����:
**�޸���:
**�޸�����:
*******************************************************************************
*********************/

#include "include/xw_export.h"



/******************************************************************************
****************************/

void XW_SignalDo_handleSignal(int s);
/******************************************************************************
*****************************
**����:
**�������:
**����:
**����ֵ:
*******************************************************************************
****************************/

void XW_SignalDo_InitSignal(void)
{
    signal(SIGINT, SIG_IGN);
    signal(SIGPIPE, XW_SignalDo_handleSignal);
}

/******************************************************************************
*****************************
**����:
**�������:
**����:
**����ֵ:
*******************************************************************************
****************************/
void XW_SignalDo_handleSignal(int s)
{
    printf("pipe:error\n");
    XW_SignalDo_InitSignal();
}

/******************************************************************************
*****************************
**����:
**�������:
**����:ϵͳģ��ĳ�ʼ��
**����ֵ:
*******************************************************************************
****************************/
int XW_SysModuleBoot(void)
{
    int retVal = 0;
    SPS_SYSTEM_INFO_T *DTSystemInfo = XW_Global_InitSystemInfo();
    /*��ʼ��ϵͳ��Ϣ*/;
    //DTSystemInfo->

    // init modules
    XW_SignalDo_InitSignal();/*�쳣�źŵĴ���*/
    XW_Tools_SetProcessPriority();/*�����̵߳����ȼ��͵��ȷ�ʽ*/
    XW_ManagePthread_StateInit();/*�̹߳���ĳ�ʼ��*/

    XW_MsgQueue_Create();/*������Ϣ����/bin/ls*/

    retVal = XW_Si3050_DAA_System_Init();/*����Si3050ϵͳ��ʼ��*/
    if(retVal < 0)
    {
        return retVal;
    }

    // Thread start run ...
    ManagePthread_UdpBroadcast();/*����UDP �㲥IP�߳�*/
    ManagePthread_ClientConnectManage();/*Client���� �����߳�*/
    ManagePthread_ClientApplicationManage();/*Client�������߳�*/
    ManagePthread_ModemCtrlDeamon();/* Modem  �����߳�*/
    
    //ManagePthread_SDCardManage();/*Ӳ�̹����߳�*/
    //ManagePthread_UsbDeviceHotplug();/*USB�Ȳ�μ��*/
    //XW_Update_CreateUpdatePthread();/* ���������߳� */

    return retVal;
}
/******************************************************************************
*****************************
**����:
**����:
**�������:ϵͳģ���ȥ��ʼ��
**����ֵ:
*******************************************************************************
****************************/
void XW_SysModuleUnBoot(void)
{
    //U8_T  i = 0;
    PTHREAD_BUF m_signal;
    //DVR_SYSTEM_INFO_T *gSystemInfo = NULL;

    bzero(&m_signal, sizeof(PTHREAD_BUF));
    //gSystemInfo = XW_Global_GetSystemGlobalContext();/*���ϵͳ��ȫ����Ϣ*/


    //XW_ManagePthread_SendExitSignal(PTHREAD_RECORD_SCHEDULE_ID, 0, false, true);/*ֹͣ¼������߳�*/

    _DEBUG("Stop UDP broadcast Thread...");
    XW_ManagePthread_SendExitSignal(PTHREAD_UDP_BROADCAST_ID, 0, true, true);
    
    _DEBUG("Stop Client Connection Thread...");
    XW_ManagePthread_SendExitSignal(PTHREAD_CLIENT_CONNECT_ID, 0, true, false);

    //_DEBUG("UnInit MPP System...");
    //XW_Global_SetSystemMPP_UnInit();/*����MPPϵͳ����ʼ��*/

    _DEBUG("Stop Client Manage Thread...");
    XW_ManagePthread_SendExitSignal(PTHREAD_CLIENT_MANAGE_ID, 0, true, false);

    _DEBUG("Stop Modem Control Thread...");
    XW_ManagePthread_SendExitSignal(PTHREAD_MODEM_CTRL_ID, 0, true, false);
    //XW_ManagePthread_SendExitSignal(PTHREAD_USBHOTPLUG_ID, 0, true, true);
    //XW_ManagePthread_SendExitSignal(PTHREAD_DISKM_ID, 0, true, true);
    //XW_ManagePthread_SendExitSignal(PTHREAD_UPDATE_ID, 0, true, true);

    printf("power off \n");
}



