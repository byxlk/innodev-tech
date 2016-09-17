
#include "include/xw_export.h"

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <errno.h>
#include <sys/mount.h>


/***********************************************************************************************************
**函数:
**输入参数:
**功能:
**返回值:
***********************************************************************************************************/
#define ADD_MODULES 0
#define REMOVE_MODULES  1
#define UEVENT_BUFFER_SIZE 2048
typedef struct __MANAGEUSB_T
{
    BACKUPDEV_T  usbInfo;
    pthread_mutex_t  usb_lock;
    bool init_mutexLock;
} MANAGEUSB_T;
static MANAGEUSB_T manageUsb = {.init_mutexLock = false};
/***********************************************************************************************************
**函数:
**输入参数:
**功能:
**返回值:
***********************************************************************************************************/
BACKUPDEV_T *XW_Hotplug_GetUsbInfo(void)
{
    return &manageUsb.usbInfo;
}
/************************************************************************************************
**
**
************************************************************************************************/
static int XW_Hotplug_initSock(void)
{
    struct sockaddr_nl snl;
    const int buffersize = 16 * 1024;
    int retval;
    memset(&snl, 0x00, sizeof(struct sockaddr_nl));
    snl.nl_family = AF_NETLINK;
    snl.nl_pid = getpid();
    snl.nl_groups = 1;
    int hotplug_sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT/*15*/);
    if(hotplug_sock == -1)
    {
        _DEBUG("error getting socket: %s", strerror(errno));
        return -1;
    }
    setsockopt(hotplug_sock, SOL_SOCKET, SO_RCVBUF/*33*/, &buffersize, sizeof(buffersize));
    retval = bind(hotplug_sock, (struct sockaddr *) &snl, sizeof(struct sockaddr_nl));
    if(retval < 0)
    {
        _DEBUG("bind failed: %s", strerror(errno));
        close(hotplug_sock);
        hotplug_sock = -1;
        return -1;
    }
    return hotplug_sock;
}

/************************************************************************************************
**
**
************************************************************************************************/
static bool XW_Hotplug_CheckPartitionHasBeenExist(char *name, U8_T *pos)
{
    bool find = false;
    U8_T i;
    char  buffer[32];
    I32_T ret;
    size_t length = strlen(name);
    for(i = 0; i < MAX_DEV_NUM; i++)
    {
        sprintf(buffer, "%s%d", manageUsb.usbInfo.dev[i].dev_typename, manageUsb.usbInfo.dev[i].partition_num);
        ret = memcmp((void *)name, (void *)buffer, length);
        if(ret == 0 && manageUsb.usbInfo.dev[i].exist)
        {
            if(pos != NULL)
            {
                *pos = i;
            }
            _DEBUG("found %s has been mounted ", buffer);
            find = true;
            break;
        }
    }
    return find;
}
/************************************************************************************************
**
**
************************************************************************************************/
static void  XW_Hotplug_AddUsbInfoRec(BACKUPDEVSUB_T  info)
{
#ifdef CFG_DISKMANAGE
    U8_T i;
    char  path[64];
    char  devPath[64];
    I32_T ret;
    unsigned long tsize, fsize,  asize;

    for(i = 0; i < MAX_DEV_NUM; i++)
    {
        if(!manageUsb.usbInfo.dev[i].exist)
        {
            bzero(&manageUsb.usbInfo.dev[i], sizeof(BACKUPDEVSUB_T));
            memcpy(&manageUsb.usbInfo.dev[i], &info, sizeof(BACKUPDEVSUB_T));
            sprintf(path, "/var/%s%d", info.dev_typename, info.partition_num);
            sprintf(devPath, "/dev/%s%d", info.dev_typename, info.partition_num);
            _DEBUG("mount %s %s ", devPath, path);
            if(access(path, F_OK) != 0)
            {
                ret = mkdir(path, 0666);
                if(ret != 0)
                {
                    _ERROR("error :create dir %s ", path);
                }
            }

            ret = XW_DiskTools_CheckMountedDisk(path);
            if(ret)
            {
                umount(path);
            }
            sleep(1);// must use sleep
            ret = mount(devPath, path, "vfat", MS_NOATIME | MS_NODIRATIME, "iocharset=utf8");            
            if(ret >= 0)
            {
                sprintf(manageUsb.usbInfo.dev[i].backup_path, "%s", path);
                XW_DiskTools_GetMountSpaceInfo(path, &tsize, &fsize, &asize, B_UNIT);
                manageUsb.usbInfo.dev[i].available_space = asize;
                manageUsb.usbInfo.dev[i].total_space = tsize;
                manageUsb.usbInfo.total_dev_number++;
            }
            else
            {
                _ERROR("[ErrNo: %d] mount %s to %s", ret, devPath, path);  
                perror("mount:");
                memset(&manageUsb.usbInfo.dev[i], 0, sizeof(BACKUPDEVSUB_T));
            }
            break;
        }
    }
#endif
}
/***********************************************************************************************************
**函数:
**输入参数:
**功能:
**返回值:
***********************************************************************************************************/

