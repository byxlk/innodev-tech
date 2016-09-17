#include "include/xw_export.h"


/******************************************************************************
*****************************/
typedef struct __MANAGEPTHREAD_T
{
    pthread_t          g_pthread[PTHREAD_MAX_ID];
    pthread_mutex_t    g_pthread_mutex[PTHREAD_MAX_ID];
    pthread_cond_t     g_pthread_cond[PTHREAD_MAX_ID];
    PTHREAD_SIG_COND   g_pthread_info[PTHREAD_MAX_ID];
    PTHREAD_BUF        g_pthread_buf[PTHREAD_MAX_ID];
    PTHREAD_STATE      g_pthread_state;
} MANAGEPTHREAD_T;



static MANAGEPTHREAD_T managePthread;


/******************************************************************************
*****************************
**函数:
**功能:
**输入参数:
**返回值:
*******************************************************************************
****************************/
RETURN XW_ManagePthread_InitializeMutex(void)
{
    int i;

    for(i = 0; i < PTHREAD_MAX_ID; i++)
    {
        if((pthread_mutex_init(&managePthread.g_pthread_mutex[i], NULL)) != 0)
        {
            _ERROR("\t!!! ERROR : pthread_mutex_init !!!\n");
            return FAILURE;
        }
    }
    return SUCCESS;
}
/******************************************************************************
*****************************
**函数:
**功能:
**输入参数:
**返回值:
*******************************************************************************
****************************/
RETURN XW_ManagePthread_DestoryMutex(void)
{
    int ii = 0;

    for(ii = 0; ii < PTHREAD_MAX_ID; ii++)
    {
        if((pthread_mutex_destroy(&managePthread.g_pthread_mutex[ii])) != 0)
        {
            _ERROR("\t!!! ERROR : pthread_mutex_destroy !!!\n");
            return FAILURE;
        }
    }
    return SUCCESS;
}


/******************************************************************************
*****************************
**函数:
**功能:
**输入参数:
**返回值:
*******************************************************************************
****************************/
RETURN XW_ManagePthread_InitializeCond(void)
{
    int i = 0;

    for(i = 0; i < PTHREAD_MAX_ID; i++)
    {
        if((pthread_cond_init(&managePthread.g_pthread_cond[i], NULL)) != 0)
        {
            _ERROR("\t!!! ERROR : pthread_cond_init !!!\n");
            return FAILURE;
        }
    }
    return SUCCESS;
}
/******************************************************************************
*****************************
**函数:
**功能:
**输入参数:
**返回值:
*******************************************************************************
****************************/
RETURN XW_ManagePthread_DestroyCond(void)
{
    int ii;

    for(ii = 0; ii < PTHREAD_MAX_ID; ii++)
    {
        if((pthread_cond_destroy(&managePthread.g_pthread_cond[ii])) != 0)
        {
            _ERROR("\t!!! ERROR : pthread_cond_destroy !!!\n");
            return FAILURE;
        }
    }
    return SUCCESS;
}

/******************************************************************************
*****************************
**函数:
**功能:
**输入参数:
**返回值:
*******************************************************************************
****************************/
void XW_ManagePthread_StateInit(void)
{
    memset(&managePthread, 0, sizeof(MANAGEPTHREAD_T));
    XW_ManagePthread_InitializeMutex();
    XW_ManagePthread_InitializeCond();
}


