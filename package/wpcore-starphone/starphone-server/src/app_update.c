
#include <stdarg.h>
#include <stdio.h>

#include "include/xw_export.h"

/******************************************************************************
*****************************
**函数:
**输入参数:
**功能:
**返回值:
*******************************************************************************
****************************/
typedef enum
{
    UPDATE_BOOT,/*启动*/
    UPDATE_KERNEL,/*内核*/
    UPDATE_ROOTFS,/*根文件系统*/
    UPDATE_MODULES,/*驱动*/
    UPDATE_WEBS,/*CGI*/
    UPDATE_APP,/*应用*/
    UPDATE_LOGO,/*LOGO*/
    UPDATE_PICTURE,/*图片*/
    UPDATE_MTD,/**/
    UPDATE_OVER,/*升级结束*/
    UPDATE_NOFINDFINE,/*没有找到升级文件*/
}
UPDATETYPE_T;

typedef enum
{
    UPDATE_DOING,
    UPDATE_SUCCESS,
    UPDATE_ERROR,
} UPDATERES_T;

typedef struct
{
    char doType;/*UPDATETYPE_T  结构中的内容，类型为UPDATE_NOFINDDEV 
时为没有中的升级文件*/
    char res;/*0---正在升级，1----升级成功，2---升级失败*/
} UPDATEINFO_T;

#define UpdateBootFileName    "updateBootEwork.bin"/*BOOT*/
#define UpdateKernelFileName  "updateKernelEwork.bin"/*内核*/
#define UpdateRootfsFileName  "updateRootfsEwork.bin"/*文件系统*/
#define UpdateModulesFileName "updateModulesEwork.bin"/*驱动*/
#define UpdateWebsFileName     "updateWebsEwork.bin"/*WEB*/
//#define UpdateAppFileName      "updateAppEwork.bin"/*应用*/
//#define UpdateLogoFileName     "updateLogoEwork.jpg"/*LOGO*/
#define UpdateMtdFileName      "updateMtdEwork.bin"/*配置*/

#define UpdateAppFileName      "update.zip"/*应用*/
#define UpdateLogoFileName     "logo.jpg"/*LOGO*/

static bool update_state = false;


int XW_Update_writeImageToMTD(char *device, char *fileName, int sectionId);

#if 0
/******************************************************************************
*****************************
**函数:
**输入参数:
**功能:
**返回值:
*******************************************************************************
****************************/
static void XW_Update_DelUpdateFile(void)
{
    char fileName[64];
    sprintf(fileName, "/mnt/hddisk1/%s", UpdateBootFileName);
    if(access(fileName, F_OK) == 0)
    {
        remove(fileName);
    }

    sprintf(fileName, "/mnt/hddisk1/%s", UpdateKernelFileName);
    if(access(fileName, F_OK) == 0)
    {
        remove(fileName);
    }

    sprintf(fileName, "/mnt/hddisk1/%s", UpdateRootfsFileName);
    if(access(fileName, F_OK) == 0)
    {
        remove(fileName);
    }

    sprintf(fileName, "/mnt/hddisk1/%s", UpdateRootfsFileName);
    if(access(fileName, F_OK) == 0)
    {
        remove(fileName);
    }

    sprintf(fileName, "/mnt/hddisk1/%s", UpdateModulesFileName);
    if(access(fileName, F_OK) == 0)
    {
        remove(fileName);
    }

    sprintf(fileName, "/mnt/hddisk1/%s", UpdateWebsFileName);
    if(access(fileName, F_OK) == 0)
    {
        remove(fileName);
    }

    sprintf(fileName, "/mnt/hddisk1/%s", UpdateAppFileName);
    if(access(fileName, F_OK) == 0)
    {
        remove(fileName);
    }

    sprintf(fileName, "/mnt/hddisk1/%s", UpdateLogoFileName);
    if(access(fileName, F_OK) == 0)
    {
        remove(fileName);
    }

    sprintf(fileName, "/mnt/hddisk1/%s", UpdateMtdFileName);
    if(access(fileName, F_OK) == 0)
    {
        remove(fileName);
    }
}
#endif
/******************************************************************************
*****************************
**函数:
**输入参数:
**功能:
**返回值:
*******************************************************************************
****************************/
bool XW_Update_findUpdateFile(U32_T fileType, char *fullPathName)
{
    char fileName[64];
    char hddiskName[64];
    bool ret;
    
    switch(fileType)
    {
    case UPDATE_BOOT:
        sprintf(fileName, "%s", UpdateBootFileName);
        break;
    case UPDATE_KERNEL:
        sprintf(fileName, "%s", UpdateKernelFileName);
        break;
    case UPDATE_ROOTFS:
        sprintf(fileName, "%s", UpdateRootfsFileName);
        break;
    case UPDATE_MODULES:
        sprintf(fileName, "%s", UpdateModulesFileName);
        break;
    case UPDATE_WEBS:
        sprintf(fileName, "%s", UpdateWebsFileName);
        break;
    case UPDATE_APP:
        sprintf(fileName, "%s", UpdateAppFileName);
        break;
    case UPDATE_LOGO:
        sprintf(fileName, "%s", UpdateLogoFileName);
        break;
    case UPDATE_MTD:
    case UPDATE_PICTURE:
        sprintf(fileName, "%s", UpdateMtdFileName);
        break;
    }
    //_DEBUG("begin update file %s", fileName);
    
    ret = XW_Hotplug_FindFileInUsb(fileName, fullPathName);    
    if(ret == HI_TRUE)
    {
        _DEBUG("success find update file %s", fullPathName);
    }
    else
    {
        sprintf(hddiskName, "/mnt/hddisk1/%s", fileName);
        if(access(hddiskName, F_OK) == 0)
        {
            _DEBUG("success find update file %s", fileName);
            sprintf(fullPathName, "%s", hddiskName);
            ret = HI_TRUE;
        }
        else
        {
            _DEBUG("not find update file %s", fileName);
        }
    }
    return ret;
}
/******************************************************************************
*****************************
**函数:
**输入参数:
**功能:
**返回值:
*******************************************************************************
****************************/
bool  XW_Update_WriteFlash(U32_T fileType, char *fileName)
{
    int partationNum = 0;
    char buffer[80];
    switch(fileType)
    {
    case UPDATE_BOOT:
        partationNum = 0;
        break;
    case UPDATE_KERNEL:
        partationNum = 2;
        break;
    case UPDATE_ROOTFS:
        partationNum = 3;
        break;
    case UPDATE_MODULES:
        partationNum = 4;
        break;
    case UPDATE_LOGO:
        partationNum = 1;
        break;
    case UPDATE_WEBS:
        partationNum = 5;
        break;
    case UPDATE_MTD:
    case UPDATE_PICTURE:
        partationNum = 6;
        break;
    case UPDATE_APP:
        partationNum = 7;
        break;
    default :
        return HI_FALSE;
        break;
    }
    sprintf(buffer, "/dev/mtd%d", partationNum);
    XW_Update_writeImageToMTD(buffer, fileName, 0);
    return HI_TRUE;
}

