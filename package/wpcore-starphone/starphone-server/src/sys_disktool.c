
#include "include/xw_export.h"

/***********************************************************************************************************
**函数:
**输入参数:
**功能:
**返回值:
***********************************************************************************************************/
bool XW_DiskTools_CheckMountedDisk(CHAR_T *pattern)
{
	/*某个路径是否真正挂载了硬盘的某个分区*/

    FILE    *fd;
    ssize_t size;
    size_t len = 0;
    CHAR_T   *line = NULL;
    char *pstr = NULL;

    if((fd = fopen("/proc/mounts", "rb")) < 0)
    {
        _ERROR("pthread_diskm.c:error In function 'fopen'\n");
        return false;
    }

    while((size = getline(&line, &len, fd)) != -1)
    {
        pstr = strstr(line, pattern);
        if(pstr != NULL)
        {
            if(line)
            {
                free(line);
            }
            fclose(fd);
            return true;
        }
    }
    if(line)
    {
        free(line);
    }
    fclose(fd);
    return false;
}
/***********************************************************************************************************
**函数:
**输入参数:
**功能:
**返回值:
***********************************************************************************************************/
bool XW_DiskTools_checkDeviceIsRemoved(char *devPartitionsName)
{
	/*检测设置是否被移除*/

    FILE *procpt = NULL;
    char line[100], ptname[100], devname[120], *s;
    int ma, mi, sz;
    procpt = fopen(PROC_PARTITIONS, "r");
    if(procpt == NULL)
    {
        return false;
    }
    while(fgets(line, sizeof(line), procpt))
    {
        if(sscanf(line, " %d %d %d %[^\n ]", &ma, &mi, &sz, ptname) != 4)
            continue;
        for(s = ptname; *s; s++);
        if(isdigit(s[-1]))
            continue;
        sprintf(devname, "/dev/%s", ptname);
        if(memcmp(devPartitionsName, devname, strlen(devPartitionsName)) == 0)
        {
            fclose(procpt);
            return false;
        }
    }
    fclose(procpt);
    _DEBUG("dev %s is removed", devPartitionsName);
    return true;
}
/***********************************************************************************************************
**函数:
**输入参数:
**功能:
**返回值:硬盘 :
***********************************************************************************************************/
I32_T XW_DiskTools_CheckIsDiskOrUsbDisk(char *devname)
{
	/*检测是硬盘还是可移动设备*/

    FILE *fp;
    I32_T  ret = -1;
    CHAR_T buffer[80], * line = NULL;
    ssize_t size;
    size_t len = 0;

    memset(buffer, 0, 80);
    CHAR_T tmpfilename[64];
    if(tmpnam(tmpfilename) == NULL)
    {
        sprintf(tmpfilename, "%s", "/tmp/check_dev_type.txt");
    }
    fp = fopen(tmpfilename, "w+");
    sprintf(buffer, "/sys/block/%s/removable", devname);
    XW_Tools_DvrCpFile(buffer, tmpfilename, false, NULL);
    size = getline(&line, &len, fp);
    fclose(fp);
    ret = atoi(line);
    switch(ret)
    {
    case 0:
        ret = DEVTYPE_DISK;
        break;
    case 1:
        ret = DEVTYPE_U;
        break;
    default:
        ret = -1;
        break;
    }
    if(line)
    {
        free(line);
    }
    remove(tmpfilename);
    return ret;
}
/************************************************************************************************
**
**
************************************************************************************************/
int XW_DiskTools_GetPartitionFilesystem(char *part_name, int *systemId, char *system_type)
{
	/*获得文件系统类型*/

    ssize_t size;
    size_t len = 0;
    int ret;
    FILE *fd = NULL;
    char tmpfilename[64], buffer[64];
    char *line = NULL, devname[64], start[64], end[64], blocks[64], id[64];
    *systemId = 0;
    if(tmpnam(tmpfilename) == NULL)
    {
        sprintf(tmpfilename, "%s", "/tmp/getFilesystemType.txt");
    }
    sprintf(buffer, "fdisk -l | grep  /dev/%s  > %s", part_name, tmpfilename);
    ret = system(buffer);
    if(ret < 0)
    {
        printf("system cmd	 fdisk is error\n");
        remove(tmpfilename);
        return -1;
    }
    fd = fopen(tmpfilename, "r");
    if(fd == NULL)
    {
        remove(tmpfilename);
        return -1;
    }

    size = getline(&line, &len, fd);
    if(size > 0)
    {
        sscanf(line, "%s\t%s\t%s\t%s\t%s\t%s ", devname, start, end, blocks, id, system_type);
    }
    *systemId = atoi(id);
    if(line)
    {
        free(line);
        line = NULL;
    }

    fclose(fd);
    remove(tmpfilename);
    return 0;
}
/***********************************************************************************************************
**函数:
**输入参数:
**功能:
**返回值:
***********************************************************************************************************/
I32_T XW_DiskTools_GetDiskPartitionSize(char *pDevname , U32_T *pSize)
{
	/*获得分区的大小*/

    I32_T  i = 0;
    FILE    *fp = NULL;
    ssize_t size;
    size_t len = 0;
    CHAR_T   *line = NULL;
    CHAR_T *pname;
    CHAR_T  tmpfilename[64];
    CHAR_T *token;
    CHAR_T seps[] = " ";
    *pSize = 0;
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
        pname++;
        if(memcmp(pname, pDevname, strlen(pDevname)) == 0)
        {
            i = 0;
            token = strtok(line, seps);
            while(token != NULL)
            {
                if(i == 2)
                {
                    *pSize = atoi(token);
                }
                i++;
                token = strtok(NULL, seps);
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
    return 0;
}

/***********************************************************************************************************
**函数:
**输入参数:
**功能:
**返回值:
***********************************************************************************************************/
I32_T  XW_DiskTools_GetMountSpaceInfo(char *deviceName, unsigned long *tsize, unsigned long *fsize, unsigned long *asize, U8_T	unit)
{
	/*获得已挂载分区的，总空间，可利用空间，自由空间*/

    unsigned long	bsize;
    char	locate_disk[48];
    struct statfs s;

    memset(locate_disk, 0, 48);
    strcat(locate_disk, deviceName);
    statfs(locate_disk, &s);
    if(unit == MB_UNIT)
    {
        bsize = s.f_bsize >> 10;			 // in kbytes
        *tsize = (bsize * s.f_blocks) >> 10;	 // in MByte
        *fsize = (bsize * s.f_bfree) >> 10; 	 // in MByte
        *asize = (bsize * s.f_bavail) >> 10;	// in MByte
    }
    else if(unit == KB_UNIT)
    {
        bsize = s.f_bsize >> 10;			  // in kbytes
        *tsize = (bsize * s.f_blocks) ; 		  // in kbytes
        *fsize = (bsize * s.f_bfree) ;			 // in kbytes
        *asize = (bsize * s.f_bavail);			 // in kbytes
    }
    else if(unit == B_UNIT)
    {
        bsize = s.f_bsize;				// in bytes
        *tsize = (bsize * s.f_blocks) ; 	 // in bytes
        *fsize = (bsize * s.f_bfree) ;		  // in bytes
        *asize = (bsize * s.f_bavail);		 // in bytes
    }
    return *asize;
}
/*********************************************************************************************************
**
**
**********************************************************************************************************/
I32_T	XW_DiskTools_getDiskInfo(DiskMangeInfo_t *disk_info, U8_T disk_num, U32_T *free_size)
{
	/*获得硬盘信息*/

    char	tmpfilename[64];
    char	buffer[80];
    I32_T	error_sys;
    FILE	*fp = NULL;
    ssize_t size;
    size_t len = 0;
    CHAR_T	 *line = NULL;
    CHAR_T seps[] = " ";
    CHAR_T *token;
    U32_T used_space = 0;

    if(tmpnam(tmpfilename) == NULL)
    {
        sprintf(tmpfilename, "%s", "/tmp/getdiskinfo.txt");
    }
    sprintf(buffer, "du -shk %s > %s", disk_info->dev_disk[disk_num].Mount_Path[1], tmpfilename);
    if((error_sys = system(buffer)) != 0)
    {
        fprintf(stderr, "du cmd is error  : 0x%x\n", error_sys);
        return -1;
    }
    fp = fopen(tmpfilename, "r");
    while((size = getline(&line, &len, fp)) != -1)
    {
        token = strtok(line, seps);
        used_space = atoi(token);
        break;
    }

    *free_size = disk_info->dev_disk[disk_num].total_space - used_space;
    if(line)
    {
        free(line);
        line = NULL;
    }

    fclose(fp);
    remove(tmpfilename);
    return 0;
}
/***********************************************************************************************************
**函	数:
**输入参数:
**功	能:
**返回	值:
***********************************************************************************************************/
int  XW_DiskTools_Mke2fs(U8_T partNum, char *devname)
{
	/*格式化分区*/

    pid_t pid;
    int status = 0;
    int ret = false;

    if((pid = fork()) < 0)
    {
        status = -1;
    }
    else if(pid == 0)
    {
        char buffer[64];
        int tmp;
        if(partNum == 0)
        {
            sprintf(buffer, "mke2fs  /dev/%s%d	-j -b 4096 -m 0 -i 5000000 >  /dev/null", devname, partNum + 1);
        }
        else
        {
            sprintf(buffer, "mke2fs  /dev/%s%d -j -b 4096 -m 0	 -i  5000000 >  /dev/null", devname, partNum + 1);
        }
        tmp = system(buffer);
        if(tmp == 0)
        {
            _DEBUG("success	");
            _exit(0);
        }
        _exit(127);
    }
    else
    {
        waitpid(pid, &status, 0);
        if(status == 0)
        {
            ret = true;
            _DEBUG("format success ");
        }
    }
    return ret;
}

/***********************************************************************************************************
**函数:
**输入参数:
**功能:
**返回值:
***********************************************************************************************************/
int  XW_DiskTools_Tune2fs(U8_T partNum, char *devname)
{
    pid_t pid;
    int status = 0;
    int ret = false;

    if((pid = fork()) < 0)
    {
        status = -1;
    }
    else if(pid == 0)
    {
        char buffer[64];
        int tmp;

        sprintf(buffer, "tune2fs  -i 0 -c 0 /dev/%s%d", devname, partNum + 1);
        tmp = system(buffer);
        if(tmp == 0)
        {
            _DEBUG("success	");
            _exit(0);
        }
        _exit(127);
    }
    else
    {
        waitpid(pid, &status, 0);
        if(status == 0)
        {
            ret = true;
            _DEBUG("format success ");
        }
    }
    return ret;
}

/***********************************************************************************************************
**函数:
**输入参数:
**功能:
**返回值:
***********************************************************************************************************/
bool  XW_DiskTools_formatDdisk(Harddisk_t hd)
{
	/*格式化硬盘*/

    bool ret = true;
    U8_T i;
    I32_T tmp, tmp2;

    for(i = 0; i < hd.partition_count; i++)
    {
        tmp = XW_DiskTools_Mke2fs(i, hd.diskname);
        if(tmp == true)
        {
            tmp2 = XW_DiskTools_Tune2fs(i, hd.diskname);
            if(tmp == false)
            {
                ret = false;
                break;
            }
        }
        else
        {
            ret = false;
            break;
        }
    }
    return ret;
}
/***********************************************************************************************************
**函	数:
**输入参数:
**功	能:
**返回	值:
***********************************************************************************************************/
void XW_DiskTools_ChangeDiskPartition(U8_T disk_num, DiskMangeInfo_t info)
{
	/*改变硬盘分区*/

    I32_T ret;
    char tmpfilename[64];
    char buffer[128];
    U8_T j;
    FILE *fp;


    if(info.dev_disk[disk_num].partition_valid == 0)
    {
        if(tmpnam(tmpfilename) == NULL)
        {
            _ERROR("create temp file fail!\n");
            return ;
        }

        _DEBUG("system create tmp file name %s", tmpfilename);
        fp = fopen(tmpfilename, "w+");
        if(fp == NULL)
        {
            remove(tmpfilename);
            _ERROR("open %s fail!\n", tmpfilename);
            return ;
        }

        _DEBUG("DISK[%d] partition_count %d", disk_num, info.dev_disk[disk_num].partition_count);
        for(j = 0; j < info.dev_disk[disk_num].partition_count; j++)
        {
            fprintf(fp, "d\n");
            if((j + 1) != info.dev_disk[disk_num].partition_count)
                fprintf(fp, "%d\n", j + 1);
        }

        fprintf(fp, "n\n");    //add new partition
        fprintf(fp, "p\n");    //primary partition
        fprintf(fp, "1\n");    //partition id
        fprintf(fp, "5\n");    //start cylinder
        fprintf(fp, "6533\n");		 //end cylinder
        fprintf(fp, "t\n"); 	//set fs type
        fprintf(fp, "83\n");	//ext3	swap

        fprintf(fp, "n\n");    //add new partition
        fprintf(fp, "p\n");    //primary partition
        fprintf(fp, "2\n");    //partition id
        fprintf(fp, "6534\n");		 //start cylinder
        fprintf(fp, "\n");	   //end cylinder
        fprintf(fp, "t\n");    //set fs type
        fprintf(fp, "2\n");    //
        fprintf(fp, "83\n");	//ext3
        fprintf(fp, "w\n");    //write table

        fclose(fp);
        fp = NULL;

        memset(buffer, 0, 128);
        sprintf(buffer, "fdisk /dev/%s < %s > /dev/null", info.dev_disk[disk_num].diskname, tmpfilename);
        ret = system(buffer);
        if(ret < 0)
        {
            _ERROR("execute system command fail!....\n");
        }
        remove(tmpfilename);
    }
}

/***********************************************************************************************************
**函数:
**输入参数:
**功能:
**返回值:
***********************************************************************************************************/
void XW_DiskTools_mountHardDiskIsReadOnly(U8_T  disk_num, Harddisk_t	*hd)
{
	/*已只读方式挂载硬盘*/

    char buffer[84];
    U8_T j;
    int ret;
    for(j = 0; j < hd->partition_count; j++)
    {
        sprintf(buffer, " mount -t ext3 -r -o noatime,commit=60   /dev/%s%d %s	", hd->diskname, j + 1, hd->Mount_Path[j]);
        ret = system(buffer);
        if(ret < 0)
        {
            _ERROR("mount  error");
        }

        sprintf(buffer, "umount %s", hd->Mount_Path[j]);
        ret = system(buffer);
        if(ret < 0)
        {
            _ERROR("umount  error");
        }
    }
}
/***********************************************************************************************************
**函数:
**输入参数:
**功能:
**返回值:
***********************************************************************************************************/
bool  XW_DiskTools_mountHardDiskIsReadWrite(U8_T  disk_num, Harddisk_t  *hd)
{
	/*已读写方式挂载硬盘*/

    char buffer[84];
    U8_T j;
    int ret = false;
    bool res = false;

    for(j = 0; j < hd->partition_count; j++)
    {
        sprintf(buffer, " mount -t ext3  -o noatime   /dev/%s%d %s	", hd->diskname, j + 1, hd->Mount_Path[j]);
        ret = system(buffer);
        if(ret < 0)
        {
            _ERROR("  ");
        }
        else
        {
            res = true;
        }
    }
    return res;
}
/***********************************************************************************************************
**函数:
**输入参数:
**功能:
**返回值:
***********************************************************************************************************/
void XW_DiskTools_StopHardDiskWork(char	*name)
{

	/*停止硬盘工作，*/

    I32_T ret;
    char  buffer[64];
    sprintf(buffer, "/sbin/hdparm -f %s", name);
    ret = system(buffer);
    sprintf(buffer, "/sbin/hdparm -F %s", name);
    ret = system(buffer);
    sprintf(buffer, "/sbin/hdparm -Y  %s", name);
    ret = system(buffer);

}


