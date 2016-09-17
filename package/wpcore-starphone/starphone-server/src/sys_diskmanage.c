#include "include/xw_export.h"

#ifdef CFG_DISKMANAGE
/***********************************************************************************************************
**����:
**�������:
**����:
**����ֵ:
***********************************************************************************************************/
#define LEGAL_PARTITION_NUM 2
#define USE_SWAP
typedef enum
{
    DISKFULL_COVER = 0,
    DISKFULL_STOP
}
RECPOLICY_TYPE_T;

typedef struct __MANAGEDISK_T
{
    DiskMangeInfo_t    DiskMangeInfo;
    tagDiskStates_t    DiskStates;
    I32_T has_disk ;
    pthread_mutex_t harddisk_mutex_lock;
    DiskMangeInfoUI_t DiskMangeInfoUI;
    U8_T  work_disk_num;
    bool init_mutexLock;
    bool runnState;
    bool ctrl;
} MANAGEDISK_T;

static MANAGEDISK_T manageDisk = {.has_disk = -1,
                                  .work_disk_num = 0xff,
                                  .init_mutexLock = false,
                                  .runnState = SPS_STATE_STOP,
                                 };
/***********************************************************************************************************
**����:
**�������:
**����:
**����ֵ:XW_Disk_MutexDestroy
***********************************************************************************************************/
void XW_Disk_MutexDestroy(void)
{
    //  pthread_mutex_destroy(&harddisk_mutex_lock);
}
/***********************************************************************************************************
**����:
**�������:
**����:
**����ֵ:
***********************************************************************************************************/
DiskMangeInfo_t *XW_Disk_GetContext(void)
{
    return &manageDisk.DiskMangeInfo;
}
/***********************************************************************************************************
**����:
**�������:
**����:
**����ֵ:
***********************************************************************************************************/
VOID_T XW_Disk_ClearDiskInfo(void)
{
    bzero(&manageDisk.DiskMangeInfo, sizeof(DiskMangeInfo_t));
}
/***********************************************************************************************************
**����:
**�������:
**����:
**����ֵ:
***********************************************************************************************************/
void XW_Disk_MutexLock(void)
{
    Lock(manageDisk.init_mutexLock, &manageDisk.harddisk_mutex_lock);
}
/***********************************************************************************************************
**����:
**�������:
**����:
**����ֵ:
***********************************************************************************************************/
void XW_Disk_MutexUnLock(void)
{
    UnLock(&manageDisk.harddisk_mutex_lock);
}
/***********************************************************************************************************
**����:
**�������:
**����:
**����ֵ:
***********************************************************************************************************/
U8_T XW_Disk_GetCurrentWorkNum(void)
{
	/*����ж��Ӳ��ʱ���ж��ĸ�Ӳ����Ϊ������*/

    U8_T ret;
    XW_Disk_MutexLock();
    ret = manageDisk.work_disk_num;
    XW_Disk_MutexUnLock();
    return ret;
}
/***********************************************************************************************************
**����:
**�������:
**����:
**����ֵ:
***********************************************************************************************************/
void XW_Disk_SetCurrentWorkNum(U8_T num)
{/*���õ�ǰ������Ӳ��*/
    XW_Disk_MutexLock();
    manageDisk.work_disk_num = num;
    XW_Disk_MutexUnLock();
}
/***********************************************************************************************************
**����:
**�������:
**����:
**����ֵ:
***********************************************************************************************************/
tagDiskStates_t XW_Disk_GetDiskStates(void)
{/*���Ӳ�̵�״̬*/
    return manageDisk.DiskStates;
}
/***********************************************************************************************************
**����:
**�������:
**����:
**����ֵ:
***********************************************************************************************************/
void XW_Disk_SetDiskStates(U8_T do_type , U8_T val)
{/*����Ӳ�̵�״̬*/
    switch(do_type)
    {
    case STATE_BIT:
        manageDisk.DiskStates.state = val;
        break;
    case DISK_RUNSTATE_BIT:
        manageDisk.DiskStates.disk_runstate = val; 
        break;
    case CURRENT_DISK_NUM_BIT:
        manageDisk.DiskStates.current_disk_num = val;
        break;
    }
}
/***********************************************************************************************************
**����:
**�������:
**����:
**����ֵ:
***********************************************************************************************************/
void  XW_Disk_CheckDiskIsLegal(DiskMangeInfo_t *dev)
{
	/*����Ƿ��ǺϷ���Ӳ�̣���������Ƿ����������ļ�ϵͳ����*/

    U8_T i;
    char partName[64], systemtype[64];
    int systemId, systemId2;
    for(i = 0; i < dev->dev_disk_totalnum; i++)
    {
        if(dev->dev_disk[i].partition_count != LEGAL_PARTITION_NUM)
        {
            dev->dev_disk[i].partition_valid = 0;
            dev->dev_disk[i].is_legal = 0;
			_DEBUG("is_legal=0");
        }
        else
        {
            sprintf(partName, "%s1", dev->dev_disk[i].diskname);
            XW_DiskTools_GetPartitionFilesystem(partName, &systemId, systemtype);
            sprintf(partName, "%s2", dev->dev_disk[i].diskname);
            XW_DiskTools_GetPartitionFilesystem(partName, &systemId2, systemtype);
            if(systemId == 83 && systemId2 == 83)
            {
                dev->dev_disk[i].partition_valid = 1;
                dev->dev_disk[i].is_legal = 1;
				_DEBUG("is_legal=1");
            }
            else
            {
                dev->dev_disk[i].partition_valid = 0;
                dev->dev_disk[i].is_legal = 0;
				_DEBUG("is_legal=0");
            }
        }
    }
}
/***********************************************************************************************************
**����:
**�������:
**����:
**����ֵ:
***********************************************************************************************************/
I32_T XW_Disk_CheckDev(BACKUPDEV_T  *usbInfo)
{
	/*�������*/

    I32_T  i = 0, j , dev_tpye;
    U8_T k,DevPartitionNum[MAX_DEVICE][MAX_DEVICE] = {{0},{0}};
    FILE    *fp = NULL;
    ssize_t size;
    size_t len = 0;
    CHAR_T   *line = NULL;
    tagDevinfo_t devinfo;
    CHAR_T *pname;
    CHAR_T  tmp_devname[12];
    CHAR_T  tmpfilename[64];
    CHAR_T *token;
    CHAR_T seps[] = " ";
    U8_T    partitionNum = 0;
    DiskMangeInfo_t *DiskMangeInfo;
    DiskMangeInfo = XW_Disk_GetContext();

    bzero(tmp_devname, 12);
    bzero(&devinfo, sizeof(devinfo));
    if(tmpnam(tmpfilename) == NULL)
    {
        sprintf(tmpfilename, "%s", "/tmp/partitions.txt");
    }
    XW_Tools_DvrCpFile("/proc/partitions", tmpfilename, true, "sd");
    fp = fopen(tmpfilename, "r");

    while((size = getline(&line, &len, fp)) != -1)
    {
        pname = strrchr(line, ' ');
        if(pname == NULL)
            continue;
        //else
        //    _DEBUG("pname:%s",pname);
        pname++;
        if(memcmp(pname, tmp_devname, 3) == 0)
        {
        	/*�·���*/
            i = 0;
            token = strtok(line, seps);
            while(token != NULL)
            {
                //_DEBUG("token: %s",token);
                if(i == 2)
                {
                    k = devinfo.dev[devinfo.devcount-1].partition_count;
                    devinfo.dev[devinfo.devcount-1].partition_size[k] = atoi(token);
                }
                else if(i == 3)
                {
                    DevPartitionNum[devinfo.devcount-1][partitionNum] = atoi(strpbrk(token,"0123456789"));
                    //_DEBUG("DevPartitionNum[%d][%d] = %d",devinfo.devcount-1,partitionNum,DevPartitionNum[devinfo.devcount-1][partitionNum]);
                }
                i++;
                token = strtok(NULL, seps);
            }
            partitionNum++;
            devinfo.dev[devinfo.devcount-1].partition_count++;
			_DEBUG("Dev %s current partions: %d, memery: %d Kb", devinfo.dev[devinfo.devcount-1].devname,
                    devinfo.dev[devinfo.devcount-1].partition_count,
                    devinfo.dev[devinfo.devcount-1].dev_totalspace);
        }
        else
        {
        	/*���豸*/
            partitionNum = 0;
            memcpy(devinfo.dev[devinfo.devcount].devname, pname, 3);//sda
            i = 0;
            token = strtok(line, seps);
            while(token != NULL)
            {
                //_DEBUG("token: %s",token);
                if(i == 2)
                {
                    devinfo.dev[devinfo.devcount].dev_totalspace = atoi(token);
                }
                i++;
                token = strtok(NULL, seps);
            }
			_DEBUG("new dev %s",devinfo.dev[devinfo.devcount].devname);
            devinfo.devcount++;
        }
        memcpy(tmp_devname, pname, 3);
    }


    if(usbInfo == NULL)
    {
        for(i = 0; i < devinfo.devcount; i++)
        {
        	_DEBUG(" ");
            dev_tpye = XW_DiskTools_CheckIsDiskOrUsbDisk(devinfo.dev[i].devname);
            devinfo.dev[i].devtype = dev_tpye;
            if(devinfo.dev[i].devtype == DEVTYPE_DISK)/*Ӳ��*/
            {
                DiskMangeInfo->dev_disk[DiskMangeInfo->dev_disk_totalnum].is_exist = 1;/*��ʾӲ�̴���*/
                DiskMangeInfo->dev_disk[DiskMangeInfo->dev_disk_totalnum].total_space = devinfo.dev[i].dev_totalspace;/*�ܿռ�*/
                DiskMangeInfo->dev_disk[DiskMangeInfo->dev_disk_totalnum].dev_num = DiskMangeInfo->dev_disk_totalnum + 1;/*�豸���*/
                sprintf(DiskMangeInfo->dev_disk[DiskMangeInfo->dev_disk_totalnum].diskname, "%s", devinfo.dev[i].devname);/*�豸��*/
                DiskMangeInfo->dev_disk[DiskMangeInfo->dev_disk_totalnum].partition_count = devinfo.dev[i].partition_count;/*��������*/
                for(j = 0; j < devinfo.dev[i].partition_count; j++)
                {
                    if(j < 4)
                        DiskMangeInfo->dev_disk[DiskMangeInfo->dev_disk_totalnum].partition_size[j] = devinfo.dev[i].partition_size[j];
                }

				/*���÷����Ĺ���·��*/
				for(j = 0; j < 4; j++)
                {
                    if(j == 0)
                    {
                        sprintf(DiskMangeInfo->dev_disk[DiskMangeInfo->dev_disk_totalnum].Mount_Path[j], "/mnt/hddisk%d", DiskMangeInfo->dev_disk_totalnum + 1); //�趨�̶��Ĺ��ص�
                    }
                    else
                    {
                        sprintf(DiskMangeInfo->dev_disk[DiskMangeInfo->dev_disk_totalnum].Mount_Path[j], "/mnt/hddisk%d%d", DiskMangeInfo->dev_disk_totalnum + 1, j + 1); //�趨�̶��Ĺ��ص�
                    }
                }
                DiskMangeInfo->dev_disk_totalnum++;/*����Ӳ������*/
            }
        }
		_DEBUG(" ");

        if(DiskMangeInfo->dev_disk_totalnum > 0)
            XW_Disk_CheckDiskIsLegal(DiskMangeInfo);/*����Ƿ��ǺϷ���Ӳ��*/
		_DEBUG(" ");
    }
    else
    {
        bzero(usbInfo, sizeof(BACKUPDEV_T));
        partitionNum = 0;
        for(i = 0; i < devinfo.devcount; i++)
        {
            dev_tpye = XW_DiskTools_CheckIsDiskOrUsbDisk(devinfo.dev[i].devname);
            devinfo.dev[i].devtype = dev_tpye;
            if(devinfo.dev[i].devtype == DEVTYPE_U)/*USB�豸*/
            {
                for(j = 0; j < devinfo.dev[i].partition_count; j++)/*��������*/
                {
                    usbInfo->dev[partitionNum].exist = 1;/*�豸���ڱ�ʾ*/
                    sprintf(usbInfo->dev[partitionNum].dev_typename, "%s", devinfo.dev[i].devname);/*�豸��*/
                    usbInfo->dev[partitionNum].partition_num = DevPartitionNum[i][j];/**/
                    _DEBUG("Usb dev %s  partition =%d", usbInfo->dev[partitionNum].dev_typename, usbInfo->dev[partitionNum].partition_num);
                    partitionNum++;
                    usbInfo->total_dev_number = partitionNum;     
                }
            }
        }
    }
    if(line)
    {
        free(line);
        line = NULL;
    }
    fclose(fp);
    remove(tmpfilename);
    return DiskMangeInfo->dev_disk_totalnum;
}