int mypopen(const char *type, FILE *fp[2], char *execPro,...)
{
    int i;
    int pfd[2];
    int pfderr[2];
    pid_t pid;
    int Ret;
    va_list lpArgs;
    char *arg[8] = {0};

    if(pipe(pfd) < 0) return -1;
    if(pipe(pfderr) < 0)
    {
        close(pfd[0]);
        close(pfd[1]);
        return -1;
    }

    if((pid = fork()) < 0) return -1;
    else if(pid == 0)
    {
        if( *type == 'r' )
        {
            if(pfd[1] != STDOUT_FILENO)
                Ret = dup2(pfd[1],STDOUT_FILENO);
            if(pfderr[1] != STDERR_FILENO)
                Ret = dup2(pfderr[1],STDERR_FILENO);
        }
        else
        {
            if(pfd[0] != STDIN_FILENO)
                Ret = dup2(pfd[0],STDIN_FILENO);
            if(pfderr[0] != STDERR_FILENO)
                Ret = dup2(pfderr[0],STDERR_FILENO);
        }
        arg[0] = execPro;
        i = 1;
        va_start(lpArgs,execPro);
        arg[i] = va_arg(lpArgs, char *);
        while(arg[i] != NULL)
        {
            i++;
            if(i >= 8) break;
            arg[i] = va_arg(lpArgs,char *);
        }
        arg[i] = NULL;
        Ret = execv(execPro,arg);
        va_end(lpArgs);
        _exit(0);
    }
    if( *type == 'r' )
    {
        close(pfd[1]);
        close(pfderr[1]);
        if((fp[0] = fdopen(pfd[0],type)) == NULL)   
        {
            close(pfd[0]);
            close(pfderr[0]);
            return -1;
        }
        if((fp[1] = fdopen(pfderr[0],type)) == NULL)   
        {
            fclose(fp[0]);
            close(pfd[0]);
            close(pfderr[0]);
            return -1;
        }
    }
    else
    {
        close(pfd[0]);
        close(pfderr[0]);
        if((fp[0] = fdopen(pfd[1],type)) == NULL)
        {
            close(pfd[1]);
            close(pfderr[1]);
            return -1;
        }
        if((fp[1] = fdopen(pfderr[1],type)) == NULL)
        {
            fclose(fp[0]);
            close(pfd[1]);
            close(pfderr[1]);
            return -1;
        }
    }
    return pid;
}

int myfread(FILE *fp[2], void *RetVal, int RetSize, int Timeout)
{
    int size = 0,Ret,RecvSize,MaxFD;
    int Flag[2] = {0,0};
    struct timeval tv;
    fd_set fdR;

    MaxFD = mymax(fileno(fp[0]),fileno(fp[1]));
    while(size < RetSize)
    {
        memset(&tv,0,sizeof(tv));
        tv.tv_sec = Timeout;
        tv.tv_usec = 0;

        FD_ZERO(&fdR);
        FD_SET(fileno(fp[0]),&fdR);
        FD_SET(fileno(fp[1]),&fdR);
        Ret = select(MaxFD+1,&fdR,NULL,NULL,&tv);
        if(Ret == 0)
        {
            return -1;
        }
        else if(Ret > 0)
        {
            if(FD_ISSET(fileno(fp[0]),&fdR))
            {
                RecvSize = read(fileno(fp[0]),(char *)RetVal+size,RetSize-size
);
                if(RecvSize == 0) Flag[0] = 1;
                else if(RecvSize < 0) return -1;
                else size += RecvSize;
            }
            if(FD_ISSET(fileno(fp[1]),&fdR))
            {
                //RecvSize = read(fileno(fp[1]),RetVal+size,RetSize-size);
                //if(RecvSize == 0) Flag[0] = 1;
                //else if(RecvSize < 0) return -1;
                //else size += RecvSize;
                //_ERROR("Script has occur error");
            }
        }
        else
        {
            return -1;
        }

        if((Flag[0] == 1 || Flag[1] == 1) && (size <= RetSize))
            return 0;
    }
    return 1;
}

int mypclose(FILE *fp[2], int *processPid)
{
    int states;
    pid_t pid;
    pid_t pid_tmp;
    int ret;

    if(fclose(fp[0]) == EOF) return -1;
    if(fclose(fp[1]) == EOF) return -1;
    fp[0] = NULL;
    fp[1] = NULL;

    if((pid = *processPid) == 0) return -1;
    ret = waitpid(pid,&states,WNOHANG);
    if(0 == ret)
    {
        kill(pid,SIGKILL);
        do{
            pid_tmp = waitpid(pid,&states,0);
        }while(pid_tmp == -1 && errno == EINTR);
    }
    
    *processPid = 0;
    return states;
}


