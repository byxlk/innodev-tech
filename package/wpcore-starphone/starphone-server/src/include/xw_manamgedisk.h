#ifndef  __DOOR_MANAGEDISK_H__
#define  __DOOR_MANAGEDISK_H__

#define MAX_DEVICE   8
#define DEVTYPE_DISK 2
#define DEVTYPE_U    3
//-----------------------------------------------------------------------------
//�ⲿ�豸���ӽṹ
typedef struct tagDevParam_T
{
    CHAR_T devname[12];
    U8_T  devtype;///2----Ӳ�̣�3----U��
    U8_T  partition_count;//��������
    U32_T dev_totalspace;//KB
    U32_T partition_size[12];/*������С����λKB*/
} tagDevParam_t, *ptagDevParam_t;


//�ⲿ�豸�� �ܽṹ
typedef struct tagDevinfo_T
{
    tagDevParam_t dev[MAX_DEVICE];
    I32_T     devcount; /*�ܹ����豸����*/
    I32_T     disk_num;
} tagDevinfo_t, *ptagDevinfo_t;

//-----------------------------------------------------------------------------
//Ӳ����Ϣ���ӽṹ
typedef struct tagHarddisk_T
{
    CHAR_T diskname[12];        //�豸��
    U8_T  disk_valid;          //0---��Ч,1---��Ч
    U8_T  is_exist;            //�Ƿ���� : 0---�����ڣ�1---����
    U8_T  is_legal;            //Ӳ�̵ĺϷ���  0---���Ϸ���1--�Ϸ�
    U8_T  partition_count;     //��������
    I8_T  partition_valid;     //������Ч  0---��Ч��1---��Ч
    I8_T  volume_lable_valid;  //��� ��Ч��0------��Ч��1------��Ч
    CHAR_T Mount_Path[4][32];  //���ص�1
    U32_T partition_size[4];/*������С����λKB*/
    U8_T  dev_num;             //Ӳ�̺�
    U32_T total_space;         //Ӳ�̵��ܿռ�  ��λKB
    U32_T free_space ;         //ʣ��ռ�
} Harddisk_t, *ptagHarddisk_t;


//Ӳ���ӽṹ
typedef struct tagDiskMangeInfo_T
{
    Harddisk_t dev_disk[SYSTEM_MAX_DISKNUM];
    U8_T   dev_disk_totalnum;
} DiskMangeInfo_t, *pDiskMangeInfo_t;
//-----------------------------------------------------------------------------
//���͸�UI��Ӳ����Ϣ
typedef struct tagHarddiskUi_T
{
    U8_T  dev_num;             //Ӳ�̺�  1  2
    U8_T  disk_valid;          //0---��Ч,1---��Ч
    U8_T  is_exist;            //�Ƿ���� : 0---�����ڣ�1---����
    U8_T  is_legal;            //Ӳ�̵ĺϷ���  0---���Ϸ���1--�Ϸ�
    U32_T total_space;         //Ӳ�̵��ܿռ�  ��λKB
    U32_T free_space;          //���ÿռ�
} HarddiskUi_t;


//����UIӲ���ӽṹ
typedef struct tagDiskMangeInfoUi_T
{
    HarddiskUi_t dev_disk[SYSTEM_MAX_DISKNUM];
    U8_T     dev_disk_totalnum;//�ܹ���Ӳ������
} DiskMangeInfoUI_t;
//-----------------------------------------------------------------------------


typedef enum
{
    ENOUGH = 1,
    FULL
} DISK_SPACE_STATE;

typedef enum
{
    STOP = 0,
    RUNING,
    NO_DISK,//û��Ӳ��
    ERROR//Ӳ�̳���
} DISK_RUN_STATE;

typedef struct tagDiskStates_T
{
    DISK_SPACE_STATE    state;
    DISK_RUN_STATE      disk_runstate;
    I8_T            current_disk_num;
} tagDiskStates_t, *ptagDiskStates_t;

typedef enum
{
    STATE_BIT = 0,
    DISK_RUNSTATE_BIT,
    CURRENT_DISK_NUM_BIT,
} SetDiskStatesBit;

#define DeleteFileLog   "deletelog.db" 
I32_T XW_Disk_CheckDev(BACKUPDEV_T  *usbInfo);
I32_T  XW_Disk_MountAllDisk(bool isALL, char *disk_name);
void *XW_Disk_pthreadDiskManage(void *args);
tagDiskStates_t XW_Disk_GetDiskStates(void);
void XW_Disk_init(void);
U8_T XW_Disk_GetCurrentWorkNum(void);
void XW_Disk_SetCurrentWorkNum(U8_T num);
void XW_Disk_MutexDestroy(void);
void XW_Disk_GetDiskInfo(DiskMangeInfo_t *disk_info);
void XW_Disk_UpdateDiskInfo(Harddisk_t disk, U8_T disk_num);
void XW_Disk_UpdateDiskInfoToUiStruct(void);
void  XW_Disk_GetUiDiskInfo(DiskMangeInfoUI_t *val);
bool  XW_Disk_Format(U8_T disk_num);
void XW_DiskTools_StopHardDiskWork(char *name);
I32_T  XW_DiskTools_GetMountSpaceInfo(char *deviceName, unsigned long *tsize, unsigned long *fsize, unsigned long *asize, U8_T  unit);
I32_T   XW_DiskTools_getDiskInfo(DiskMangeInfo_t *disk_info, U8_T disk_num, U32_T *free_size);
int  XW_DiskTools_Mke2fs(U8_T partNum, char *devname);
int  XW_DiskTools_Tune2fs(U8_T partNum, char *devname);
bool  XW_DiskTools_formatDdisk(Harddisk_t hd);
void XW_DiskTools_ChangeDiskPartition(U8_T disk_num, DiskMangeInfo_t info);
void XW_DiskTools_mountHardDiskIsReadOnly(U8_T  disk_num, Harddisk_t    *hd);
bool  XW_DiskTools_mountHardDiskIsReadWrite(U8_T  disk_num, Harddisk_t  *hd);
I32_T XW_DiskTools_CheckIsDiskOrUsbDisk(char *devname);
I32_T XW_Disk_UnMountAllDisk(bool isALL, char *disk_name);
I32_T XW_Disk_CheckDiskIsError(void);
DiskMangeInfo_t *XW_Disk_GetContext(void);
bool XW_DiskTools_checkDeviceIsRemoved(char *devPartitionsName);
bool XW_DiskTools_CheckMountedDisk(CHAR_T *pattern);
void XW_Disk_SetDiskStates(U8_T do_type , U8_T val);
I32_T XW_Disk_HaveValidDisk(void);
int XW_DiskTools_GetPartitionFilesystem(char *part_name, int *systemId, char *system_type);
I32_T XW_DiskTools_GetDiskPartitionSize(char *pDevname , U32_T *pSize);
U32_T XW_Disk_GetDiskCheckPthreadState(void);
void XW_Disk_PauseDiskCheckPthread(void);
void XW_Disk_StartDiskCheckPthread(void);
void XW_Disk_EndDiskCheckPthread(void);
bool XW_Disk_isRunningDiskCheck();
void XW_Disk_PauseDiskCheck(void);
bool  XW_MsgUI_StartFormatDisk(void);
void XW_MsgUI_EndFormatDisk(void);

#endif
