#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "include/xw_export.h"


int main(int argc, char **argv) 
{	
    char key;
    _DEBUG("Compile Time:%s",__TIME__);
    _DEBUG("Perper memory init ...");
    
    //��������ϵͳӦ��
    XW_SysModuleBoot();

	//�ȴ��ն��˳�ָ��
	_DEBUG("system boot complete ,please char  q toexit system !");
    while(1)
    {
        key = XW_Tools_GetTtyInputChar();/*����ն�������ַ�*/
        /*�������Զ�����ʱ�䵽��ر�ϵͳ���û�����q����Qʱ���˳�ϵͳ��*/
        if( key == 'q' || key == 'Q' )
        {
            _DEBUG("recv exit key");
            break;
        }
	}
     _DEBUG("system maintenance is exit");
   
	//�˳����ر�ϵͳ�ĸ���Ӧ��
    XW_SysModuleUnBoot();    
	
    _DEBUG("system exit !");

    return 0;
}