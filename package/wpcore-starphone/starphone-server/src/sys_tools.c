/***************************************************************************************************
**�ļ�:
**��д��:http://sunshengquan.taobao.com/
**��д����:2012��01��02��
**��Ҫ����:
**�޸���:
**�޸�����:
**:ע:��ȨΪ�������������У���Ҳ������κ���ʽת�أ��������Ϸ�����
����ת�͸����ˣ�����ת�������ˣ�ֻ����˾�ڲ���Ա������ʹ��
�������������й���Ĵ���ֻ��ʹ��Ȩ�������δ�������Ϲ�������Դ����й��
��������⳥������������(http://sunshengquan.taobao.com)50������ҵ��⳥����.
****************************************************************************************************/

#include <stdio.h>
#include "include/xw_export.h"


/***********************************************************************************************************
**����:
**�������:
**����:
**����ֵ:
***********************************************************************************************************/
char XW_Tools_GetTtyInputChar(void)
{
	/*����ն�������ַ�*/

    int n = 1;
    unsigned char ch;
    struct timeval tv;
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(0, &rfds);
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    n = select(1, &rfds, NULL, NULL, &tv);
    if(n > 0)
    {
        n = read(0, &ch, 1);
        if(n == 1)
            return ch;
        return n;
    }
    return -1;
}

/***********************************************************************************************************
**����:
**�������:
**����:
**����ֵ:
***********************************************************************************************************/
bool XW_Tools_DisAllOpenedFile(int pid)
{
	/*�ر�ָ���������д򿪵��ļ�*/

    int ret;
#define   NAME_MAX_DVR 256
#define   PATH_MAX_DVR  NAME_MAX_DVR
    char fdDirPath[PATH_MAX_DVR];
    sprintf(fdDirPath, "ls -l /proc/%d/fd", pid);
    ret = system(fdDirPath);
    if(ret < 0)
    {
        _ERROR("lsAllOpenedSocketFd");
    }
    return true;
}

/***********************************************************************************************************
**����:
**�������:
**����:
**����ֵ:
***********************************************************************************************************/
int	XW_Tools_DvrCpFile(char *srcFile, char *targetFile, bool findString, char *string)
{
    int ret = 0;
    FILE *srcfp = NULL;
    FILE *targetfp = NULL;
    int length = 0;
    ssize_t size;
    size_t len = 0;
    char *p = NULL;
    CHAR_T   *line = NULL;

    if(access(srcFile, F_OK) != 0)
    {
        _ERROR("ERROR: src file is not exist!");
        return -1;
    }
    if(ret == 0)
    {
        srcfp = fopen(srcFile, "r");
        targetfp = fopen(targetFile, "w+");
        if((targetfp != NULL) && (srcfp != NULL))
        {
            while((size = getline(&line, &len, srcfp)) != -1)
            {
                if(size > 0)
                {
                    if(findString == true)
                    {
                        p = strstr(line, string);
                        if(p != NULL)
                        {
                            length = fwrite(line, size, 1, targetfp);
                            //_DEBUG("/proc/partions: %s",line);
                        }
                    }
                    else
                    {
                        length = fwrite(line, size, 1, targetfp);
                    }
                }
            }
            fclose(srcfp);
            fclose(targetfp);
            if(line)
            {
                free(line);
            }
            ret = 0;
        }
        else
        {
            if(srcfp)
            {
                _ERROR("ERROR: open flie %s", targetFile);
                fclose(srcfp);
            }
            if(targetfp)
            {
                _ERROR("ERROR: open flie %s", srcFile);
                fclose(targetfp);
            }
            ret = -1;
        }
    }
    return ret;
}
/***********************************************************************************************************
**����:
**�������:
**����:
**����ֵ:
***********************************************************************************************************/
int XW_Tools_SetProcessPriority(void)
{
/*�����߳����ȼ�*/
    struct sched_param param;
    {
        sched_getparam(0, &param);
        param.sched_priority = 20;
        if(sched_setscheduler(0, SCHED_RR, &param) == -1)
        {
            perror("sched_setscheduler");
        }
        sched_getparam(0, &param);
    }
    return 0;
}