void XW_Hotplug_ClearInvaildDev(void)
{
#ifdef CFG_DISKMANAGE
    U8_T i;
    char buffer[32];

    for(i = 0; i < MAX_DEV_NUM; i++)
    {
        if(manageUsb.usbInfo.dev[i].exist)
        {
            sprintf(buffer, "/dev/%s", manageUsb.usbInfo.dev[i].dev_typename);
            if(XW_DiskTools_checkDeviceIsRemoved(buffer))
            {
                umount(manageUsb.usbInfo.dev[i].backup_path);
                bzero(&manageUsb.usbInfo.dev[i], sizeof(BACKUPDEVSUB_T));
                manageUsb.usbInfo.total_dev_number--;
            }
        }
    }
#endif
}
/***********************************************************************************************************
**函数:
**输入参数:
**功能:
**返回值:
***********************************************************************************************************/

void XW_Hotplug_CheckUsbDiskDev(BACKUPDEV_T  *info)
{
    BACKUPDEV_T  tmpInfo;
    U8_T i;
    char buffer[32];
    bool ret;

    Lock(manageUsb.init_mutexLock, &manageUsb.usb_lock);
    bzero(&tmpInfo, sizeof(BACKUPDEV_T));
#ifdef CFG_DISKMANAGE
    XW_Disk_CheckDev(&tmpInfo);
#endif
    XW_Hotplug_ClearInvaildDev();
    for(i = 0; i < MAX_DEV_NUM; i++)
    {
        if(tmpInfo.dev[i].exist)
        {
            sprintf(buffer, "%s%d", tmpInfo.dev[i].dev_typename, tmpInfo.dev[i].partition_num);
            _DEBUG("XW_P2pNat_check dev %s", buffer);
            ret = XW_Hotplug_CheckPartitionHasBeenExist(buffer, NULL);
            if(ret == false)
            {
                _DEBUG("add new dev");
                XW_Hotplug_AddUsbInfoRec(tmpInfo.dev[i]);
            }
        }
    }

    if(info != NULL)
    {
        memcpy(info, &tmpInfo, sizeof(BACKUPDEV_T));
        {
            for(i = 0; i < MAX_DEV_NUM; i++)
            {
                if(info->dev[i].exist)
                {
                    _DEBUG("total_space =%ld", info->dev[i].total_space);
                    _DEBUG("available_space =%ld", info->dev[i].available_space);
                }
            }
        }
    }
    UnLock(&manageUsb.usb_lock);
    _DEBUG("total dev is %d ", manageUsb.usbInfo.total_dev_number);
}
/***********************************************************************************************************
**函数:
**输入参数:
**功能:
**返回值:
***********************************************************************************************************/

static void  XW_Hotplug_UmountUsbDisk(char  *name)
{
    bool ret;
    I32_T tmp;
    U8_T  pos = 0;
    CHAR_T buffer[64];

    Lock(manageUsb.init_mutexLock, &manageUsb.usb_lock);
    
    _DEBUG("check umount device name: %s",name);
    ret = XW_Hotplug_CheckPartitionHasBeenExist(name, &pos);
    if(ret)
    {
        sprintf(buffer, "%s", manageUsb.usbInfo.dev[pos].backup_path);
        _DEBUG("umount device: %s",buffer);
        tmp = umount(buffer);
        {
            _DEBUG("success umount path %s", buffer);
            tmp = rmdir(buffer);
            if(tmp < 0)
            {
                _ERROR("error:delete  %s", buffer);
            }
            else
            {
                _DEBUG("success delete %s", buffer);
            }
        }
        manageUsb.usbInfo.total_dev_number--;
        memset(&manageUsb.usbInfo.dev[pos], 0, sizeof(BACKUPDEVSUB_T));
    }
    
    UnLock(&manageUsb.usb_lock);
}
/***********************************************************************************************************
**函数:
**输入参数:
**功能:
**返回值:
***********************************************************************************************************/

void  XW_Hotplug_UmountAllUsbDisk(void)
{
    U8_T i;
    I32_T tmp;

    Lock(manageUsb.init_mutexLock, &manageUsb.usb_lock);
    for(i = 0; i < MAX_DEV_NUM; i++)
    {
        if(manageUsb.usbInfo.dev[i].exist)
        {
            if(access(manageUsb.usbInfo.dev[i].backup_path, F_OK) == 0)
            {
                tmp = umount(manageUsb.usbInfo.dev[i].backup_path);
                if(tmp < 0)
                {
                    _ERROR("umount path %s   error", manageUsb.usbInfo.dev[i].backup_path);
                }
                else
                {
                    _DEBUG("success umount path %s", manageUsb.usbInfo.dev[i].backup_path);
                    tmp = rmdir(manageUsb.usbInfo.dev[i].backup_path);
                    if(tmp < 0)
                    {
                        _ERROR("error:delete  %s", manageUsb.usbInfo.dev[i].backup_path);
                    }
                    else
                    {
                        _DEBUG("success delete %s", manageUsb.usbInfo.dev[i].backup_path);
                    }
                }
                manageUsb.usbInfo.total_dev_number--;
                memset(&manageUsb.usbInfo.dev[i], 0, sizeof(BACKUPDEVSUB_T));
            }
        }
    }
    UnLock(&manageUsb.usb_lock);
}
/***********************************************************************************************************
**函数:
**输入参数:
**功能:
**返回值:
***********************************************************************************************************/