/***********************************************************************************************************
**����:
**�������:
**����:
**����ֵ:
***********************************************************************************************************/
I32_T XW_Disk_CheckAllMountedDisk(void)
{
	/*�����ص�Ӳ��*/

    I8_T i, j;
    CHAR_T target_path[32];
    I32_T ret = 0;
    bool val = false;
    DiskMangeInfo_t *DiskMangeInfo;
    DiskMangeInfo = XW_Disk_GetContext();

    for(i = 0; i < DiskMangeInfo->dev_disk_totalnum; i++)
    {
        val = false;
        for(j = 0; j < DiskMangeInfo->dev_disk[i].partition_count; j++)
        {
            sprintf(target_path, "%s", DiskMangeInfo->dev_disk[i].Mount_Path[j]);
			_DEBUG("%s",target_path);
			if(access(target_path,F_OK)==0)
			{
				ret = XW_DiskTools_CheckMountedDisk(target_path);/*������·���Ƿ��ڱ�����*/
				if(ret == true)
				{
					val = true;
					_DEBUG("find  /dev/%s  mounted %s", DiskMangeInfo->dev_disk[i].diskname, target_path);
				}
				else
				{
					val = false;
				}
			}
			else
			{
				val = false;
			}
        }

        if(val == false)
        {
            DiskMangeInfo->dev_disk[i].disk_valid = DISK_ISNOT_VALID;/*Ӳ����Ч*/
        }
        else
        {
            DiskMangeInfo->dev_disk[i].disk_valid = DISK_IS_VALID;/*Ӳ����Ч*/
        }
    }
    return val;
}
/***********************************************************************************************************
**����:
**�������:
**����:
**����ֵ:
***********************************************************************************************************/
I32_T  XW_Disk_MountAllDisk(bool isALL, char *disk_name)
{
	/*����Ӳ��*/

    I8_T i;
    bool res, enableCheck;
    DiskMangeInfo_t *DiskMangeInfo;

    DiskMangeInfo = XW_Disk_GetContext();
    for(i = 0; i < DiskMangeInfo->dev_disk_totalnum; i++)
    {
        if(isALL == true)
        {
            enableCheck = 0;
        }
        else
        {
            enableCheck = memcmp(disk_name,  DiskMangeInfo->dev_disk[i].diskname, 3);
        }

        if(enableCheck == 0)
        {
            if(DiskMangeInfo->dev_disk[i].is_legal == DISK_IS_VALID)
            {
                XW_DiskTools_mountHardDiskIsReadOnly(i, &DiskMangeInfo->dev_disk[i]);
            }

            if(DiskMangeInfo->dev_disk[i].is_legal == DISK_IS_VALID)
            {
                res = XW_DiskTools_mountHardDiskIsReadWrite(i, &DiskMangeInfo->dev_disk[i]);
            }
        }
    }
    XW_Disk_CheckAllMountedDisk();
    return 0;
}

