#include "include/xw_export.h"

#define PERM S_IRUSR | S_IWUSR | IPC_CREAT | 0666/*��Ϣ���е�����*/
#define PROCESS_MESFILE "/bin/ls"/*��Ϣ���е�����*/
#define MSG_SEND_BUFFER_SIZE 1024*3/*��Ϣ���еķ��ͻ����������С*/
#define MSG_RCV_BUFFER_SIZE 1024*3/*��Ϣ���н��ܻ����������С*/
struct msgtype_process
{
    long mtype;
    char buffer[MSG_RCV_BUFFER_SIZE];
} msgtype_process_t;
struct msgtype_process_small
{
    long mtype;
    char buffer[MSG_SEND_BUFFER_SIZE];
};
static I32_T process_msgid = -1;/*��Ϣ���е�id*/
struct msgtype_process msg_process_receivebuffer;
/******************************************************************************
*****************************
**����:
**�������:
**����:
**����ֵ:
*******************************************************************************
****************************/
I32_T XW_MsgQueue_Create(void)
{
    key_t key;
    int tryTimes = 5;
    bool ret = true;
    if((key = ftok(PROCESS_MESFILE, 'a')) == -1)
    {
        _ERROR("Creat Process Key(%s) Error��%s\a\n", PROCESS_MESFILE,strerror(errno));
        exit(1);
    }
	_DEBUG("ftok %s for Mesage Queue Success, key is 0x%x",PROCESS_MESFILE,key);
	

    while(ret && tryTimes > 1)
    {
        if((process_msgid = msgget(key, PERM)) == -1)//create
        {
            _ERROR("create Process message id is error ! Try the %dth times.",tryTimes);
            sleep(4);
            if((process_msgid = msgget(key,IPC_EXCL | S_IRUSR | S_IWUSR)) == -1)
            {
                tryTimes--;
            }
            else
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
    //msgctl(process_msgid,IPC_RMID,NULL);//Clean all message
	_DEBUG("Create Process message id(%d) success !",process_msgid);
	
    return process_msgid;
}
/******************************************************************************
*****************************
**����:
**�������:
**����:
**����ֵ:
*******************************************************************************
****************************/
void XW_MsgQueue_Send(MspCmdID ctrlModel, MspSendCmd_t *cmdData, long message_type)
{
    if(process_msgid < 0)
    {
        _ERROR("The message queue may or may not have been created. ");
        return ;
    }
	
    struct msgtype_process_small msg;/*��Ϣ���з��ͻ�����*/
    memset(&msg, 0, sizeof(msg));

    msg.mtype = message_type;    
	
    if(NULL != cmdData)
    {
        cmdData->ctrlModel = ctrlModel;
        memcpy(msg.buffer, (char*)cmdData, sizeof(MspSendCmd_t));
    }
	else
	{
		_ERROR("The message data eare is no data. ");
		return;
	}
	
    if(msgsnd(process_msgid, &msg, sizeof(long) + sizeof(MspSendCmd_t), /*IPC_NOWAIT*/0) == -1)
    {
        _ERROR("process message queue send error, message may be full.");
    }
    return ;
}
/******************************************************************************
*****************************
**����:
**�������:
**����:
**����ֵ:
*******************************************************************************
****************************/
//void XW_MsgQueue_SelfSend(MspSendCmd_t  *pcmd, MspCmdID cid, U8_T *cmdData, U32_T cmdDataLength)
//{
//    pcmd->ctrlModel = cid;
//    pcmd->cmdDataLength = cmdDataLength;
//    if(cmdDataLength > 0)
//        memcpy(pcmd->cmdData, cmdData, sizeof(MspSendCmd_t));
//}
/******************************************************************************
*****************************
**����:
**�������:
**����:
**����ֵ:
*******************************************************************************
****************************/
I32_T XW_MsgQueue_Receive(I32_T message_type)
{
    I32_T receive_size;
	/*msgrcv��������:��Ϣ����ID,��Ϣ���н��ջ�����,��������С,
��Ҫ����Ϣ������ƥ�����Ϣ����,���ƺ�����Ϊ*/
    receive_size = msgrcv(process_msgid, &msg_process_receivebuffer, sizeof(struct msgtype_process), message_type, 0);
    if(receive_size < 0)
    {
        usleep(1000);
        return -1;
    }
    return receive_size;
}

/******************************************************************************
*****************************
**����:
**�������:
**����:
**����ֵ:
*******************************************************************************
****************************/
CHAR_T *XW_MsgQueue_Receive_Data(void)
{
    return msg_process_receivebuffer.buffer;
}