/******************************************************************************
*****************************
**函数:
**输入参数:
**功能:
**返回值:
*******************************************************************************
****************************/
void *XW_Update_pthread(void *argv)
{
    char fullName[96] = {'\0'};
    bool resLogo = false, hasUpdateFile = false;
    bool hasLogoFile = false;    
    bool Update_Flags = HI_TRUE;
    bool needUpdatesystem = false;
    int retTmpfs,sysret;
    char versionInfo[96] = {'\0'};
    char pathUsb[96] = {'\0'};
    PTHREAD_BUF     signal;
    STATE_PREVIEW *p;
    MspSendCmd_t updateInfo;
    
    DVR_PTHREAD_DETACH;

    p = (STATE_PREVIEW *)XW_ManagePthread_GetPthreadState(PTHREAD_UPDATE_ID, 0);
    p->power = PTHREAD_POWER_ON;
    p->state = ALIVE;
	
    _DEBUG("Usb update thread running........");

    while(p->power == PTHREAD_POWER_ON)
	{
	    update_state = false;
		XW_ManagePthread_ReadSignal(&signal, PTHREAD_UPDATE_ID, true);               
		if (signal.start_id != PTHREAD_USBHOTPLUG_ID)
                        continue;

        update_state = true;
        XW_Hotplug_CheckUsbDiskDev(NULL); 

        // 先判断升级文件update.zip是否存在
        hasUpdateFile = XW_Update_findUpdateFile(UPDATE_APP, fullName);
        if(hasUpdateFile != HI_TRUE)//存在
            continue;
        
        // 判断是否需要进行升级 -- mount -t tmpfs 
        if(access("/var/ramdisk", F_OK) != 0)// check dir is exist or not
            mkdir("/var/ramdisk", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        retTmpfs = mount("tmpfs", "/var/ramdisk", "tmpfs", MS_NOATIME | MS_NODIRATIME, "size=32M,mode=0755");            
        if(retTmpfs < 0)
        {
            _ERROR("[ErrNo: %d] mount tmpfs to /var/ramdisk", retTmpfs);  
            perror("mount:");
            //Update_Flags = HI_FALSE;    
            continue;
        }
        else
            _DEBUG("mount -t vfat tmpfs /var/ramdisk -o size=16M,mode=0755 Ok !");
        
        // unzip file
        char unzipcmd[64] = {'\0'};
        sprintf(unzipcmd,"unzip %s -d /var/ramdisk/",fullName);
        sysret = system(unzipcmd);
        if(sysret != -1 && WIFEXITED(sysret) && WEXITSTATUS(sysret) == 0)
        {
            _DEBUG("unzip /var/ramdisk/update.zip Ok !");
        }
        else
        {
            _ERROR("unzip update.zip file failed %d.",sysret);
            umount("/var/ramdisk");
            continue;
        }
        //check update_install.ini
        if(access("/var/ramdisk/updateInfo.ini", F_OK) == 0)
        {
            unsigned char i = 0;
            char *line_new = NULL;
            char *line_old = NULL;
            char *line_tmp = NULL;
            FILE *fp_new = NULL;
            FILE *fp_old = NULL;
            size_t size,len =0;
            
            fp_new = fopen("/var/ramdisk/updateInfo.ini", "r");
            while((size = getline(&line_new, &len, fp_new)) != -1)
            {
                if(line_new != NULL)
                {
                    for(i = 0; *(line_new+i) != '\0'; i++)
                    {
                        if(line_new[i] == '\n' || line_new[i] == '\r')
                            line_new[i] = '\0';
                    }        
                    line_tmp = line_new;
                    while(*line_tmp == ' ' || *line_tmp == '\t')
                        line_tmp++;
                    if(strncmp(line_tmp,"#VERSION",8) == 0)
                    {
                        memset(versionInfo,0,sizeof(versionInfo));
                        strcpy(versionInfo,line_tmp);
                        _DEBUG("versionInfo: %s",versionInfo);
                        break;
                    }
                }
            }
            if(access("/root/version.txt", F_OK) == 0)
            {
                fp_old = fopen("/root/version.txt", "r");
                while((size = getline(&line_old, &len, fp_old)) != -1)
                {
                    if(line_old != NULL)
                        if(strncmp(line_old,"#VERSION",8) == 0)
                            break;
                }
            
                if(strcmp(line_old,line_tmp) != 0)
                {
                    needUpdatesystem = true;
                    _DEBUG("old: %s  new: %s",line_old,line_tmp);
                }
                else
                {
                    needUpdatesystem = false;                    
                    _DEBUG("The same version, not need update system");

                }
            }
            else
            {
                needUpdatesystem = true; 
                _DEBUG("/root/version.txt not exist");
            }
            
            if(fp_old != NULL)
                fclose(fp_old);
            fclose(fp_new); 
            if(needUpdatesystem == false)
            {
                umount("/var/ramdisk");
                _DEBUG("Not need update system ...");
                continue;
            }

        }
        else
        {
            needUpdatesystem = false;
            umount("/var/ramdisk");
            _DEBUG("Not need update system ...");
            continue;
        }             

        //---------------------------------------------------------------------
        if(needUpdatesystem == true && Update_Flags == HI_TRUE)
        {           
            updateInfo.ctrlModel = MSP_CMD_SYSTEMUPGRADE;
            updateInfo.cmdFunc = MSP_CMD_UPDATE_BEGIN;
            XW_MsgQueue_Send(MSP_CMD_SYSTEMUPGRADE, &updateInfo, SEND_MESSAGE_TO_UI);
            XW_MsgQueue_Receive(SEND_MESSAGE_TO_SLAVE);
            _DEBUG("Need update system, Start system upgrade ...");
            
            sleep(2);
            //update versiom.txt
            if(access("/root/version.txt", F_OK) == 0)
                remove("/root/version.txt");
            FILE* fp_ver = fopen("/root/version.txt", "w+");
            fprintf(fp_ver,"%s",versionInfo);
            fclose(fp_ver);
            _DEBUG("Update version information Ok !");
            sleep(1);
            //copy logo and script 
            //if(access("/var/ramdisk/logo.jpg", F_OK) == 0)
            //{
            //    system("cp -f /var/ramdisk/logo.jpg /tmp/logo.jpg");
            //    _DEBUG("copy /var/ramdisk/logo.jpg  ---> /tmp/logo,jpg Ok !");
            //}            

            updateInfo.ctrlModel = MSP_CMD_SYSTEMUPGRADE;
            updateInfo.cmdFunc = MSP_CMD_UPDATE_INITOK;
            XW_MsgQueue_Send(MSP_CMD_SYSTEMUPGRADE, &updateInfo, SEND_MESSAGE_TO_UI);
            XW_MsgQueue_Receive(SEND_MESSAGE_TO_SLAVE);
        }
 
        //--------------------- back up system -----------------------------
        if(needUpdatesystem == true && Update_Flags == HI_TRUE)
        {            
            char backupcmd[96] = {'\n'};  
            memset(pathUsb,'\0',sizeof(pathUsb));
            strncpy(pathUsb,fullName,strlen(fullName) - 11 );
            sprintf(backupcmd,"tar -czvf %s/backup_dvrapp.tar.gz /root",pathUsb);
            sysret = system(backupcmd);
            if(sysret != -1 && WIFEXITED(sysret) && WEXITSTATUS(sysret) == 0)
            {
                _DEBUG("Backup system application ---> %s Ok !",pathUsb);
            
                updateInfo.ctrlModel = MSP_CMD_SYSTEMUPGRADE;
                updateInfo.cmdFunc = MSP_CMD_UPDATE_BACKUP;
                XW_MsgQueue_Send(MSP_CMD_SYSTEMUPGRADE, &updateInfo, SEND_MESSAGE_TO_UI);
                XW_MsgQueue_Receive(SEND_MESSAGE_TO_SLAVE);
                sleep(3);
            }
            else
            {
                Update_Flags = HI_FALSE;
                umount("/var/ramdisk");
                _ERROR("Backup system failed, give up update system.");
            }
        }
        //--------------------- copy file to dir ---------------------------
        if(needUpdatesystem == true && Update_Flags == HI_TRUE)
        {
            unsigned char i;
            FILE *fp_copy = NULL;
            char *line_copy = NULL;
            char *line_pro = NULL;
            size_t size = 0, len = 0;
            char dirpath[96] = {'\0'};
            char dstPath[96] = {'\0'};
            char srcPath[96] = {'\0'};
            char copycmd[256] = {'\0'};            

            _DEBUG("Start update application files.");
            system("dos2unix  /var/ramdisk/updateInfo.ini");
            fp_copy = fopen("/var/ramdisk/updateInfo.ini", "r");
            while((size = getline(&line_copy, &len, fp_copy)) != -1)
            {
                memset(srcPath,'\0',sizeof(srcPath));
                memset(copycmd,'\0',sizeof(copycmd));
                memset(dirpath,'\0',sizeof(dirpath));
                memset(dstPath,'\0',sizeof(dirpath));
                //_DEBUG("[getline] %s",line_copy);
                
                if(line_copy[0] == '\n' || line_copy[0] == '#')
                {
                    //_DEBUG("line == %s",line_copy);
                    continue;
                }
                
                // delete blank space for each line
                line_pro = line_copy;
                while(*line_pro == ' ' || *line_pro == '\t')
                    line_pro++;
                if(line_pro[0] == '\n' || line_pro[0] == '#')
                {
                    _DEBUG("line == %s",line_pro);
                    continue;
                }
                for(i = 0; *(line_pro+i) != '\0'; i++)
                {
                    if(line_copy[i] == '\n' || line_copy[i] == '\r')
                        line_pro[i] = '\0';
                }                
                
                sprintf(srcPath,"/var/ramdisk/%s",line_pro);                
                if(access(srcPath, F_OK) != 0)
                {
                    _DEBUG("Waring: file %s not exist, not copy. ",srcPath);
                    continue;
                }
                sprintf(dstPath,"/%s",line_pro);
                memcpy(dirpath,dstPath,strlen(dstPath)-strlen(strrchr(dstPath,'/')));
                if(access(dirpath, F_OK) != 0)// check dir is exist or not
                {                  
                    mkdir(dirpath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
                    _DEBUG("creat director %s",dirpath);
                }
                sprintf(copycmd,"cp -rf %s /%s",srcPath,line_pro);
                chmod(srcPath,0777);
                sysret = system(copycmd);
                if(sysret != -1 && WIFEXITED(sysret) && WEXITSTATUS(sysret) == 0)
                {
                    _DEBUG("[cp -rf] %s  --->  /%s",srcPath,line_pro);
                }
                else
                {
                    _ERROR("Update system failed, perpre reback system...");
                    Update_Flags = HI_FALSE;
                    sprintf(copycmd,"tar -xzvf %s/backup_dvrapp.tar.gz -C /",pathUsb);
                    system(copycmd);   
                    break;
                }
                
            }
            fclose(fp_copy);

            updateInfo.ctrlModel = MSP_CMD_SYSTEMUPGRADE;
            updateInfo.cmdFunc = MSP_CMD_UPDATE_DOING;
            XW_MsgQueue_Send(MSP_CMD_SYSTEMUPGRADE, &updateInfo, SEND_MESSAGE_TO_UI);
            XW_MsgQueue_Receive(SEND_MESSAGE_TO_SLAVE);
        }
        //--------------------- release reourse ----------------------------
        if(needUpdatesystem == true && Update_Flags == HI_TRUE)
        {            
            sleep(1);
            umount("/var/ramdisk");
            sleep(1);
            updateInfo.ctrlModel = MSP_CMD_SYSTEMUPGRADE;
            updateInfo.cmdFunc = MSP_CMD_UPDATE_END;
            XW_MsgQueue_Send(MSP_CMD_SYSTEMUPGRADE, &updateInfo, SEND_MESSAGE_TO_UI);
            XW_MsgQueue_Receive(SEND_MESSAGE_TO_SLAVE);

        }
        if(needUpdatesystem == true && Update_Flags == HI_FALSE)
        {
            updateInfo.ctrlModel = MSP_CMD_SYSTEMUPGRADE;
            updateInfo.cmdFunc = MSP_CMD_UPDATE_ERROR;
            XW_MsgQueue_Send(MSP_CMD_SYSTEMUPGRADE, &updateInfo, SEND_MESSAGE_TO_UI);
            XW_MsgQueue_Receive(SEND_MESSAGE_TO_SLAVE);
        }
        
        //-----------------------------Update Logo-----------------------//
        sprintf(fullName,"/%s",UpdateLogoFileName);
        if(hasUpdateFile == HI_TRUE )
        {
            if(access(fullName, F_OK) == 0)
            {
                hasLogoFile = HI_TRUE;
                updateInfo.ctrlModel = MSP_CMD_SYSTEMUPGRADE;
                updateInfo.cmdFunc = MSP_CMD_UPDATE_LOGO;
                XW_MsgQueue_Send(MSP_CMD_SYSTEMUPGRADE, &updateInfo, SEND_MESSAGE_TO_UI);
                XW_MsgQueue_Receive(SEND_MESSAGE_TO_SLAVE);
                _DEBUG("Logo file access: %s",fullName);
            }
            else
            {
                hasLogoFile = HI_FALSE;
                _DEBUG("logo file: %s not exist !",fullName);
            }
        }
        else
            hasLogoFile = HI_FALSE;
      
        if(hasUpdateFile == HI_TRUE && hasLogoFile == HI_TRUE)
        {        
            sleep(2);
            resLogo = XW_Update_WriteFlash(UPDATE_LOGO, fullName);
            if(resLogo == HI_TRUE)
            {
                updateInfo.ctrlModel = MSP_CMD_SYSTEMUPGRADE;
                updateInfo.cmdFunc = MSP_CMD_UPDATE_LOGOOK;
                XW_MsgQueue_Send(MSP_CMD_SYSTEMUPGRADE, &updateInfo, SEND_MESSAGE_TO_UI);
                XW_MsgQueue_Receive(SEND_MESSAGE_TO_SLAVE);
            }
            else
            {
                updateInfo.ctrlModel = MSP_CMD_SYSTEMUPGRADE;
                updateInfo.cmdFunc = MSP_CMD_UPDATE_LOGOERR;
                XW_MsgQueue_Send(MSP_CMD_SYSTEMUPGRADE, &updateInfo, SEND_MESSAGE_TO_UI);
                XW_MsgQueue_Receive(SEND_MESSAGE_TO_SLAVE);
            }
           
            // update success, delete update files 
            if(access(fullName, F_OK) == 0)
            {
                remove(fullName);
                _DEBUG("Delete logo file : %s",fullName);
            }
        } 

        if((hasUpdateFile == HI_TRUE && needUpdatesystem == HI_TRUE) || hasLogoFile == HI_TRUE)
        {
            if(resLogo && Update_Flags)
            {
                _DEBUG("Update Application and Logo were success !");
            }
            else if(!resLogo && Update_Flags)
            {
                _DEBUG("Update Application success, but update Logo failed!");
            }
            else if(resLogo && !Update_Flags)
            {
                _DEBUG("Update Logo success, but update Application failed !");
            }
            else
            {
                _ERROR("Update Application and Logo were failed !");
            }
            _DEBUG("System will reboot after 3 second !");
            sleep(3);            
            system("reboot");
        }

        update_state = false;
        
    }
    
    p->state = EXIT;
    return NULL;
}


#if 0
/******************************************************************************
*****************************
**函数:
**输入参数:
**功能:
**返回值:
*******************************************************************************
****************************/
void *XW_Update_pthread(void *argv)
{
    char fullName[80] = {0};
    bool res, hasUpdateFile = false;
    bool hasLogoFile = false;
    MspSendCmd_t updateInfo;
    FILE *stream[2];
    char buf[512];
    int pid;
    int Update_Flags = HI_TRUE;

    DVR_PTHREAD_DETACH;

    // 先判断升级文件update.zip是否存在
    hasUpdateFile = XW_Update_findUpdateFile(UPDATE_APP, fullName);
    if(hasUpdateFile == HI_TRUE && Update_Flags == HI_TRUE)//存在
    {   // init
        updateInfo.ctrlModel = MSP_CMD_SYSTEMUPGRADE;
        updateInfo.cmdFunc = MSP_CMD_UPDATE_BEGIN;
        XW_MsgQueue_Send(MSP_CMD_SYSTEMUPGRADE, &updateInfo, 
SEND_MESSAGE_TO_UI);
        //XW_MsgQueue_Receive(SEND_MESSAGE_TO_SLAVE);
        _DEBUG("Start system upgrade ...");

        pid = mypopen("r",stream,"/etc/update.sh","-i",fullName,NULL);
        if(pid > 0)
        {
            memset(buf,0,sizeof(buf));
            if(myfread(stream, buf, sizeof(buf),30) == 0)
            {                
                if(strncmp(buf,"UPDATE_TRUE",11) == 0)// can update
                {                    
                    updateInfo.ctrlModel = MSP_CMD_SYSTEMUPGRADE;
                    updateInfo.cmdFunc = MSP_CMD_UPDATE_INITOK;
                    XW_MsgQueue_Send(MSP_CMD_SYSTEMUPGRADE, &updateInfo, 
SEND_MESSAGE_TO_UI);
                    //XW_MsgQueue_Receive(SEND_MESSAGE_TO_SLAVE);
                    Update_Flags = HI_TRUE;
                    _DEBUG("System upgrade is ready !");
                }
                else
                {
                    Update_Flags = HI_FALSE;
                    _ERROR("%s",buf);
                }
            }
            else
            {
                Update_Flags = HI_FALSE;
                _ERROR("%s",buf);
            }
        }
        mypclose(stream, &pid);
    }
    if(hasUpdateFile == HI_TRUE && Update_Flags == HI_TRUE)//存在
    {//backup application system -b
        pid = mypopen("r",stream,"/etc/update.sh","-b",fullName,NULL);
        if(pid > 0)
        {
            memset(buf,0,sizeof(buf));
            if(myfread(stream, buf, sizeof(buf),30) == 0)
            {
                if(strncmp(buf,"UPDATE_TRUE",11) == 0)
                {                    
                    updateInfo.ctrlModel = MSP_CMD_SYSTEMUPGRADE;
                    updateInfo.cmdFunc = MSP_CMD_UPDATE_BACKUP;
                    XW_MsgQueue_Send(MSP_CMD_SYSTEMUPGRADE, &updateInfo, 
SEND_MESSAGE_TO_UI);
                    //XW_MsgQueue_Receive(SEND_MESSAGE_TO_SLAVE);
                    Update_Flags = HI_TRUE;
                    _DEBUG("Backup system before upgrade !");
                }
                else
                {
                    Update_Flags = HI_FALSE;
                    _ERROR("%s",buf);
                }
            }
            else
            {
                Update_Flags = HI_FALSE;
                _ERROR("%s",buf);
            }
        }
        mypclose(stream, &pid);
    }
    if(hasUpdateFile == HI_TRUE && Update_Flags == HI_TRUE)//存在
    {//update application system -u
        pid = mypopen("r",stream,"/etc/update.sh","-u",fullName,NULL);
        if(pid > 0)
        {
            memset(buf,0,sizeof(buf));
            if(myfread(stream, buf, sizeof(buf),30) == 0)
            {
                if(strncmp(buf,"UPDATE_TRUE",11) == 0)
                {                    
                    updateInfo.ctrlModel = MSP_CMD_SYSTEMUPGRADE;
                    updateInfo.cmdFunc = MSP_CMD_UPDATE_DOING;
                    XW_MsgQueue_Send(MSP_CMD_SYSTEMUPGRADE, &updateInfo, 
SEND_MESSAGE_TO_UI);
                    //XW_MsgQueue_Receive(SEND_MESSAGE_TO_SLAVE);
                    Update_Flags = HI_TRUE;
                    _DEBUG("System upgrade now ...");
                }
                else
                {
                    Update_Flags = HI_FALSE;
                    _ERROR("%s",buf);
                }
            }
            else
            {
                Update_Flags = HI_FALSE;
                _ERROR("%s",buf);
            }
        }
        mypclose(stream, &pid);
    }
    if(hasUpdateFile == HI_TRUE && Update_Flags == HI_TRUE)//存在
    {//delete reoures -e
        pid = mypopen("r",stream,"/etc/update.sh","-e",fullName,NULL);
        if(pid > 0)
        {
            memset(buf,0,sizeof(buf));
            if(myfread(stream, buf, sizeof(buf),30) == 0)
            {               
                if(strncmp(buf,"UPDATE_TRUE",11) == 0)
                {                    
                    updateInfo.ctrlModel = MSP_CMD_SYSTEMUPGRADE;
                    updateInfo.cmdFunc = MSP_CMD_UPDATE_END;
                    XW_MsgQueue_Send(MSP_CMD_SYSTEMUPGRADE, &updateInfo, 
SEND_MESSAGE_TO_UI);
                    //XW_MsgQueue_Receive(SEND_MESSAGE_TO_SLAVE);
                    Update_Flags = HI_TRUE;
                    _DEBUG("System upgrade end ...");
                }
                else
                {
                    Update_Flags = HI_FALSE;
                    _ERROR("%s",buf);
                }
            }
            else
            {
                Update_Flags = HI_FALSE;
                _ERROR("%s",buf);
            }
        }
        mypclose(stream, &pid);
    }
    if(hasUpdateFile == HI_TRUE && Update_Flags == HI_FALSE)//存在
    {//update application failed
        updateInfo.ctrlModel = MSP_CMD_SYSTEMUPGRADE;
        updateInfo.cmdFunc = MSP_CMD_UPDATE_ERROR;
        XW_MsgQueue_Send(MSP_CMD_SYSTEMUPGRADE, &updateInfo, 
SEND_MESSAGE_TO_UI);
        //XW_MsgQueue_Receive(SEND_MESSAGE_TO_SLAVE);
        _ERROR("System upgrade Error ...");
    }

    //-----------------------------Update Logo-------------------------------
----//
    sprintf(fullName,"/tmp/%s",UpdateLogoFileName);
    if(hasUpdateFile == HI_TRUE && access(fullName, F_OK) == 0)
    {
        hasLogoFile = HI_TRUE;
        updateInfo.ctrlModel = MSP_CMD_SYSTEMUPGRADE;
        updateInfo.cmdFunc = MSP_CMD_UPDATE_LOGO;
        XW_MsgQueue_Send(MSP_CMD_SYSTEMUPGRADE, &updateInfo, 
SEND_MESSAGE_TO_UI);
        //XW_MsgQueue_Receive(SEND_MESSAGE_TO_SLAVE);
        _DEBUG("Logo file access: %s",fullName);
    }
    else
        hasLogoFile = HI_FALSE;
  
    if(hasUpdateFile == HI_TRUE && hasLogoFile == HI_TRUE)
    {        
        sleep(2);
        res = XW_Update_WriteFlash(UPDATE_LOGO, fullName);
        if(res == HI_TRUE)
        {
            updateInfo.ctrlModel = MSP_CMD_SYSTEMUPGRADE;
            updateInfo.cmdFunc = MSP_CMD_UPDATE_LOGOOK;
            XW_MsgQueue_Send(MSP_CMD_SYSTEMUPGRADE, &updateInfo, 
SEND_MESSAGE_TO_UI);
            //XW_MsgQueue_Receive(SEND_MESSAGE_TO_SLAVE);
        }
        else
        {
            updateInfo.ctrlModel = MSP_CMD_SYSTEMUPGRADE;
            updateInfo.cmdFunc = MSP_CMD_UPDATE_LOGOERR;
            XW_MsgQueue_Send(MSP_CMD_SYSTEMUPGRADE, &updateInfo, 
SEND_MESSAGE_TO_UI);
            //XW_MsgQueue_Receive(SEND_MESSAGE_TO_SLAVE);
        }
       
        // update success, delete update files 
        if(access(fullName, F_OK) == 0)
        {
            remove(fullName);
            _DEBUG("Delete logo file : %s",fullName);
        }
    } 
    else
        _DEBUG("logo file: %s not exist !",fullName);

    if(hasUpdateFile == HI_TRUE || hasLogoFile == HI_TRUE)
    {
        sleep(1);
        system("reboot");
    }
    update_state = false;
    //XW_Update_DelUpdateFile();
    
    return NULL;
}

/******************************************************************************
*****************************
**函数:
**输入参数:
**功能:
**返回值:
*******************************************************************************
****************************/
void *XW_Update_pthread(void *argv)
{
    char fullName[80] = {0};
    bool res, hasUpdateFile = false;
    bool hasLogoFile = false;
    MspSendCmd_t updateInfo;
    //FILE *stream[2];
    FILE *fp = NULL;
    char buf[512] = {'\n'};
    char cmdStr[128] ={'\n'};
    //int pid;
    int Update_Flags = HI_TRUE;

    DVR_PTHREAD_DETACH;

    // 先判断升级文件update.zip是否存在
    hasUpdateFile = XW_Update_findUpdateFile(UPDATE_APP, fullName);
    if(hasUpdateFile == HI_TRUE && Update_Flags == HI_TRUE)//存在
    {   // init
        updateInfo.ctrlModel = MSP_CMD_SYSTEMUPGRADE;
        updateInfo.cmdFunc = MSP_CMD_UPDATE_BEGIN;
        XW_MsgQueue_Send(MSP_CMD_SYSTEMUPGRADE, &updateInfo, 
SEND_MESSAGE_TO_UI);
        XW_MsgQueue_Receive(SEND_MESSAGE_TO_SLAVE);
        _DEBUG("Start system upgrade ...");
        
        sprintf(cmdStr,"/etc/update.sh %s %s","-i",fullName);
        fp = popen(cmdStr,"r");
        if(NULL != fp)
        {
            memset(buf,'\n',sizeof(buf));
            if(fread(buf, sizeof(char),sizeof(buf),fp) > 0)
            {                
                if(strncmp(buf,"UPDATE_TRUE",11) == 0)// can update
                {                    
                    updateInfo.ctrlModel = MSP_CMD_SYSTEMUPGRADE;
                    updateInfo.cmdFunc = MSP_CMD_UPDATE_INITOK;
                    XW_MsgQueue_Send(MSP_CMD_SYSTEMUPGRADE, &updateInfo, 
SEND_MESSAGE_TO_UI);
                    XW_MsgQueue_Receive(SEND_MESSAGE_TO_SLAVE);
                    Update_Flags = HI_TRUE;
                    _DEBUG("System upgrade is ready !");
                }
                else
                {
                    Update_Flags = HI_FALSE;
                    _ERROR("%s",buf);
                }
            }
            else
            {
                Update_Flags = HI_FALSE;
                _ERROR("%s",buf);
            }
            pclose(fp);
        }        
    }
    if(hasUpdateFile == HI_TRUE && Update_Flags == HI_TRUE)//存在
    {//backup application system -b
        sprintf(cmdStr,"/etc/update.sh %s %s","-b",fullName);
        fp = popen(cmdStr,"r");
        if(NULL != fp)
        {
            memset(buf,'\n',sizeof(buf));
            if(fread(buf, sizeof(char),sizeof(buf),fp) > 0)
            {
                if(strncmp(buf,"UPDATE_TRUE",11) == 0)
                {                    
                    updateInfo.ctrlModel = MSP_CMD_SYSTEMUPGRADE;
                    updateInfo.cmdFunc = MSP_CMD_UPDATE_BACKUP;
                    XW_MsgQueue_Send(MSP_CMD_SYSTEMUPGRADE, &updateInfo, 
SEND_MESSAGE_TO_UI);
                    XW_MsgQueue_Receive(SEND_MESSAGE_TO_SLAVE);
                    Update_Flags = HI_TRUE;
                    _DEBUG("Backup system before upgrade ok !");
                }
                else
                {
                    Update_Flags = HI_FALSE;
                    _ERROR("%s",buf);
                }
            }
            else
            {
                Update_Flags = HI_FALSE;
                _ERROR("%s",buf);
            }
            pclose(fp);
        }        
    }
    if(hasUpdateFile == HI_TRUE && Update_Flags == HI_TRUE)//存在
    {//update application system -u
        sprintf(cmdStr,"/etc/update.sh %s %s","-u",fullName);
        fp = popen(cmdStr,"r");
        if(NULL != fp)
        {
            memset(buf,'\n',sizeof(buf));
            if(fread(buf, sizeof(char),sizeof(buf),fp) > 0)
            {
                if(strncmp(buf,"UPDATE_TRUE",11) == 0)
                {                    
                    updateInfo.ctrlModel = MSP_CMD_SYSTEMUPGRADE;
                    updateInfo.cmdFunc = MSP_CMD_UPDATE_DOING;
                    XW_MsgQueue_Send(MSP_CMD_SYSTEMUPGRADE, &updateInfo, 
SEND_MESSAGE_TO_UI);
                    XW_MsgQueue_Receive(SEND_MESSAGE_TO_SLAVE);
                    Update_Flags = HI_TRUE;
                    _DEBUG("System upgrade copy files success ...");
                }
                else
                {
                    Update_Flags = HI_FALSE;
                    _ERROR("%s",buf);
                }
            }
            else
            {
                Update_Flags = HI_FALSE;
                _ERROR("%s",buf);
            }
            pclose(fp);
        }        
    }
    if(hasUpdateFile == HI_TRUE && Update_Flags == HI_TRUE)//存在
    {//delete reoures -e
        sprintf(cmdStr,"/etc/update.sh %s %s","-e",fullName);
        fp = popen(cmdStr,"r");
        if(NULL != fp)
        {
            memset(buf,'\n',sizeof(buf));
            if(fread(buf, sizeof(char),sizeof(buf),fp) > 0)
            {               
                if(strncmp(buf,"UPDATE_TRUE",11) == 0)
                {                    
                    updateInfo.ctrlModel = MSP_CMD_SYSTEMUPGRADE;
                    updateInfo.cmdFunc = MSP_CMD_UPDATE_END;
                    XW_MsgQueue_Send(MSP_CMD_SYSTEMUPGRADE, &updateInfo, 
SEND_MESSAGE_TO_UI);
                    XW_MsgQueue_Receive(SEND_MESSAGE_TO_SLAVE);
                    Update_Flags = HI_TRUE;
                    _DEBUG("System upgrade success ...");
                }
                else
                {
                    Update_Flags = HI_FALSE;
                    _ERROR("%s",buf);
                }
            }
            else
            {
                Update_Flags = HI_FALSE;
                _ERROR("%s",buf);
            }
            pclose(fp);
        }        
    }
    if(hasUpdateFile == HI_TRUE && Update_Flags == HI_FALSE)//存在
    {//update application failed
        updateInfo.ctrlModel = MSP_CMD_SYSTEMUPGRADE;
        updateInfo.cmdFunc = MSP_CMD_UPDATE_ERROR;
        XW_MsgQueue_Send(MSP_CMD_SYSTEMUPGRADE, &updateInfo, 
SEND_MESSAGE_TO_UI);
        XW_MsgQueue_Receive(SEND_MESSAGE_TO_SLAVE);
        _ERROR("System upgrade Error ...");
    }

    //-----------------------------Update Logo-------------------------------
----//
    sprintf(fullName,"/tmp/%s",UpdateLogoFileName);
    if(hasUpdateFile == HI_TRUE )
    {
        if(access(fullName, F_OK) == 0)
        {
            hasLogoFile = HI_TRUE;
            updateInfo.ctrlModel = MSP_CMD_SYSTEMUPGRADE;
            updateInfo.cmdFunc = MSP_CMD_UPDATE_LOGO;
            XW_MsgQueue_Send(MSP_CMD_SYSTEMUPGRADE, &updateInfo, 
SEND_MESSAGE_TO_UI);
            XW_MsgQueue_Receive(SEND_MESSAGE_TO_SLAVE);
            _DEBUG("Logo file access: %s",fullName);
        }
        else
        {
            hasLogoFile = HI_FALSE;
            _DEBUG("logo file: %s not exist !",fullName);
        }
    }
    else
        hasLogoFile = HI_FALSE;
  
    if(hasUpdateFile == HI_TRUE && hasLogoFile == HI_TRUE)
    {        
        sleep(2);
        res = XW_Update_WriteFlash(UPDATE_LOGO, fullName);
        if(res == HI_TRUE)
        {
            updateInfo.ctrlModel = MSP_CMD_SYSTEMUPGRADE;
            updateInfo.cmdFunc = MSP_CMD_UPDATE_LOGOOK;
            XW_MsgQueue_Send(MSP_CMD_SYSTEMUPGRADE, &updateInfo, 
SEND_MESSAGE_TO_UI);
            XW_MsgQueue_Receive(SEND_MESSAGE_TO_SLAVE);
        }
        else
        {
            updateInfo.ctrlModel = MSP_CMD_SYSTEMUPGRADE;
            updateInfo.cmdFunc = MSP_CMD_UPDATE_LOGOERR;
            XW_MsgQueue_Send(MSP_CMD_SYSTEMUPGRADE, &updateInfo, 
SEND_MESSAGE_TO_UI);
            XW_MsgQueue_Receive(SEND_MESSAGE_TO_SLAVE);
        }
       
        // update success, delete update files 
        if(access(fullName, F_OK) == 0)
        {
            remove(fullName);
            _DEBUG("Delete logo file : %s",fullName);
        }
    } 

    if((hasUpdateFile == HI_TRUE && Update_Flags == HI_FALSE) || hasLogoFile 
== HI_TRUE)
    {
        sleep(3);
        system("reboot");
    }
    update_state = false;
    //XW_Update_DelUpdateFile();
    
    return NULL;
}

/******************************************************************************
*****************************
**函数:
**输入参数:
**功能:
**返回值:
*******************************************************************************
****************************/

void *XW_Update_pthread(void *argv)
{
    char fullName[80];
    bool ret, res, hasUpdateFile = false;
    UPDATEINFO_T updateInfo;

    DVR_PTHREAD_DETACH;

    ret = XW_Update_findUpdateFile(UPDATE_BOOT, fullName);
    if(ret == HI_TRUE)
    {
        updateInfo.doType = UPDATE_BOOT;
        updateInfo.res = UPDATE_DOING;
        //XW_MsgQueue_Send(MSP_CMD_SYSTEMUPGRADE, (U8_T *)&updateInfo, 
SEND_MESSAGE_TO_UI);
        sleep(2);
        res = XW_Update_WriteFlash(UPDATE_BOOT, fullName);
        if(res == HI_TRUE)
        {
            updateInfo.res = UPDATE_SUCCESS;
        }
        else
        {
            updateInfo.res = UPDATE_ERROR;
        }
        //XW_MsgQueue_Send(MSP_CMD_SYSTEMUPGRADE, (U8_T *)&updateInfo,
SEND_MESSAGE_TO_UI);
        hasUpdateFile = true;
        sleep(1);
    }

    ret = XW_Update_findUpdateFile(UPDATE_KERNEL, fullName);
    if(ret == HI_TRUE)
    {
        updateInfo.doType = UPDATE_KERNEL;
        updateInfo.res = UPDATE_DOING;
        //XW_MsgQueue_Send(MSP_CMD_SYSTEMUPGRADE, (U8_T *)&updateInfo, 
SEND_MESSAGE_TO_UI);
        sleep(2);
        res = XW_Update_WriteFlash(UPDATE_KERNEL, fullName);
        if(res == HI_TRUE)
        {
            updateInfo.res = UPDATE_SUCCESS;
        }
        else
        {
            updateInfo.res = UPDATE_ERROR;
        }
        //XW_MsgQueue_Send(MSP_CMD_SYSTEMUPGRADE, (U8_T *)&updateInfo, 
SEND_MESSAGE_TO_UI);
        hasUpdateFile = true;
        sleep(1);
    }

    ret = XW_Update_findUpdateFile(UPDATE_ROOTFS, fullName);
    if(ret == HI_TRUE)
    {
        updateInfo.doType = UPDATE_ROOTFS;
        updateInfo.res = UPDATE_DOING;
        //XW_MsgQueue_Send(MSP_CMD_SYSTEMUPGRADE, (U8_T *)&updateInfo, 
SEND_MESSAGE_TO_UI);
        sleep(2);
        res = XW_Update_WriteFlash(UPDATE_ROOTFS, fullName);
        if(res == HI_TRUE)
        {
            updateInfo.res = UPDATE_SUCCESS;
        }
        else
        {
            updateInfo.res = UPDATE_ERROR;
        }
        //XW_MsgQueue_Send(MSP_CMD_SYSTEMUPGRADE, (U8_T *)&updateInfo, 