/***********************************************************************************************************
**����:
**�������:
**����:
**����ֵ:
***********************************************************************************************************/
I32_T  XW_Disk_MountDisk(char *disk_name)
{
    XW_Disk_MountAllDisk(HI_FALSE, disk_name);
    return 0;
}
/***********************************************************************************************************
**����:
**�������:
**����:
**����ֵ:
***********************************************************************************************************/
I32_T XW_Disk_UnMountAllDisk(bool isALL, char *disk_name)
{
	/*ж�ع��ص�Ӳ��*/

    I32_T ret = -1, i, j;
    char buffer[32], target_path[32];
    bool enable_umont;
    DiskMangeInfo_t *DiskMangeInfo;
    DiskMangeInfo = XW_Disk_GetContext();
    for(i = 0; i < DiskMangeInfo->dev_disk_totalnum; i++)
    {
        if(isALL == true)
        {
            enable_umont = 0;
        }
        else
        {
            enable_umont = memcmp(disk_name, DiskMangeInfo->dev_disk[i].diskname, 3);
        }
        if(enable_umont != 0)
        {
            continue;
        }

        for(j = 0; j < DiskMangeInfo->dev_disk[i].partition_count; j++)
        {
            sprintf(target_path, "%s",  DiskMangeInfo->dev_disk[i].Mount_Path[j]);
            if(access(target_path, F_OK) == 0)
            {
                ret = umount(target_path);
                if(ret != 0)
                {
                    _ERROR("umount %s is error !", target_path);
                }
                else
                {
                    _DEBUG("umount %s is success !", target_path);
                    DiskMangeInfo->dev_disk[i].disk_valid = DISK_ISNOT_VALID;
                }
                sprintf(buffer, "/dev/%s",  DiskMangeInfo->dev_disk[i].diskname);
                XW_DiskTools_StopHardDiskWork(buffer);
            }
        }
    }
    return 0;
}
/***********************************************************************************************************
**����:
**�������:
**����:
**����ֵ:
***********************************************************************************************************/
I32_T  XW_Disk_UnMountDisk(char *disk_name)
{
    XW_Disk_UnMountAllDisk(false, disk_name);
    return 0;
}
/***********************************************************************************************************
**����:
**�������:
**����:
**����ֵ:
***********************************************************************************************************/
I32_T XW_Disk_CheckDiskIsError(void)
{
	/*�����ص�Ӳ���Ƿ����*/

    I32_T ret;
    ret = XW_Disk_CheckAllMountedDisk();
    if(ret != true)
    {
        XW_Disk_SetDiskStates(DISK_RUNSTATE_BIT, ERROR);
        XW_Disk_SetDiskStates(STATE_BIT, FULL);
        _ERROR("Disk is error");
        return -1;
    }
    return 0;
}
/***********************************************************************************************************
**����:
**�������:
**����:
**����ֵ:
***********************************************************************************************************/
void XW_Disk_DynamicDetectionDisk(void)
{
	/*��̬�ļ��Ӳ�̣��Ƿ�ĳ��ԭ���Ƴ�*/

    char devName[24];
    U8_T i;
    I32_T ret;
    DiskMangeInfo_t *DiskMangeInfo;
    DiskMangeInfo = XW_Disk_GetContext();
    for(i = 0; i < DiskMangeInfo->dev_disk_totalnum; i++)
    {
        if(DiskMangeInfo->dev_disk[i].disk_valid == DISK_IS_VALID)
        {
            sprintf(devName, "/dev/%s", DiskMangeInfo->dev_disk[i].diskname);
            ret = XW_DiskTools_checkDeviceIsRemoved(devName);
            if(ret == true)
            {
                DiskMangeInfo->dev_disk[i].disk_valid = DISK_ISNOT_VALID;
            }
        }
    }
}
/***********************************************************************************************************
**����:
**�������:
**����:
**����ֵ:
***********************************************************************************************************/
I32_T XW_Disk_HaveValidDisk(void)
{
	/*�Ƿ�����Ч��Ӳ��*/

    I32_T ret = 0;
    U8_T i;
    DiskMangeInfo_t *DiskMangeInfo;
    DiskMangeInfo = XW_Disk_GetContext();
    for(i = 0; i < DiskMangeInfo->dev_disk_totalnum; i++)
    {
        if(DiskMangeInfo->dev_disk[i].disk_valid == DISK_ISNOT_VALID)
        {
            ret = -1;
            _DEBUG("XW_Disk_HaveValidDisk ");
        }
    }

    if(DiskMangeInfo->dev_disk_totalnum == 0)
    {
        return -1;
    }
    return ret;
}
/***********************************************************************************************************
**����:
**�������:
**����:
**����ֵ:
***********************************************************************************************************/
void XW_Disk_GetDiskInfo(DiskMangeInfo_t *disk_info)
{
	/*���Ӳ�̵���Ϣ*/

    XW_Disk_MutexLock();
    memcpy(disk_info, &manageDisk.DiskMangeInfo, sizeof(DiskMangeInfo_t));
    XW_Disk_MutexUnLock();
}
/***********************************************************************************************************
**����:
**�������:
**����:
**����ֵ:
***********************************************************************************************************/
void XW_Disk_UpdateDiskInfo(Harddisk_t disk, U8_T disk_num)
{
	/*����Ӳ�̵���Ϣ*/

    XW_Disk_MutexLock();
    memcpy(&manageDisk.DiskMangeInfo.dev_disk[disk_num], &disk, sizeof(Harddisk_t));
    XW_Disk_MutexUnLock();
}
/***********************************************************************************************************
**����:
**�������:
**����:
**����ֵ:
***********************************************************************************************************/
U32_T  XW_Disk_MinDiskFreeSpace(U8_T  diskNum)
{
	/*���ĳ��Ӳ�̵�ʣ��ռ�*/

    U32_T min_space;
    min_space = (manageDisk.DiskMangeInfo.dev_disk[diskNum].partition_size[1] >> 10) * 0.05 + 200;
    return min_space;
}
/***********************************************************************************************************
**����:
**�������:
**����:
**����ֵ:
***********************************************************************************************************/
I32_T  XW_Disk_GetWorkdiskNum(void)
{
	/*��ù�����Ӳ�̺�*/

    I32_T  i;
    I32_T  min_space = 0;
    unsigned long fsize = 0, tsize = 0, asize = 0;
    for(i = 0; i < SYSTEM_MAX_DISKNUM; i++)
    {
        if(manageDisk.DiskMangeInfo.dev_disk[i].disk_valid == DISK_IS_VALID)
        {
            XW_DiskTools_GetMountSpaceInfo(manageDisk.DiskMangeInfo.dev_disk[i].Mount_Path[1], &tsize, &fsize, &asize, MB_UNIT);
            min_space = XW_Disk_MinDiskFreeSpace(i);
            if(fsize > min_space)
            {
                return i;
            }
        }
    }
    return CURRENT_WORK_DISK_FULL;
}
/***********************************************************************************************************
**����:
**�������:
**����:
**����ֵ:
***********************************************************************************************************/
I32_T  XW_Disk_UpdateDiskSpaceInfo(void)
{
    I32_T  i;
    unsigned long fsize = 0, tsize = 0, asize = 0;
    for(i = 0; i < SYSTEM_MAX_DISKNUM; i++)
    {
        if(manageDisk.DiskMangeInfo.dev_disk[i].disk_valid == DISK_IS_VALID)
        {
            XW_DiskTools_GetMountSpaceInfo(manageDisk.DiskMangeInfo.dev_disk[i].Mount_Path[1], &tsize, &fsize, &asize, KB_UNIT);
            manageDisk.DiskMangeInfo.dev_disk[i].free_space = fsize;
        }
    }
    return 0;
}
/***********************************************************************************************************
**����:
**�������:
**����:
**����ֵ:
***********************************************************************************************************/
void  XW_Disk_GetUiDiskInfo(DiskMangeInfoUI_t *val)
{/*��UIʹ�õ�Ӳ����Ϣ*/
    memcpy(val, &manageDisk.DiskMangeInfoUI, sizeof(DiskMangeInfoUI_t));
}
/***********************************************************************************************************
**��    ��:
**�������:
**��    ��:
**����  ֵ:
***********************************************************************************************************/
void XW_Disk_SetUiDiskInfo(DiskMangeInfoUI_t val)
{
    memcpy(&manageDisk.DiskMangeInfoUI, &val, sizeof(DiskMangeInfoUI_t));
}
/***********************************************************************************************************
**��    ��:
**�������:
**��    ��:
**����  ֵ:
***********************************************************************************************************/
void XW_Disk_UpdateDiskInfoToUiStruct(void)
{
	/*����Ӳ����Ϣ��UI��ʹ��*/

    U8_T i;
    DiskMangeInfoUI_t info;
    DiskMangeInfo_t *DiskMangeInfo;
    DiskMangeInfo = XW_Disk_GetContext();

    bzero(&info, sizeof(DiskMangeInfoUI_t));
    XW_Disk_UpdateDiskSpaceInfo();
    info.dev_disk_totalnum = DiskMangeInfo->dev_disk_totalnum;
    for(i = 0; i < SYSTEM_MAX_DISKNUM; i++)
    {
        if(DiskMangeInfo->dev_disk[i].is_exist == 1)
        {
            info.dev_disk[i].dev_num = DiskMangeInfo->dev_disk[i].dev_num;
            info.dev_disk[i].disk_valid = DiskMangeInfo->dev_disk[i].disk_valid;
            info.dev_disk[i].is_exist = DiskMangeInfo->dev_disk[i].is_exist;
            info.dev_disk[i].is_legal = DiskMangeInfo->dev_disk[i].is_legal;
            info.dev_disk[i].total_space = DiskMangeInfo->dev_disk[i].total_space;
            info.dev_disk[i].free_space = DiskMangeInfo->dev_disk[i].free_space;
        }
    }
    XW_Disk_SetUiDiskInfo(info);
}
/***********************************************************************************************************
**����:
**�������:
**����:
**����ֵ:
***********************************************************************************************************/
void XW_Disk_CreateDeleteLogFile(U8_T work_num, bool need_sw, char *TargetFileName)
{
    char buffer[64];
    char SrcFileName[32];
    if(need_sw == true)
    {
        if(work_num == 0)
        {
            work_num = 1;
        }
        else
        {
            work_num = 0;
        }
    }
    sprintf(TargetFileName, "%s/%s", manageDisk.DiskMangeInfo.dev_disk[work_num].Mount_Path[1], DeleteFileLog);
    if(access(TargetFileName, F_OK) != 0)
    {
        sprintf(buffer, "cp -arf %s %s", SrcFileName, TargetFileName);
        if(system(buffer) < 0)
        {
            _ERROR("create delete log is error");
        }
    }
}
/***********************************************************************************************************
**����:
**�������:
**����:
**����ֵ:
***********************************************************************************************************/
I32_T  XW_Disk_DeleteFile(I32_T work_num, bool need_sw)
{
    char TargetFileName[32];
    U32_T recNumber;
    sprintf(TargetFileName, "%s/%s", manageDisk.DiskMangeInfo.dev_disk[work_num].Mount_Path[1], DeleteFileLog);
    if(access(TargetFileName, F_OK) != 0)
    {
        XW_Disk_CreateDeleteLogFile(work_num, need_sw, TargetFileName);
    }
    else
    {
        //recNumber = XW_DataBase_RecordFileNum(TargetFileName);
        if(recNumber == 0)
        {
            if(access(TargetFileName, F_OK) == 0)
            {
                unlink(TargetFileName);
            }
            XW_Disk_CreateDeleteLogFile(work_num, need_sw, TargetFileName);
        }
    }
    //XW_DataBase_DeleteIdx(work_num, manageDisk.DiskMangeInfo, TargetFileName);
    //XW_ReBuildRecIdx(work_num, manageDisk.DiskMangeInfo);
    return work_num;
}

