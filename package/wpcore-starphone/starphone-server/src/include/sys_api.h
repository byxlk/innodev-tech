#ifndef __SYS_API_H__
#define __SYS_API_H__

#include "common.h"
#include "xw_config.h"



SPS_SYSTEM_INFO_T  *XW_Global_InitSystemInfo(void);


int XW_SysModuleBoot(void);
void XW_SysModuleUnBoot(void);

// Commmon Interface
char XW_Tools_GetTtyInputChar(void);
bool XW_Tools_DisAllOpenedFile(int pid);
int	XW_Tools_DvrCpFile(char *srcFile, char *targetFile, bool findString, char *string);
void kill_uiprocess(void);
I32_T XW_Tools_getpidbyname(const CHAR_T *name, int pos);
int  XW_Tools_SetProcessPriority(void);
void Lock(bool block_init, pthread_mutex_t *lock);
void UnLock(pthread_mutex_t *lock);
int XW_Tools_dirIsNull(char *dir_path);
time_t	XW_Tools_getCurrentTime(void);
U32_T XW_Tools_fileLen(char *fileName);
void XW_Tools_delayMs(int ms);
int mymax(int a, int b);


//Msg Query
I32_T XW_MsgQueue_Create(void);
void XW_MsgQueue_Send(MspCmdID ctrlModel, MspSendCmd_t *cmdData, long message_type);
void XW_MsgQueue_SelfSend(MspSendCmd_t  *pcmd, MspCmdID cid, U8_T *cmdData, U32_T cmdDataLength);
I32_T XW_MsgQueue_Receive(I32_T message_type);
CHAR_T *XW_MsgQueue_Receive_Data(void);


/* 线程相关 */
void XW_ManagePthread_StateInit(void);
void *XW_ManagePthread_GetPthreadState(U8_T pthread_id, U8_T channel);
bool XW_ManagePthread_ReadSignal(PTHREAD_BUF *rbuf, PTHREAD_ID thread_num, bool wait);
bool XW_ManagePthread_SendSignal(PTHREAD_BUF *sbuf, PTHREAD_ID thread_num);
void XW_ManagePthread_SendExitSignal(U8_T pthread_id, U8_T channel, bool isSend, bool block);

int ManagePthread_UdpBroadcast(void);
void *XW_Pthread_Udp_Broadcast(void *args);
int ManagePthread_ClientConnectManage(void);
void *XW_Pthread_ClientConnectManage(void *args);
int ManagePthread_ClientApplicationManage(void);
void *XW_Pthread_ClientApplicationManage(void *args);
int ManagePthread_UsbDeviceHotplug(void);
void *XW_UsbDeviceHotplug_Pthread(void *args);
int ManagePthread_SDCardManage(void);
void *XW_Disk_pthreadDiskManage(void *args);

int ManagePthread_ModemCtrlDeamon(void);


/*  时钟设置  */
I32_T  XW_Hwclock_RtcGetTime(DateTimeDef *dt);
I32_T  XW_Hwclock_RtcSetTime(CommonSet_t *dt);
void   XW_Sync_SystemTime(void);
void   XW_Timezone(void);


//usb hotplug
void *XW_Hotplug_Pthread(void *arg);
void  XW_Hotplug_UmountAllUsbDisk(void);
void XW_Hotplug_CheckUsbDiskDev(BACKUPDEV_T  *info);
void  XW_Hotplug_UmountAllUsbDisk(void);
bool XW_Hotplug_FindFileInUsb(char *FileName, char *fullPathName);
BACKUPDEV_T *XW_Hotplug_GetUsbInfo(void);

// update system
int XW_Update_CreateUpdatePthread(void);
void *XW_Update_pthread(void *argv);
bool XW_Update_GetPthreadUpdateState(void);


#endif