SEND_MESSAGE_TO_UI);
        hasUpdateFile = true;
        sleep(1);
    }

    ret = XW_Update_findUpdateFile(UPDATE_MODULES, fullName);
    if(ret == HI_TRUE)
    {
        updateInfo.doType = UPDATE_MODULES;
        updateInfo.res = UPDATE_DOING;
        //XW_MsgQueue_Send(MSP_CMD_SYSTEMUPGRADE, (U8_T *)&updateInfo, 
SEND_MESSAGE_TO_UI);
        sleep(2);
        res = XW_Update_WriteFlash(UPDATE_MODULES, fullName);
        if(res == HI_TRUE)
        {
            updateInfo.res = UPDATE_SUCCESS;
        }
        else
        {
            updateInfo.res = UPDATE_ERROR;
        }
        //XW_MsgQueue_Send(MSP_CMD_SYSTEMUPGRADE, (U8_T *)&updateInfo, 
SEND_MESSAGE_TO_UI);
        hasUpdateFile = true;
        sleep(1);
    }

    ret = XW_Update_findUpdateFile(UPDATE_WEBS, fullName);
    if(ret == HI_TRUE)
    {
        updateInfo.doType = UPDATE_WEBS;
        updateInfo.res = UPDATE_DOING;
        //XW_MsgQueue_Send(MSP_CMD_SYSTEMUPGRADE, (U8_T *)&updateInfo, 
SEND_MESSAGE_TO_UI);
        sleep(2);
        res = XW_Update_WriteFlash(UPDATE_WEBS, fullName);
        if(res == HI_TRUE)
        {
            updateInfo.res = UPDATE_SUCCESS;
        }
        else
        {
            updateInfo.res = UPDATE_ERROR;
        }
        //XW_MsgQueue_Send(MSP_CMD_SYSTEMUPGRADE, (U8_T *)&updateInfo, 