/***********************************************************************************************************
**����:
**�������:
**����:
**����ֵ:
***********************************************************************************************************/
void XW_Disk_init(void)
{
	/*Ӳ�̳�ʼ��*/

    XW_Disk_SetDiskStates(DISK_RUNSTATE_BIT, STOP);/*Ӳ�̵�ֹͣ״̬*/
    XW_Disk_SetDiskStates(CURRENT_DISK_NUM_BIT, -1);/*�޹�����Ӳ�̺�*/
    XW_Disk_ClearDiskInfo();/*��Ӳ����Ϣ*/
    manageDisk.has_disk = XW_Disk_CheckDev(NULL);/*����豸*/
    if(manageDisk.has_disk > 0)/*��Ӳ��*/
    {/*����Ӳ��*/
        XW_Disk_MountAllDisk(true, NULL);
    }
    else
    {/*����û��Ӳ��*/
        XW_Disk_SetDiskStates(DISK_RUNSTATE_BIT, NO_DISK);
        _DEBUG("doordvr	 no find hard disk !");
    }
}
/***********************************************************************************************************
**��    ��:
**�������:
**��    ��:
**����  ֵ:
***********************************************************************************************************/
bool XW_Disk_Format(U8_T disk_num)
{
	/*��ʽ��ĳ��Ӳ��*/

    U8_T i, j;
    bool find = false, IsFormatSuccess = false;
    I32_T ret, res;
    char partName[32];
    DiskMangeInfo_t *DiskMangeInfo;
    DiskMangeInfo = XW_Disk_GetContext();

    for(i = 0; i < SYSTEM_MAX_DISKNUM; i++)
    {
        _DEBUG("dev_num %d", DiskMangeInfo->dev_disk[i].dev_num);
        if(DiskMangeInfo->dev_disk[i].dev_num == disk_num)
        {
            find = true;
            break;
        }
    }
    if((find == false) || (DiskMangeInfo->dev_disk[i].is_exist != 1))
    {
        _ERROR("To format disk  does not exist. ");
        return false ;
    }

	_DEBUG(" ");
    //XW_MsgUI_StartFormatDisk();/*��ʽ��Ӳ��ʱ������ͣһЩ�߳�*/
	_DEBUG(" ");

    XW_Disk_SetDiskStates(DISK_RUNSTATE_BIT, STOP);
    if(DiskMangeInfo->dev_disk[i].disk_valid == DISK_IS_VALID)
    {/*�������Ч��Ӳ�̣�����ж�ع��صķ���*/
        _DEBUG("This is a legitimate hard disk  that System will be the first to uninstall it.");

        for(j = 0; j < DiskMangeInfo->dev_disk[i].partition_count; j++)
        {
            _DEBUG("umount %s", DiskMangeInfo->dev_disk[i].Mount_Path[j]);
            ret = umount(DiskMangeInfo->dev_disk[i].Mount_Path[j]);
            if(ret != 0)
            {
                _DEBUG("umount  %s error ", DiskMangeInfo->dev_disk[i].Mount_Path[j]);
                return false;
            }
        }
        system("/sbin/mdev -s");
    }
    else
    {
        _DEBUG("it is not valid disk");
		 DiskMangeInfo->dev_disk[i].is_legal=0;
        for(j = 0; j < DiskMangeInfo->dev_disk[i].partition_count; j++)
        {
            _DEBUG("umount %s", DiskMangeInfo->dev_disk[i].Mount_Path[j]);
            ret = umount(DiskMangeInfo->dev_disk[i].Mount_Path[j]);
            if(ret != 0)
            {
                _DEBUG("umount  %s error ", DiskMangeInfo->dev_disk[i].Mount_Path[j]);
            }
        }
    }

    if(DiskMangeInfo->dev_disk[i].is_legal == 1)
    {
    /*�Ϸ���Ӳ�̣�ֱ�ӽ��и�ʽ��*/
        _DEBUG("start   format %s  ...", DiskMangeInfo->dev_disk[i].diskname);
        IsFormatSuccess = XW_DiskTools_formatDdisk(DiskMangeInfo->dev_disk[i]);
    }
    else
    {
    /*�Ƿ���Ӳ�̣���Ҫ�ı������Ȼ���ʽ��*/
        _DEBUG("start   XW_P2pNat_check  partition   ...");
        XW_DiskTools_ChangeDiskPartition(i, *DiskMangeInfo);
        system("/sbin/mdev -s");
		DiskMangeInfo->dev_disk[i].partition_count = 2;
        DiskMangeInfo->dev_disk[i].is_legal = 1;
        _DEBUG("start   XW_DiskTools_formatDdisk %s   ...", DiskMangeInfo->dev_disk[i].diskname);
        IsFormatSuccess = XW_DiskTools_formatDdisk(DiskMangeInfo->dev_disk[i]);
    }

    system("/sbin/mdev -s");
    XW_Disk_MountDisk(DiskMangeInfo->dev_disk[i].diskname);
    if(IsFormatSuccess == true)
    {/*��ʽ���ɹ���Ȼ������Ӳ�̵���Ϣ*/
        DiskMangeInfo->dev_disk[i].is_legal = 1;
        DiskMangeInfo->dev_disk[i].partition_valid = 1;
        DiskMangeInfo->dev_disk[i].dev_num = i + 1;
        DiskMangeInfo->dev_disk[i].partition_count = 2;
        res = true;
        for(j = 0; j < 2; j++)
        {
            sprintf(partName, "%s%d", DiskMangeInfo->dev_disk[i].diskname, j + 1);
            XW_DiskTools_GetDiskPartitionSize(partName, &DiskMangeInfo->dev_disk[i].partition_size[j]);
            _DEBUG("partName size =%d KB", DiskMangeInfo->dev_disk[i].partition_size[j]);
            ret = XW_DiskTools_CheckMountedDisk(DiskMangeInfo->dev_disk[i].Mount_Path[j]);
            if(ret == false)
            {
                res = false;
            }
        }

        if(res == true)
        {
            DiskMangeInfo->dev_disk[i].disk_valid = DISK_IS_VALID;/*Ӳ����Ч*/
            XW_Disk_SetDiskStates(DISK_RUNSTATE_BIT, RUNING);/*����״̬*/
            _DEBUG("FORMAN DISK OK  ");
        }
        else
        {
            DiskMangeInfo->dev_disk[i].disk_valid = DISK_ISNOT_VALID;
            _DEBUG(" FORMAN DISK ERROR");
        }
    }
    _DEBUG("dev_disk[%d] disk_valid=%d", i, DiskMangeInfo->dev_disk[i].disk_valid);
    XW_Disk_UpdateDiskInfoToUiStruct();
    //XW_MsgUI_EndFormatDisk();/*��ʽ����ϣ��ָ�����*/
    return IsFormatSuccess;
}
/***********************************************************************************************************
**����:
**�������:
**����:
**����ֵ:
***********************************************************************************************************/
static void XW_Disk_SetDiskCheckPthreadState(U32_T runState)
{
	/*����Ӳ�̵Ĺ���״̬*/

    manageDisk.runnState = runState;
}
/***********************************************************************************************************
**����:
**�������:
**����:
**����ֵ:
***********************************************************************************************************/
U32_T XW_Disk_GetDiskCheckPthreadState(void)
{/*���Ӳ�̵Ĺ���״̬*/
    return manageDisk.runnState;
}
/***********************************************************************************************************
**����:
**�������:
**����:
**����ֵ:
***********************************************************************************************************/
void XW_Disk_PauseDiskCheckPthread(void)
{
	/*��ͣӲ�̼���߳�*/

    XW_Disk_SetDiskCheckPthreadState(SPS_STATE_RUNNING);
    manageDisk.ctrl = SPS_STATE_PAUSE;
}
/***********************************************************************************************************
**����:
**�������:
**����:
**����ֵ:
***********************************************************************************************************/
void XW_Disk_StartDiskCheckPthread(void)
{
	/*��ʼӲ�̼���߳�*/

    manageDisk.ctrl = SPS_STATE_RUNNING;
}
/***********************************************************************************************************
**����:
**�������:
**����:
**����ֵ:
***********************************************************************************************************/
void XW_Disk_EndDiskCheckPthread(void)
{
	/*ֹͣӲ�̼���߳�*/

    manageDisk.ctrl = SPS_STATE_STOP;
}
/***********************************************************************************************************
**����:
**�������:
**����:
**����ֵ:
***********************************************************************************************************/
bool XW_Disk_isRunningDiskCheck()
{
	/*�ж�Ӳ�̼���߳��Ƿ���������*/

    if(manageDisk.ctrl == SPS_STATE_RUNNING)
    {
        return true;
    }
    return false;
}

