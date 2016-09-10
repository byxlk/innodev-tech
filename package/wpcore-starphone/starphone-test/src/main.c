/* tinyplay.c
**
** Copyright 2011, The Android Open Source Project
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**     * Redistributions of source code must retain the above copyright
**       notice, this list of conditions and the following disclaimer.
**     * Redistributions in binary form must reproduce the above copyright
**       notice, this list of conditions and the following disclaimer in the
**       documentation and/or other materials provided with the distribution.
**     * Neither the name of The Android Open Source Project nor the names of
**       its contributors may be used to endorse or promote products derived
**       from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY The Android Open Source Project ``AS IS'' AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL The Android Open Source Project BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
** SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
** CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
** OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
** DAMAGE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include<ctype.h>
#include <malloc.h>

#include <termios.h> /* for tcxxxattr, ECHO, etc */
#include <unistd.h> /* for STDIN_FILENO */


#include "pcm.h"
#include "si3050_api.h"
#include "common.h"


/**************************************************************************** 
getch()：Linux下的模拟实现
* 在windows下可以通过#include <conio.h>使用getch()，但是conio.h并不是一个标准的头文件，
* conio也不是标准的c库。所以如果在Linux下的c程序中#include <conio.h>，编程就会报错： No 
* Such file or directory!
* 那么如果想在Linux下使用与getch() 功能相同的函数，怎么办呢？我们可以通过以下的程序模拟实现
* 
* getch()。
****************************************************************************/

/*simulate windows' getch(), it works!!*/
int getch (void)
{
    int ch;
    struct termios oldt, newt;

    // get terminal input's attribute
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    //set termios' local mode
    newt.c_lflag &= ~(ECHO|ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    //read character from terminal input
    ch = getchar();

    //recover terminal's attribute
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    return ch;
}



int main(int argc, char **argv)
{    
    unsigned char menuFlag = 0;
    unsigned char iMenuChoice;
    unsigned char pcm_data_buf[128];
    starphone_server *sps;

    
    _DEBUG("Si3050 voip test system, Start ...");
    si3050_hw_reset();

    _DEBUG("Perper memory init ...");
    sps = (starphone_server *) malloc(sizeof(starphone_server));
    if(NULL ==  sps)
    {
            _ERROR("malloc starphone_server struct faild.");
            exit(-1);
    }

    _DEBUG("init pcm");
    sps->pcm_dat_buff = pcm_data_buf;
    si3050_pcm_dev_drv_init(sps);
    si3050_get_ver_info();
    //si3050_sw_reset(sps);
    
    // Start Show Test Menu and Process task events    
    iMenuChoice = 10;//enter key
    while(iMenuChoice != 'q' && iMenuChoice != 'Q')
    {
        
        //system("clear");
        if(menuFlag == 0)
        {
            printf("  *********** Voip WiFi Phone Auto Test sysytem ***********\n\n");
            printf("  a. Get Si3050 version informations \n");
            printf("  b. Reset si3050 by reset pin \n");
            printf("  c. ReInit PCM driver \n");
            printf("  d. Resered \n");
            printf("  e. Resered\n");
            printf("  f. Resered\n");
            printf("  q. Quit\n\n");
            printf("  Please input your Chioce(0-6): ");
        }
        iMenuChoice = getch();
        if((iMenuChoice > 47 && iMenuChoice < 90) || 
                (iMenuChoice > 64 && iMenuChoice < 91) || 
                (iMenuChoice > 96 && iMenuChoice < 123))
        {
                printf("\n  You selected Title Number: %c \n\n",iMenuChoice);
                menuFlag = 0;
        }
        else
        {
                menuFlag = 1;
                continue;        
        }
        
        switch (iMenuChoice)
        {
             case 'a': 
             {                  
                 si3050_get_ver_info();
                 break;
             }
            case 'b': 
            {
                si3050_hw_reset();   
                _DEBUG("hardware reset");
                break;
            }
            case 'c':
            {
                si3050_close_pcm_in(sps);
                si3050_close_pcm_out(sps);
                si3050_pcm_dev_drv_init(sps);
                _DEBUG("pcm init");
                break;
            }
            case 'd': 
            {
                si3050_hw_reset();  
                si3050_get_ver_info();
                break;
            }   
            case 'e': 
            {
                si3050_hw_reset();  
                si3050_close_pcm_in(sps);
                si3050_close_pcm_out(sps);
                si3050_pcm_dev_drv_init(sps);
                si3050_get_ver_info();
                break;
            }
            case 'f': 
            {
                si3050_sw_reset(sps);
                break;
            }
            case 'q': 
            {                  
               printf("Quit system ...\n\n");
                break;
            }
            case 'Q':
                printf("Quit system ...\n\n");
                break;
            default: 
            {
                menuFlag = 1;
                printf("Your select is unknow, please input again...");
                break;
            }
        }
    }

    si3050_close_pcm_in(sps);
    si3050_close_pcm_out(sps);
    free(sps);

    system("clear");
    
    return 0;
}