SEND_MESSAGE_TO_UI);
        hasUpdateFile = true;
        sleep(1);
    }

    ret = XW_Update_findUpdateFile(UPDATE_PICTURE, fullName);
    if(ret == HI_TRUE)
    {
        updateInfo.doType = UPDATE_PICTURE;
        updateInfo.res = UPDATE_DOING;
        //XW_MsgQueue_Send(MSP_CMD_SYSTEMUPGRADE, (U8_T *)&updateInfo, 
SEND_MESSAGE_TO_UI);
        sleep(2);
        res = XW_Update_WriteFlash(UPDATE_PICTURE, fullName);
        if(res == HI_TRUE)
        {
            updateInfo.res = UPDATE_SUCCESS;
        }
        else
        {
            updateInfo.res = UPDATE_ERROR;
        }
        //XW_MsgQueue_Send(MSP_CMD_SYSTEMUPGRADE, (U8_T *)&updateInfo, 
SEND_MESSAGE_TO_UI);
        hasUpdateFile = true;
        sleep(1);
    }

    ret = XW_Update_findUpdateFile(UPDATE_APP, fullName);
    if(ret == HI_TRUE)
    {
        updateInfo.doType = UPDATE_APP;
        updateInfo.res = UPDATE_DOING;
        //XW_MsgQueue_Send(MSP_CMD_SYSTEMUPGRADE, (U8_T *)&updateInfo, 
SEND_MESSAGE_TO_UI);
        sleep(2);
        //res = XW_Update_WriteFlash(UPDATE_APP, fullName);
        //res = XW_UpdateApplication(fullName);
        if(res == HI_TRUE)
        {
            updateInfo.res = UPDATE_SUCCESS;
        }
        else
        {
            updateInfo.res = UPDATE_ERROR;
        }
        //XW_MsgQueue_Send(MSP_CMD_SYSTEMUPGRADE, (U8_T *)&updateInfo, 
