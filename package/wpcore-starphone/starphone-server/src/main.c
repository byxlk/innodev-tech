#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "include/xw_export.h"

static SPS_SYSTEM_INFO_T gSystemInfo ;

SPS_SYSTEM_INFO_T  *XW_Global_InitSystemInfo(void)
{
    return &gSystemInfo;
}

int main(int argc, char **argv) 
{	
    char key;
    _DEBUG("Compile Time:%s",__TIME__);
    _DEBUG("Perper memory init ...");
    
    //启动各种系统应用
    memset(&gSystemInfo, 0x0, sizeof(SPS_SYSTEM_INFO_T));
    XW_SysModuleBoot();

	//等待终端退出指令
    _DEBUG("system boot complete ,please char  q toexit system !");
    while(1)
    {
        key = XW_Tools_GetTtyInputChar();/*获得终端输入的字符*/
        /*当设置自动重启时间到或关闭系统，用户输入q或者Q时，退出系统。*/
        if( key == 'q' || key == 'Q' )
        {
            _DEBUG("recv exit key");
            break;
        }
	}
     _DEBUG("system maintenance is exit");
   
	//退出病关闭系统的各种应用
    XW_SysModuleUnBoot();    
	
    _DEBUG("system exit !");

    return 0;
}