bool XW_Hotplug_FindFileInUsb(char *FileName, char *fullPathName)
{
    U8_T i;

    Lock(manageUsb.init_mutexLock, &manageUsb.usb_lock);
    for(i = 0; i < MAX_DEV_NUM; i++)
    {
        if(manageUsb.usbInfo.dev[i].exist)
        {
            if(access(manageUsb.usbInfo.dev[i].backup_path, F_OK) == 0)
            {
                sprintf(fullPathName, "%s/%s", manageUsb.usbInfo.dev[i].backup_path, FileName);
                if(access(fullPathName, F_OK) == 0)
                {
                    UnLock(&manageUsb.usb_lock);
                    return HI_TRUE;
                }
            }
        }
    }
    UnLock(&manageUsb.usb_lock);
    return HI_FALSE;
}

void *XW_UsbDeviceHotplug_Pthread(void *args)
{
    int hotplug_sock = -1;
    int i = 0, len = 0, ret;
    char buf[UEVENT_BUFFER_SIZE*2] = {0};
    char *ptr = NULL;
    char *tmp = NULL;
    char *action = NULL;
    char *devpath = NULL;
    char *modalias = NULL;
    char *product = NULL;
    char *subsys = NULL;
    STATE_PTHREAD *p = NULL;
    fd_set fds;
    
    DVR_PTHREAD_DETACH;

    bzero(&manageUsb.usbInfo, sizeof(BACKUPDEV_T));
    
    hotplug_sock = XW_Hotplug_initSock();
    if(hotplug_sock <= 0)
    {
        _DEBUG("Can't open NetLink socket\n");
        return NULL;
    }
    
    p = (STATE_PTHREAD *)XW_ManagePthread_GetPthreadState(PTHREAD_USBHOTPLUG_ID, NOTNEEND_CH);
    p->power = PTHREAD_POWER_ON;
    p->state = ALIVE;
    
    _DEBUG("Usb DeviceHotplug pthread is running.......");
    while(p->power == PTHREAD_POWER_ON)
    {        
        FD_ZERO(&fds);
        FD_SET(hotplug_sock, &fds);  
        ret = select(hotplug_sock + 1, &fds, NULL, NULL, NULL);
        if((ret < 0) || (!(ret > 0 && FD_ISSET(hotplug_sock, &fds))))  
            continue;  

        /* receive data */ 
        memset(buf,0,UEVENT_BUFFER_SIZE*2);
        len = recv(hotplug_sock, &buf, sizeof(buf), 0);
        if(len <= 0)
            continue;
        //else
        //    _DEBUG("%s",buf);
        
        i        = 0;
        ptr      = buf;
        tmp      = NULL;
        action   = NULL;
        devpath = NULL;
        modalias = NULL;
        product  = NULL;
        subsys   = NULL;
        
        while(i < len)
        {
            tmp = strchr(ptr, '=');
            if(tmp)
            {
                *tmp = 0;
                if(strcmp(ptr, "ACTION") == 0) action = tmp + 1;
                else if(strcmp(ptr, "DEVPATH") == 0) devpath = tmp + 1;
                else if(strcmp(ptr, "MODALIAS") == 0) modalias = tmp + 1;
                else if(strcmp(ptr, "PRODUCT") == 0) product = tmp + 1;
                else if(strcmp(ptr, "SUBSYSTEM") == 0) subsys = tmp + 1;
            }
            i += strlen(buf + i) + 1;
            ptr = buf + i;
        }
        
        //_DEBUG("%s",devpath);
        //printf("\n");
        char *usbdevpath = NULL;
        usbdevpath = strstr(devpath, "/block/sd");        
        if(action && devpath && usbdevpath && strpbrk(usbdevpath,"0123456789"))
        {            
            if(strcmp(action, "remove") == 0)
            {
                char devname[8] = {'\n'};
                strncpy(devname,usbdevpath+strcspn(usbdevpath,"0123456789")-3,4);
                
                _DEBUG("umount device: %s",usbdevpath);
                XW_Hotplug_UmountUsbDisk(devname);                
            }
            else   if(strcmp(action, "add") == 0)
            {
                //char mountdev[16] = {'\n'};
                //char devname[8] = {'\n'};
                
                //strncpy(devname,usbdevpath+strcspn(usbdevpath,"0123456789")-3,4);
                //sprintf(mountdev,"mount -t vfat /dev/%s /usb/",devname);
                //system(mountdev);  
                _DEBUG("Found device: %s",usbdevpath);
#ifdef CFG_UPDATE
                if(XW_Update_GetPthreadUpdateState() == false)
                {
                    _DEBUG("Start Update syatem Thread ...");
                    XW_Update_CreateUpdatePthread();
                }
#endif
            }
        }
    }
    p->state = EXIT;

    return NULL;


}