SEND_MESSAGE_TO_UI);
        hasUpdateFile = true;
        sleep(1);
    }

    ret = XW_Update_findUpdateFile(UPDATE_LOGO, fullName);
    if(ret == HI_TRUE)
    {
        updateInfo.doType = UPDATE_LOGO;
        updateInfo.res = UPDATE_DOING;
        //XW_MsgQueue_Send(MSP_CMD_SYSTEMUPGRADE, (U8_T *)&updateInfo, 
SEND_MESSAGE_TO_UI);
        sleep(2);
        res = XW_Update_WriteFlash(UPDATE_LOGO, fullName);
        if(res == HI_TRUE)
        {
            updateInfo.res = UPDATE_SUCCESS;
        }
        else
        {
            updateInfo.res = UPDATE_ERROR;
        }
        //XW_MsgQueue_Send(MSP_CMD_SYSTEMUPGRADE, (U8_T *)&updateInfo, 
SEND_MESSAGE_TO_UI);
        hasUpdateFile = true;
        sleep(1);
    }    

    if(hasUpdateFile == false)
    {
        updateInfo.doType = UPDATE_NOFINDFINE;
        updateInfo.res = UPDATE_DOING;
        //XW_MsgQueue_Send(MSP_CMD_SYSTEMUPGRADE, (U8_T *)&updateInfo, 
SEND_MESSAGE_TO_UI);

    }
    else
    {
        updateInfo.doType = UPDATE_OVER;
        updateInfo.res = UPDATE_DOING;
        //XW_MsgQueue_Send(MSP_CMD_SYSTEMUPGRADE, (U8_T *)&updateInfo, 
SEND_MESSAGE_TO_UI);
    }
    update_state = false;
    XW_Update_DelUpdateFile();
    return NULL;
}
#endif