void *XW_ManagePthread_GetPthreadState(U8_T pthread_id, U8_T channel)
{
    switch(pthread_id)
    {
    case PTHREAD_MAIN_ID:
        return &managePthread.g_pthread_state.state_main;
        break;
    case PTHREAD_MODEM_CTRL_ID:
        return &managePthread.g_pthread_state.state_serial2;
        break;
    case PTHREAD_CLIENT_MANAGE_ID:
        return &managePthread.g_pthread_state.state_serial3;
        break;
    case PTHREAD_CLIENT_CONNECT_ID:
        return &managePthread.g_pthread_state.state_videolost;
        break;    
    case PTHREAD_UDP_BROADCAST_ID:
        return &managePthread.g_pthread_state.state_ptz;
        break;    
    case PTHREAD_UPDATE_ID:
        return &managePthread.g_pthread_state.state_osddisplay;
        break;   
    case PTHREAD_USBHOTPLUG_ID:
        return &managePthread.g_pthread_state.state_usbhotplug;
        break;
    case PTHREAD_DISKM_ID:
        return &managePthread.g_pthread_state.state_diskm;
        break;
    default:
        return NULL;
        break;
    }
    return NULL;
}


/******************************************************************************
*****************************
**函数:
**功能:
**输入参数:
**返回值:
*******************************************************************************
****************************/
bool XW_ManagePthread_ReadSignal(PTHREAD_BUF *rbuf, PTHREAD_ID thread_num, 
bool wait)
{
    bool    b_valid = false;

    pthread_mutex_lock(&managePthread.g_pthread_mutex[thread_num]);
    if(wait == true)
        pthread_cond_wait(&managePthread.g_pthread_cond[thread_num], &
managePthread.g_pthread_mutex[thread_num]);

    if(managePthread.g_pthread_info[thread_num] == WRITE_FLAG)
    {
        managePthread.g_pthread_info[thread_num] = READ_FLAG;
        memcpy((PTHREAD_BUF *)rbuf, (PTHREAD_BUF *)&managePthread.
g_pthread_buf[thread_num], sizeof(PTHREAD_BUF));
        b_valid = true;
    }
    pthread_mutex_unlock(&managePthread.g_pthread_mutex[thread_num]);
    return b_valid;
}

/******************************************************************************
*****************************
**函数:
**功能:
**输入参数:
**返回值:
*******************************************************************************
****************************/
bool XW_ManagePthread_SendSignal(PTHREAD_BUF *sbuf, PTHREAD_ID thread_num)
{
    bool    b_valid = false;

    pthread_mutex_lock(&managePthread.g_pthread_mutex[thread_num]);
    managePthread.g_pthread_info[thread_num] = WRITE_FLAG;
    if(sbuf)
    {
        memcpy((PTHREAD_BUF *)&managePthread.g_pthread_buf[thread_num], (
PTHREAD_BUF *)sbuf, sizeof(PTHREAD_BUF));
    }
    pthread_mutex_unlock(&managePthread.g_pthread_mutex[thread_num]);
    pthread_cond_signal(&managePthread.g_pthread_cond[thread_num]);
    b_valid = true;
    return b_valid;
}


/******************************************************************************
*****************************
  **函数:
  **输入参数:
  **功能:
  **返回值:
  
*******************************************************************************
****************************/
void XW_ManagePthread_SendExitSignal(U8_T pthread_id, U8_T channel, bool 
isSend, bool block)
{
	/*发送停止线程信号*/

    PTHREAD_BUF m_signal;
    //U8_T i = 10;
    STATE_PTHREAD *p;
    p = ((STATE_PTHREAD *)XW_ManagePthread_GetPthreadState(pthread_id, channel
));
    if(p->state == ALIVE)
    {
        p->power = PTHREAD_POWER_OFF;
        m_signal.start_id = PTHREAD_MAIN_ID;
        m_signal.m_signal = EXIT;
        if(isSend)
        {
            //if(PTHREAD_MOTION_ID == pthread_id)
            //{
            //    m_signal.m_value = EXIT;
            //}
            if(XW_ManagePthread_SendSignal(&m_signal, pthread_id) == false)
            {
                _ERROR("PTHREAD_SNAPJPEG[%d] error !'\n", pthread_id);
            }
        }

        if(block)
        {
            //while((i > 0) && (p->state == ALIVE));
            //{
            //    p->power = PTHREAD_POWER_OFF;
            //    sleep(2);
            //    i--;
            //}
            if(XW_ManagePthread_ReadSignal(&m_signal, pthread_id,true) == 
false)
            {
                _ERROR("PTHREAD_SNAPJPEG[%d] error !'\n", pthread_id);
            }
        }
    }
}