/***********************************************************************************************************
**����:
**�������:
**����:
**����ֵ:
***********************************************************************************************************/
I32_T XW_Tools_getpidbyname(const CHAR_T *name, int pos)
{
	/*ͨ����������ý��̵�PID*/

    CHAR_T buf[80];
    I32_T pid = -1;
    ssize_t size;
    size_t len = 0;
    CHAR_T	 *line = NULL;
    char *pstr = NULL;
    bool find = false;
    FILE *fp;
#define filename "/var/getpidname"

    assert(name);

    fp = fopen(filename, "w+");
    sprintf(buf, "ps |grep %s > %s", name, filename);
    if(-1 == system(buf))
    {
        fclose(fp);
        perror("system ERROR");
        return -1;
    }
    fseek(fp, 0, SEEK_SET);
    while((size = getline(&line, &len, fp)) != -1)
    {
        pstr = strstr(line, "root");
        if(pstr != NULL)
        {
            find = true;
            break;
        }
    }
    if(find)
    {
        bzero(buf, sizeof(buf));
        memcpy(buf, line, pstr - line - 1);
        pid = atoi(buf);
    }

    if(line)
    {
        free(line);
    }
    fclose(fp);
    remove(filename);
    return pid;
}

/***********************************************************************************************************
**��    ��:
**��    ��:
**�������:
**����  ֵ:
***********************************************************************************************************/
int XW_Tools_dirIsNull(char *dir_path)
{
	/*�ж�Ŀ¼�Ƿ�Ϊ��*/

    int has_file = 0;

    DIR   *dir;
    char   currfile[100];
    struct   dirent   *s_dir;
    struct   stat   stat_buf;
    dir = opendir(dir_path);
    if(dir == NULL )
    {
        return 0;
    }

    while((s_dir = readdir(dir)) != NULL)
    {
        if((strcmp(s_dir->d_name, ".") == 0) || (strcmp(s_dir->d_name, "..") == 0))
            continue;
        sprintf(currfile, "%s/%s", dir_path, s_dir->d_name);
        stat(currfile, &stat_buf);
        if(S_ISDIR(stat_buf.st_mode))
        {
            has_file = 1;
            break;
        }
        else
        {
            has_file = 1;
            break;
        }
    }
    closedir(dir);
    return has_file;
}
/***********************************************************************************************************
**����:
**�������:
**����:
**����ֵ:
***********************************************************************************************************/
U8_T XW_Tools_DayofTheWeek(void)
{
	/*���������һ�ܵĵڼ���*/

    struct tm *tmv;
    time_t tv;
    tv = time(NULL);
    tmv = localtime(&tv);
    switch(tmv->tm_wday)
    {
    case 0:
        return 6;
        break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
        return tmv->tm_wday - 1;
    }
    return 0;
}
/***********************************************************************************************************
    **����:
    **�������:
    **����:
    **����ֵ:
    ***********************************************************************************************************/
time_t  XW_Tools_getCurrentTime(void)
{
    time_t  p_time;
    time(&p_time);
    return p_time;
}
/***********************************************************************************************************
**����:
**�������:
**����:
**����ֵ:
***********************************************************************************************************/
void kill_uiprocess(void)
{
    CHAR_T buffer[32];
    I32_T ret;

    ret = XW_Tools_getpidbyname("DvrUi2", 0);
    if(ret > 0)
    {
        sprintf(buffer, "kill %d", ret);
        system(buffer);
    }
}
/***********************************************************************************************************
**����:
**�������:
**����:
**����ֵ:
***********************************************************************************************************/
void Lock(bool block_init, pthread_mutex_t *lock)
{
    if(!block_init)
    {
        if(pthread_mutex_init(lock, NULL) != 0)
        {
            _ERROR("Lock Mutex initialization failed!\n");
            return;
        }
        block_init = 1;
    }
    pthread_mutex_lock(lock);
}
/***********************************************************************************************************
**����:
**�������:
**����:
**����ֵ:
***********************************************************************************************************/
void UnLock(pthread_mutex_t *lock)
{
    pthread_mutex_unlock(lock);
}

/*********************************************************************************************************/
U32_T XW_Tools_fileLen(char *fileName)
{
	/*�ļ��Ĵ�С*/

    U32_T ft;
    FILE *fp;
    fp = fopen(fileName, "r");
    if(fp == NULL)
    {
        _ERROR("open filename %s is error", fileName);
        return 0;
    }
    fseek(fp, 0, SEEK_END);
    ft = ftell(fp);
    fclose(fp);
    return ft;
}

void XW_Tools_delayMs(int ms)
{
    usleep(1000 * ms);
}

int mymax(int a, int b)
{
    if( a > b) return a;
    else return b;
}