/******************************************************************************
*****************************
**函数:
**输入参数:
**功能:
**返回值:
*******************************************************************************
****************************/
bool XW_Update_GetPthreadUpdateState(void)
{
    return update_state;
}

struct mtd_info_user mtd;
/******************************************************************************
*****************************
**函数:
**输入参数:
**功能:
**返回值:
*******************************************************************************
****************************/
int XW_Update_safeOpen(char *pathName, int flags)
{
    int fd;
    fd = open(pathName, flags);
    if(fd < 0)
    {
        fprintf(stdout, "Open '%s' = %d\n", pathName, fd);
    }
    return (fd);
}
/******************************************************************************
*****************************
**函数:
**输入参数:
**功能:
**返回值:
*******************************************************************************
****************************/

int XW_Update_safeClose(int fd)
{
    close(fd);
    return 0;
}
/******************************************************************************
*****************************
**函数:
**输入参数:
**功能:
**返回值:
*******************************************************************************
****************************/

int XW_Update_safeRead(int fd, void *buf, size_t count, int val)
{
    size_t result;
    result = read(fd, buf, count);
    if(count != result)
    {
        fprintf(stdout, "read error! required count is %d, result is %d\n", (
int)count, (int)result);
    }
    return result;
}
/******************************************************************************
*****************************
**函数:
**输入参数:
**功能:
**返回值:
*******************************************************************************
****************************/