/******************************************************************************
*****************************
**函数:
**功能:
**输入参数:
**返回值:
*******************************************************************************
****************************/

int ManagePthread_UdpBroadcast(void)
{
    _DEBUG("Start pthread: XW_Preview_pthread");
    if(pthread_create(&managePthread.g_pthread[PTHREAD_UDP_BROADCAST_ID] , 
			NULL,(void * ( *)(void *)) XW_Pthread_Udp_Broadcast, NULL) != 0)
    {
        return FAILURE;
    }

    return 0;
}


/*Client链接 管理线程*/
int ManagePthread_ClientConnectManage(void)
{
	/*串口通信管理*/
	_DEBUG("Start pthread: XW_SerialCommunication_pthread");
    if(pthread_create(&managePthread.g_pthread[PTHREAD_CLIENT_CONNECT_ID] , 
			NULL, (void * ( *)(void *)) XW_Pthread_ClientConnectManage, NULL) != 0)
    {
        return FAILURE;
    }

	return 0;
}

/*Client任务处理线程*/
int ManagePthread_ClientApplicationManage(void)
{
	/*视频信号丢失时叠加无视频信号图片*/
	_DEBUG("Start pthread: XW_VideoLostCheck_pthread");
    if(pthread_create(&managePthread.g_pthread[PTHREAD_CLIENT_MANAGE_ID] , NULL, 
        (void * ( *)(void *)) XW_Pthread_ClientApplicationManage, NULL) != 0)
    {
        return FAILURE;
    }

	return 0;
}

/* Modem  处理线程*/
int ManagePthread_ModemCtrlDeamon(void)
{
	/*OSD叠加图片管理*/
	_DEBUG("Start pthread: XW_OsdDisplay_pthread");
    if(pthread_create(&managePthread.g_pthread[PTHREAD_MODEM_CTRL_ID] , NULL, 
        (void * ( *)(void *)) XW_Pthread_ModemCtrlDeamon, NULL) != 0)
    {
        return FAILURE;
    }

	return 0;
}


int ManagePthread_UsbDeviceHotplug(void)
{
    _DEBUG("Start pthread: XW_UsbDeviceHotplug_Pthread");
    if(pthread_create(&managePthread.g_pthread[PTHREAD_USBHOTPLUG_ID] , NULL, 
        (void * ( *)(void *)) XW_UsbDeviceHotplug_Pthread, NULL) != 0) 
/*等待客户端 连接*/
    {
        _ERROR("create XW_Hotplug_Pthread pthread error !");
        return FAILURE;
    }
    
    return 0;
}

int ManagePthread_SDCardManage(void)
{
    _DEBUG("Start pthread: XW_Disk_pthreadDiskManage");
#ifdef CFG_DISKMANAGE
    if(pthread_create(&managePthread.g_pthread[PTHREAD_DISKM_ID] , NULL, 
        (void * ( *)(void *)) XW_Disk_pthreadDiskManage, NULL) != 0) 
/*等待客户端 连接*/
    {
        _ERROR("create XW_Disk_pthreadDiskManage pthread error !");
        return FAILURE;
    }
#endif
    return 0;
}


/* 升级处理线程 */
int XW_Update_CreateUpdatePthread(void)
{
    _DEBUG("Start pthread: XW_Update_pthread");    
    if(pthread_create(&managePthread.g_pthread[PTHREAD_UPDATE_ID] , NULL, 
        (void * ( *)(void *)) XW_Update_pthread, NULL) != 0) /*等待客户端连接*/
    {
        _ERROR("create XW_Update_CreateUpdatePthread pthread error !");
        return FAILURE;
    }

    return 0;
}


