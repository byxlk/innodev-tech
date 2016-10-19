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

#include "gpio-spi.h"

#define SAMPLE_RATE 8000
#define CHANNEL 2
#define BUFFER_SIZE (1024)


void Si3050_Get_VersionInfo(void)
{

	{
		sys_ver_val &=0xF;
		_ERROR("SI3050 dev id error %x\n",sys_ver_val);	
	}

    
    /* This is a simple method of verifying if the deivce is alive */
    if(sys_ver_val == 0)
    {
        return ;
    }
        
    return ;
}



int main()
{
    unsigned char sys_ver_val = 0x00;
    char line_device[4][5] = {"UNKN", "3018", "UNKN", "3019"};

    while(1)
    {
        sys_ver_val = gpio_spi_read(SI3050_REG_CHIP_A_REV);    
        if((sys_ver_val>>4) > 3 || sys_ver_val == 0)
        {
            printf("[Error] Read si3050 version faild ...");
            return -1;
        }

        printf("######################################################\n");
        printf("Compile Time:  %s - %s\n",__DATE__ ,__TIME__);
        printf("Check SI3050 revision %u and Line-side device is %s \n",  
          ((sys_ver_val)&0xF), line_device[(sys_ver_val>>4)&0xF]);
        printf("######################################################\n");

        //////////////////////////////////////////////////////////
        printf("\n"
        "[01][Control1]\n"
        "[02][Control2]\n"
        "[03][Control1]\n"
        "[04][Control1]\n"
        "[05][Control1]\n"
        "[06][Control1]\n"
        "[07][Control1]\n"
        "[10][Control1]\n"
        "[11][Control1]\n"
        "[12][Control1]\n"
        "[13][Control1]\n"
        "[14][Control1]\n"
        "[15][Control1]\n"
        "[16][Control1]\n"
        "[17][Control1]\n"
        "[18][Control1]\n"
        "[19][Control1]\n"
        "[20][Control1]\n"
        "[21][Control1]\n"
        "[22][Control1]\n"
        "[23][Control1]\n"
        "[24][Control1]\n"
        "[25][Control1]\n"
        "[26][Control1]\n"
        "[28][Control1]\n"
        "[29][Control1]\n"
        "[30][Control1]\n"
        "[31][Control1]\n"
        "[32][Control1]\n"
        "[33][Control1]\n"
        "[34][Control1]\n"
        "[35][Control1]\n"
        "[36][Control1]\n"
        "[37][Control1]\n"
        "[38][Control1]\n"
        "[39][Control1]\n"
        "[40][Control1]\n"
        "[41][Control1]\n"
        "[42][Control1]\n"
        "[43][Control1]\n"
        "[44][Control1]\n"
        "[45][Control1]\n"
        "[46][Control1]\n"
        "[47][Control1]\n"
        "[48][Control1]\n"
        "[49][Control1]\n"
        "[50][Control1]\n"
        "[51][Control1]\n"
        "[52][Control1]\n"
        "[59][Control1]\n"

        );        
        sleep(10);
    }
    return 0;
}