int XW_Update_safeWrite(int fd, void *buf, size_t count, int val)
{
    size_t result;
    result = write(fd, buf, count);
    if(count != result)
    {
        fprintf(stdout, "write error! required count is %d, result is %d\n", (
int)count, (int)result);
    }
    return result;
}
/******************************************************************************
*****************************
**函数:
**输入参数:
**功能:
**返回值:
*******************************************************************************
****************************/

int XW_Update_safeSeek(int fd, size_t addr)
{
    ssize_t result;
    result = lseek(fd, addr, SEEK_SET);
    if(result < 0)
    {
        fprintf(stdout, "Rewind file error!\n");
    }
    return result;
}
/******************************************************************************
*****************************
**函数:
**输入参数:
**功能:
**返回值:
*******************************************************************************
****************************/

int XW_Update_safeGetLength(char *fName)
{
    struct stat fileStat;
    int fileFd = -1;

    /* open and get some info about the file */
    fileFd = XW_Update_safeOpen(fName, O_SYNC | O_RDONLY);
    if(fileFd <= 0)
    {
        return -1;
    }
    if(fstat(fileFd, &fileStat) < 0)
    {
        fprintf(stdout, "Can not get file '%s' infomation !\n", fName);
        XW_Update_safeClose(fileFd);
        return -2;
    }
    else
    {
        //fprintf(stdout, "Size of '%s' : %d\n", fName, (int)fileStat.st_size);
    }
    XW_Update_safeClose(fileFd);

    return fileStat.st_size;

}
/******************************************************************************
*****************************
**函数:
**输入参数:
**功能:
**返回值:
*******************************************************************************
****************************/

int XW_Update_getMtdInfo(char *device)
{
    int devFd = -1;

    /* get some info about the flash device */
    devFd = XW_Update_safeOpen(device, O_SYNC | O_RDWR);
    if(devFd <= 0)
    {
        return -1;
    }
    if(ioctl(devFd, MEMGETINFO, &mtd) < 0)
    {
        fprintf(stdout, "Can not get MTD infomation!\n");
        XW_Update_safeClose(devFd);
        return -2;
    }
    XW_Update_safeClose(devFd);
    return 0;

}

/******************************************************************************
*****************************
**函数:
**输入参数:
**功能:
**返回值:
*******************************************************************************
****************************/

int XW_Update_earseMtd(char *device, size_t fsize, int sectionId)
{
    struct mtd_info_user mtd;
    struct erase_info_user erase;

    int devFd = -1, i, blocks;

    /* get some info about the flash device */
    devFd = XW_Update_safeOpen(device, O_SYNC | O_RDWR);
    if(devFd <= 0)
    {
        return -1;
    }
    if(ioctl(devFd, MEMGETINFO, &mtd) < 0)
    {
        fprintf(stdout, "Can not get MTD infomation!\n");
        XW_Update_safeClose(devFd);
        return -2;
    }

    /* calc all the max length of the operation */
    if(fsize <= 0)
    {
        erase.length = mtd.size;
    }
    else
    {
        erase.length = fsize & ~(mtd.erasesize - 1);
        if(fsize % mtd.erasesize) erase.length  += mtd.erasesize;
        if(erase.length > mtd.size) erase.length = mtd.size;
    }

    /* erase 1 block at a time and show what's going on */
    blocks = erase.length / mtd.erasesize;
    erase.length = mtd.erasesize;
    erase.start = 0;

    for(i = 1; i <= blocks; i++)
    {
        // printf("Erasing blocks: %d/%d", i, blocks);
        if(ioctl(devFd, MEMERASE, &erase) < 0)
        {
            printf("Error while erasing blocks %d!\n", blocks);
            XW_Update_safeClose(devFd);
            return -3;
        }
        erase.start += mtd.erasesize;
    }

    _DEBUG("Erasing finished!\n");

    XW_Update_safeClose(devFd);

    return 0;

}

/******************************************************************************
*****************************
**函数:
**输入参数:
**功能:
**返回值:
*******************************************************************************
****************************/

int XW_Update_writeImageToMTD(char *device, char *fileName, int sectionId)
{
    int transforSize = 4 * 1024;
    int XW_Tools_fileLen, remainLen, readLen, tryCount;
    unsigned long fileAddr, tmocs;
    unsigned char tmpBuffer[transforSize+16];
    unsigned char verifyBuffer[transforSize+16];
    int devFd = -1, fileFd = -1;
    int verifyLen = 0;
    int pos = 0;

    XW_Tools_fileLen = XW_Update_safeGetLength(fileName);
    fileAddr = 0;
    if(XW_Update_getMtdInfo(device) != 0)
    {
        _ERROR("Get MTD info failed !");
        return -1;
    }
    if(XW_Tools_fileLen > (int)mtd.size)
    {
        _ERROR("file length is larger than mtd size!\n");
        return -1;
    }

    if(strcmp(fileName, UpdateBootFileName))
    {
        if(XW_Update_earseMtd(device, mtd.size - mtd.erasesize, sectionId) != 
0)
        {
            _ERROR("earse Mtd failed\n");
            return -1;
        }
    }
    else
    {
        if(XW_Update_earseMtd(device, mtd.size, sectionId) != 0)
        {
            _ERROR("earse Mtd failed\n");
            return -1;
        }
    }

    fileFd = XW_Update_safeOpen(fileName, O_SYNC | O_RDONLY);
    if(fileFd <= 0)
    {
        return -1;
    }
    /* open flash device */
    devFd = XW_Update_safeOpen(device, O_SYNC | O_RDWR);
    if(devFd <= 0)
    {
        return -1;
    }
    remainLen = XW_Tools_fileLen;
    tmocs    = 3000;
    tryCount = 0;

    /*begin write flash*/
    while(remainLen > 0)
    {
        if(tryCount > 4)
        {
            _ERROR("device %s write error\n", device);
            XW_Update_safeClose(devFd);
            XW_Update_safeClose(fileFd);
            return -1;
        }

        readLen = (remainLen > transforSize) ? transforSize : remainLen;

        XW_Update_safeSeek(fileFd, fileAddr);

        readLen = XW_Update_safeRead(fileFd, tmpBuffer, readLen , tmocs);
        if(readLen <= 0)
        {
            _ERROR("File %s read error\n", fileName);
            XW_Update_safeClose(devFd);
            XW_Update_safeClose(fileFd);
            return -1;
        }

        pos = lseek(devFd, 0, SEEK_CUR);
        readLen = XW_Update_safeWrite(devFd, tmpBuffer, readLen , tmocs);
        if(readLen < 0)
        {
            tmocs = 5000;
            tryCount++;
            continue;
        }
        else
        {
            //verify
            lseek(devFd, (0 - readLen), SEEK_CUR);
            verifyLen = XW_Update_safeRead(devFd, verifyBuffer, readLen, tmocs
);
            if(verifyLen != readLen)
            {
                tryCount++;
                XW_Update_safeSeek(devFd, pos);
                _ERROR("read device %s failed\n", device);
                continue;
            }

            if(0 != memcmp(tmpBuffer, verifyBuffer, readLen))
            {
                tryCount++;
                XW_Update_safeSeek(devFd, pos);
                _ERROR("verify data failed on device %s\n", device);
                continue;
            }

            tryCount    =  0;
            fileAddr   += readLen;
            remainLen    -= readLen;
            tmocs    = 3000;
            // printf("write size is %d\n",readLen);
        }
    };

    if(strcmp(fileName, UpdateLogoFileName))
    {
        _DEBUG("set env jpeg size");
        //setUbootEnv("jpeg_size", XW_Tools_fileLen);
    }
    return 0;
}