/***********************************************************************************************************
**����:
**�������:
**����:
**����ֵ:
***********************************************************************************************************/
void XW_Disk_PauseDiskCheck(void)
{
	/*��ͣӲ�̼���߳�*/

    XW_Disk_PauseDiskCheckPthread();
    while(XW_Disk_GetDiskCheckPthreadState() == SPS_STATE_RUNNING)
    {
        usleep(1000);
    }
}
/***********************************************************************************************************
**����:
**�������:
**����:
**����ֵ:
***********************************************************************************************************/
void *XW_Disk_pthreadDiskManage(void *args)
{
	/*Ӳ�̼���߳�*/

    U32_T record_count;
    I32_T work_num;
    bool mount_valid_disk;
    U8_T i = 0;
    DiskMangeInfo_t info;
    DiskMangeInfoUI_t ui_info;
    U8_T  diskNum;
    unsigned long tsize, fsize, asize;
    U32_T  minSpace;
    bool hasgooddisk = false;
    STATE_DISKM *p;
    char filename[32];
    DiskMangeInfo_t *DiskMangeInfo;

    DVR_PTHREAD_DETACH;
    
    DiskMangeInfo = XW_Disk_GetContext();
    p = (STATE_DISKM *)XW_ManagePthread_GetPthreadState(PTHREAD_DISKM_ID, NOTNEEND_CH);
    if(manageDisk.has_disk > 0)
    {
        mount_valid_disk = XW_Disk_HaveValidDisk();/*�Ƿ�����Ч��Ӳ��*/
        if(mount_valid_disk < 0)
        {
            _DEBUG(" doordvr have not valid disk please format disk ! ");
            XW_Disk_SetDiskStates(DISK_RUNSTATE_BIT, ERROR);
        }
        XW_Disk_UpdateDiskSpaceInfo();
        XW_Disk_UpdateDiskInfoToUiStruct();
        XW_Disk_GetUiDiskInfo(&ui_info);
        XW_Disk_GetDiskInfo(&info);
        //XW_DataBase_CheckDbFile(info);
        if(mount_valid_disk == 0)
        {
            //record_count = XW_DataBase_RecordCount(0, info);
            _DEBUG("disk have %d rec file ", record_count);
            XW_Disk_SetDiskStates(STATE_BIT, FULL);
            XW_Disk_SetCurrentWorkNum(0);
            XW_Disk_SetDiskStates(DISK_RUNSTATE_BIT, RUNING);
        }
    }
    if(manageDisk.has_disk <= 0)
    {
        goto end;
    }
    
    _DEBUG("Disk manage thread running....");
    
    p->power = PTHREAD_POWER_ON;
    p->state = ALIVE;
    XW_Disk_StartDiskCheckPthread();
    while((p->power == PTHREAD_POWER_ON))
    {
        hasgooddisk = false;
        XW_Disk_DynamicDetectionDisk();/*��̬���Ӳ��*/
        for(i = 0; i < SYSTEM_MAX_DISKNUM; i++)
        {/*����Ƿ��кõĹ�����Ӳ��*/
            if(DiskMangeInfo->dev_disk[i].disk_valid == DISK_IS_VALID)
            {
                hasgooddisk = true;
            }
        }
        if(hasgooddisk&&XW_Disk_isRunningDiskCheck())
        {
            work_num = XW_Disk_GetWorkdiskNum();/*���Ӳ�̹�����*/
            if(work_num < CURRENT_WORK_DISK_FULL)/*Ӳ��û����*/
            {
                XW_Disk_SetCurrentWorkNum(work_num);/*���ù�����Ӳ�̺�*/
                XW_Disk_SetDiskStates(STATE_BIT, ENOUGH);/*Ӳ�̿ռ��㹻*/
            }
            if(work_num == CURRENT_WORK_DISK_FULL)/*Ӳ�̿ռ���*/
            {
                _DEBUG("disk is full");
                //XW_DVR_GetConfig(oper_recordset_RecPolicy_type, &i, 0);
                i = DISKFULL_COVER;
                if(i == DISKFULL_COVER)/*����¼��*/
                {
#ifdef CFG_RECORDER
                    DVR_StopAllChannelRecorderInstance();/*ֹͣ����ͨ����¼��*/
#endif
                    XW_Disk_SetDiskStates(STATE_BIT, FULL);/*����Ӳ���Ѿ���*/
                    work_num = XW_Disk_GetCurrentWorkNum();
                    diskNum = DiskMangeInfo->dev_disk_totalnum;/*Ӳ������*/
                    if(diskNum == 1)/*ֻ��һ��Ӳ��*/
                    {
                        work_num = 0;
                        sprintf(filename, "%s/%s", DiskMangeInfo->dev_disk[work_num].Mount_Path[1], DeleteFileLog);
                        if(access(filename, F_OK) == 0)
                        {
                            unlink(filename);
                        }
						/*ɾ��¼���ļ�*/
                        //XW_DataBase_DeleteIdx(work_num, *DiskMangeInfo, NULL);
                    }
                    else
                    {
                        if((DiskMangeInfo->dev_disk[0].disk_valid == DISK_IS_VALID) && (DiskMangeInfo->dev_disk[1].disk_valid == DISK_IS_VALID))
                        {
                            work_num = XW_Disk_DeleteFile(work_num, true);
                        }
                        else if(DiskMangeInfo->dev_disk[0].disk_valid == DISK_IS_VALID)
                        {
                            work_num = 0;
                            XW_Disk_DeleteFile(work_num, false);
                        }
                        else
                        {
                            work_num = 1;
                            XW_Disk_DeleteFile(work_num, false);
                        }
                    }
                    XW_DiskTools_GetMountSpaceInfo(DiskMangeInfo->dev_disk[work_num].Mount_Path[1], &tsize, &fsize, &asize, MB_UNIT);
                    minSpace = XW_Disk_MinDiskFreeSpace(work_num);
                    //record_count = XW_DataBase_RecordCount(0, info);
                    _DEBUG("disk have %d rec file ", record_count);
                    if(asize <= minSpace)/*Ӳ�̿ռ仹�ǲ����������ɾ��¼���ļ�*/
                    {
                        sleep(5);
                        continue;
                    }
                    XW_Disk_SetCurrentWorkNum(work_num);
                    XW_Disk_SetDiskStates(STATE_BIT, ENOUGH);
                }
                else
                {
                    XW_Disk_SetDiskStates(STATE_BIT, FULL);
                }
            }
        }

        for(i = 0; i < 30; i++)
        {/*30����һ��Ӳ�̿ռ�*/
            if(p->power == PTHREAD_POWER_OFF)
            {
                _DEBUG("disk power off!");
                goto end;
            }
            while(!XW_Disk_isRunningDiskCheck())
            {/*ĳ���߳�ֹͣ��Ӳ�̼��*/
                XW_Disk_SetDiskCheckPthreadState(SPS_STATE_PAUSE);
                if(p->power != PTHREAD_POWER_ON)
                {/*ϵͳ�Ƴ������Զ��˳�Ӳ�̼���߳�*/
                    break;
                }
                usleep(1000 * 100);
            }
            sleep(1);
        }
    }
end:
    p->state = EXIT;
    XW_Disk_EndDiskCheckPthread();
    _DEBUG("XW_P2pNat_check disk thread exit....!");
    return NULL;
}

#endif


