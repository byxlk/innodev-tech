#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <math.h>


#include "include/xw_export.h"



///////////////////////////////////////////////////////////////////
//  PCM INTERFACE
///////////////////////////////////////////////////////////////////

unsigned short buffer[BUFFER_SIZE];


void si3050_generate_sine(int freq, int volume)
{
    int i = 0;
    for (i = 0; i < BUFFER_SIZE / CHANNEL; i++) {
        double x = i * 2 * 3.1415926 / (SAMPLE_RATE / freq);
        
        unsigned short data = (unsigned short) (volume * sin(x) + 32768);
        if (CHANNEL == 1) {
            buffer[i] = data;
        } else {
            buffer[2 * i] = data;
            buffer[2 * i + 1] = data;
        }
    }
}

void si3050_pcm_init_config(struct pcm_config* config)
{
    unsigned int channels = 2;
    unsigned int rate = SAMPLE_RATE;
    //unsigned int bits = 16;
    unsigned int period_size = 128;
    unsigned int period_count = 2;
    
    config->channels = channels;
    config->rate = rate;
    config->period_size = period_size;
    config->period_count = period_count;
    config->format = PCM_FORMAT_S16_LE;
    config->start_threshold = 0;
    config->stop_threshold = 0;
    config->silence_threshold = 0;   
}

struct pcm* si3050_get_pcm_out(void)
{
    unsigned int card = 0;
    unsigned int device = 0;
    
    struct pcm_config config;
    struct pcm *pcm_out;

    si3050_pcm_init_config(&config);
    
    pcm_out = pcm_open(card, device, PCM_OUT, &config);
   	if (!pcm_out || !pcm_is_ready(pcm_out)) {
        printf("Unable to open PCM device %u (%s)\n",
                device, pcm_get_error(pcm_out));
        return NULL;
    }
    
    return pcm_out;
}

void si3050_close_pcm_out(SPS_SYSTEM_INFO_T *sps)
{
    close(sps->si3050_pcm_out->fd);
    free(sps->si3050_pcm_out);
}

struct pcm* si3050_get_pcm_in(void)
{
    unsigned int card = 0;
    unsigned int device = 0;
    
    struct pcm_config config;
    struct pcm *pcm_in;

    si3050_pcm_init_config(&config);
    
    pcm_in = pcm_open(card, device, PCM_IN, &config);
   	if (!pcm_in || !pcm_is_ready(pcm_in)) {
        printf("Unable to open PCM device %u (%s)\n",
                device, pcm_get_error(pcm_in));
        return NULL;
    }
    
    return pcm_in;
}
void si3050_close_pcm_in(SPS_SYSTEM_INFO_T *sps)
{
    close(sps->si3050_pcm_in->fd);
    free(sps->si3050_pcm_in);
}

int si3050_play_sine(void)
{
    struct pcm *pcm_out = si3050_get_pcm_out();
       
    si3050_generate_sine(2000, 10000);
    
    while (1) {
        if (!pcm_write(pcm_out, buffer, BUFFER_SIZE)) {
            //printf("sucess in to out: %d\n", BUFFER_SIZE);	
        } else {
            printf("failed\n");
            break;
        }
    }
    
    return 0;
}

void si3050_pcm_loopback(void)
{
    struct pcm *pcm_out = si3050_get_pcm_out();
    struct pcm *pcm_in = si3050_get_pcm_in();
    unsigned char buff[128];
    
    if (!pcm_out || !pcm_in)
        return;
    
    printf("buffer size:%d %d\n", pcm_get_buffer_size(pcm_out), pcm_get_buffer_size(pcm_in));
    
    while(1) {
        int ret = pcm_read(pcm_in, buff, sizeof(buff)); 
        if (!ret) {
                ret = pcm_write(pcm_out, buff, sizeof(buff));
       		if (!ret) {
                        continue;
       			//printf("sucess in to out: %d\n", sizeof(buff));	
       		} else {
       		        printf("pcm_write return value: %d\n",ret);
       			break;
       		}
       	} else {
       	    printf("pcm_read return value: %d\n",ret);
            break;
        }
    }
    
    printf("exit!\n");
}

///////////////////////////////////////////////////////////////////
// SPI CONTROL
///////////////////////////////////////////////////////////////////
void si3050_get_ver_info(void)
{
        unsigned char sys_ver_val = 0x00;
        unsigned char line_ver_val = 0x00;
        
        _DEBUG("Get si3050 infomation Start ...");        
        
        sys_ver_val = gpio_spi_read(11);        
        //line_ver_val = gpio_spi_read(13);
        
        _DEBUG("\n");
        _DEBUG("Read reg[11] Value = 0x%0x",sys_ver_val);
        _DEBUG("Read reg[13] Value = 0x%0x",line_ver_val);
        
        switch((sys_ver_val & 0xF0) >> 4)
        {
                case 0x01:
                         _DEBUG("Line-Side ID[si3018]: 0x01");
                        break;
                case 0x03:
                        _DEBUG("Line-Side ID[si3019]: 0x03");
                        break;
                case 0x04:
                        _DEBUG("Line-Side ID[si3011]: 0x04");
                        break;
                default:
                        _DEBUG("Line-Side ID[Unknown]: 0x%0x ",(sys_ver_val & 0xF0) >> 4); 
                        break;
        }
        
        _DEBUG("System-Side Revision: 0x%0x",(sys_ver_val & 0x0F));       
        _DEBUG("Line-Side Device Revision: 0x%0x",((line_ver_val & 0x3C) >> 2));
        _DEBUG("\n");

        _DEBUG("REG[2] Default = 0x03, Read Value = 0x%0x", gpio_spi_read(2)); //0x0000_0011
        gpio_spi_write(2, 0xc5);
        _DEBUG("REG[2] Write Value = 0xc5, Read Value = 0x%0x", gpio_spi_read(2)); //0x0000_0011
        gpio_spi_write(2, 0xc3);
}

void si3050_pcm_dev_drv_init(SPS_SYSTEM_INFO_T *sps)
{
    sps->si3050_pcm_out = si3050_get_pcm_out();
    sps->si3050_pcm_in = si3050_get_pcm_in();
    
    if (!sps->si3050_pcm_out || !sps->si3050_pcm_in)
        return;
    
    printf("buffer size:%d %d\n", 
        pcm_get_buffer_size(sps->si3050_pcm_out), 
        pcm_get_buffer_size(sps->si3050_pcm_in));    

}

void si3050_sw_reset(SPS_SYSTEM_INFO_T *sps)
{
    unsigned char regCfg = 0;

    
    //Enable si3050 PCM interface 
    regCfg = gpio_spi_read(33);
    regCfg |= (0x1 << 3) | (0x1 << 5); // Enable PCM & u-Law
    regCfg &= ~(0x1 << 4);
    gpio_spi_write(33, regCfg);

    //Specific county Seting for Taiwan
    regCfg = gpio_spi_read(16);
    regCfg &= ~((0x1 << 0) & (0x1 << 1) & (0x1 << 4) &  (0x1 << 6)); // OHS RZ RT
    gpio_spi_write(16, regCfg);

    regCfg = gpio_spi_read(26);
    regCfg |= (0x1 << 6) | (0x1 << 7); // DCV[1:0] = 11
    regCfg &= ~((0x1 << 1) & (0x1 << 4) & (0X1 << 5));
    gpio_spi_write(26, regCfg);

    regCfg = gpio_spi_read(30);
    regCfg &= ~((0x1 << 0) & (0x1 << 1) & (0x1 << 2) & (0x1 << 3) & (0x1 << 4));
    gpio_spi_write(30, regCfg);

    regCfg = gpio_spi_read(31);
    regCfg &= ~(0x1 << 3); // OHS2 = 0
    gpio_spi_write(31, regCfg);    

}


void si3050_hw_reset(void)
{
        usleep(1000);
        set_reset_pin_low(); // RESET
        usleep(50*1000);
        usleep(50*1000);
        //usleep(50*1000);
        //usleep(50*1000);
        //usleep(50*1000);
        //usleep(50*1000);      
        //sleep(1);
        set_reset_pin_high(); // RESET
        usleep(50*1000);
        usleep(50*1000);
        //usleep(50*1000);
        //usleep(50*1000);
        //usleep(50*1000);
        //usleep(50*1000);
        //usleep(50*1000);
        //usleep(50*1000);
        //usleep(50*1000);
        //usleep(50*1000);
}


void Si3050_Power_Up_Si3019(void)
{
        gpio_spi_write(6, 0x00);
}

/*******************************************************************************
* FUNCTION:     si3050_hw_reset
*
********************************************************************************
* DESCRIPTION:    Power up the device
*
*******************************************************************************/

bool Si3050_Hw_Reset(void)
{
	unsigned char ac_termination_data;
	unsigned char dc_termination_data;
	unsigned char international_control1_data;
	unsigned char daa_control5_data;
	unsigned char loop_cnt = 0;
	unsigned char data;
	unsigned char interrupt_mask;
	//char i,j;


	/* UINT8 intterupt_mask; */
	/* Enable link + power up line side device */
	gpio_spi_write(SI3050_REG_DAA_CONTROL2, 0x00);

	/* Read FTD */
	data=gpio_spi_read(SI3050_REG_LINE_STATUS);//reg12
	while((!(data & 0x40) | data==0xff)
		&& (loop_cnt < SI3050_MAX_FTD_RETRY))
	{
		usleep(2*1000);
		loop_cnt++;
		data=gpio_spi_read(SI3050_REG_LINE_STATUS);
	}
	if(loop_cnt >= SI3050_MAX_FTD_RETRY)
	{
		//j=sprintf(str,"\nFailed to get FTD register sync\n");
		//ser_Write(str,j);
		return 0;
	}


		ac_termination_data = 0x00;
		dc_termination_data = 0xc0;
		international_control1_data = 0x00;
		daa_control5_data = 0x20;


	/* Set AC termination */
	gpio_spi_write(SI3050_REG_AC_TERMIATION_CONTROL, ac_termination_data);//reg30
	/* Set DC termination */
	gpio_spi_write(SI3050_REG_DC_TERMINATION_CONTROL, dc_termination_data);//reg26
	/* Set International Control: set ring impedance, ring detection threshold, on-hook speed , enable/disable iir filter */
	gpio_spi_write(SI3050_REG_INTERNATIONAL_CONTROL1, international_control1_data);//reg16 /* OHS IIRE RZ RT */
	/* Set off-hook speed , enable full scale */
	daa_control5_data |= 0x80 | 0x02;//0x80 | 0x02; /* Move the Pole of the built in filter from 5 Hz to 200 Hz this may affect voice quality */
	gpio_spi_write(SI3050_REG_DAA_CONTROL5, daa_control5_data);//reg31 /* OHS IIRE RZ RT */

	/* Enable Ring Validation */
	/* fmin 10Hz */
	/* fmax 83Hz */
	/* ring confirmation count 150ms */
	/* ring time out 640ms */
	/* ring delay 0ms */
	gpio_spi_write(SI3050_REG_RING_VALIDATION_CONTROL1, 0x16);//reg22 //0x56
	gpio_spi_write(SI3050_REG_RING_VALIDATION_CONTROL2, 0x29);//reg23//0x2b
	gpio_spi_write(SI3050_REG_RING_VALIDATION_CONTROL3, 0x99);//reg24

	/* Set interrupt mask */
	/* interrupt_mask = SI3050_RDT_INT | SI3050_ROV_INT | SI3050_FDT_INT | SI3050_BTD_INT | SI3050_DOD_INT |SI3050_LCSO_INT | SI3050_TGD_INT | SI3050_POL_INT; */
	/* si3050_write_reg(SI3050_REG_INTERRUPT_MASK, interrupt_register, tcid); */
	/* Clear intterupt register */
	/* si3050_write_reg(SI3050_REG_INTERRUPT_SOURCE, 0x00, tcid); */

	//si3050_write_reg(43 ,0x03, tcid);
	//si3050_write_reg(44 ,0x07, tcid);

	interrupt_mask = SI3050_RDT_INT | SI3050_ROV_INT | SI3050_FDT_INT | \
                         SI3050_BTD_INT | SI3050_DOD_INT |SI3050_LCSO_INT | SI3050_TGD_INT | SI3050_POL_INT;
	interrupt_mask=0x80;
	gpio_spi_write(SI3050_REG_INTERRUPT_MASK, interrupt_mask);
	/* Clear intterupt register */
	gpio_spi_write(SI3050_REG_INTERRUPT_SOURCE, 0x00);



	/* Set international control registers */
	gpio_spi_write(SI3050_REG_INTERNATIONAL_CONTROL2, 0x00); //reg17 /* RT2 OPE BTE ROV BTD */
	gpio_spi_write(SI3050_REG_INTERNATIONAL_CONTROL3, 0x00); //reg18 /* RFWE */
	gpio_spi_write(SI3050_REG_INTERNATIONAL_CONTROL4, 0x00); //reg19 /* OVL DOD OPE */

	return 1;
}

/*******************************************************************************
* FUNCTION:     pcm_init
*
********************************************************************************
* DESCRIPTION:    Perform hardware initialization of PCM interface.
*
*******************************************************************************/
void Si3050_Pcm_Init(unsigned short timeslot)
{
    unsigned short  pcm_offset;
    unsigned char   pcm_mode;


    pcm_offset = (timeslot)*8;
    pcm_mode = SI3050_PCM_ENABLE ;//| SI3050_PCM_TRI ;//| 1<<1;//PHCF

    gpio_spi_write(SI3050_REG_PCM_TX_LOW, (pcm_offset & 0x0ff));
    gpio_spi_write(SI3050_REG_PCM_TX_HIGH, (pcm_offset >> 8) & 0x0003);
    gpio_spi_write(SI3050_REG_PCM_RX_LOW, pcm_offset & 0x0ff);
    gpio_spi_write(SI3050_REG_PCM_RX_HIGH, (pcm_offset >> 8) & 0x0003);
    gpio_spi_write(SI3050_REG_PCM_SPI_MODE_SELECT, pcm_mode);//reg33

}



/*------------------------------------------------------------------------------*/
/* FUNCTION:    si3050_hw_init                                                  */
/*------------------------------------------------------------------------------*/
/* DESCRIPTION:    Perform hardware initialization of TID analog interface.     */
/*------------------------------------------------------------------------------*/

bool Si3050_Hw_Init(unsigned short timeslot)
{

	unsigned char device_id;
	//int i,j;

	/* Initialize and power up DAA */
	if(Si3050_Hw_Reset() == HI_FALSE)
	{
		return HI_FALSE;
	}


	device_id = gpio_spi_read(SI3050_REG_CHIP_A_REV);//reg11

	if((device_id>>4) > 3)
	{
		device_id &=0xF;
		//j=sprintf(str,"\nSI3050 dev id error %x on tcid %u\n",device_id,tcid);
		//ser_Write(str,j);
		return HI_FALSE;
	}


	/* This is a simple method of verifying if the deivce is alive */
	if(device_id == 0)
	{
		//j=sprintf(str,"\nDetected SI3050 revision %u and a %s on tcid %u\n", ((device_id)&0xF), line_device[(device_id>>4)&0xF],tcid);
		//ser_Write(str,j);
		return HI_FALSE;
	}

	/* enable PCM and assign timeslot for PCM */

	Si3050_Pcm_Init(timeslot);

	/* Enable intterupt */
	gpio_spi_write(SI3050_REG_CONTROL2, 0x83); //0x87
	//j=sprintf(str,"\nFXO_ProSLIC device %d initialized", tcid);
	//ser_Write(str,j);

	return HI_TRUE;
}




/*******************************************************************************
* FUNCTION:     si3050_set_hook
*
********************************************************************************
* DESCRIPTION:    Toggle the hook switch
*
*******************************************************************************/
void Si3050_Set_Hook(bool si3050_off_hook)
{
    unsigned char data = 0;
    if(si3050_off_hook)
    {
        data = 0x1;
	}
	gpio_spi_write(SI3050_REG_DAA_CONTROL1, data);
}

/*******************************************************************************
* FUNCTION:     si3050_hw_gain_control
*
********************************************************************************
* DESCRIPTION:    Modify the DAA gain - note, not used for normal operation.
*
*******************************************************************************/
void Si3050_Hw_Gain_Control(unsigned char gain_value_high, 
							unsigned char gain_value_low, bool f_is_tx_gain)
{
	if(f_is_tx_gain)
	{
		gpio_spi_write(SI3050_REG_TX_GAIN_CONTROL2, gain_value_high);
		gpio_spi_write(SI3050_REG_TX_GAIN_CONTROL3, gain_value_low);
	}
	else
	{
		gpio_spi_write(SI3050_REG_RX_GAIN_CONTROL2, gain_value_high);//r39
		gpio_spi_write(SI3050_REG_RX_GAIN_CONTROL3, gain_value_low);//r41
		/*
		reg 39
		7:5 Reserved Read returns zero.
		4 RGA2 Receive Gain or Attenuation 2.
		0 = Incrementing the RXG2[3:0] bits results in gaining up the receive path.
		1 = Incrementing the RXG2[3:0] bits results in attenuating the receive path.
		3:0 RXG2[3:0] Receive Gain 2.
		Each bit increment represents 1 dB of gain or attenuation, up to a maximum of +12 dB and
		?5 dB respectively.
		For example:
		RGA2 RXG2[3:0] Result
		X 0000 0 dB gain or attenuation is applied to the receive path.
		0 0001 1 dB gain is applied to the receive path.
		0 :
		0 11xx 12 dB gain is applied to the receive path.
		1 0001 1 dB attenuation is applied to the receive path.
		1 :
		1 1111 15 dB attenuation is applied to the receive path.
		*/

		/*
		reg 41
				7:5 Reserved Read returns zero.
				4 RGA3 Receive Gain or Attenuation 2.
				0 = Incrementing the RXG3[3:0] bits results in gaining up the receive path.
				1 = Incrementing the RXG3[3:0] bits results in attenuating the receive path.
				3:0 RXG3[3:0] Receive Gain 3.
				Each bit increment represents 0.1 dB of gain or attenuation, up to a maximum of 1.5 dB.
				For example:
				RGA3 RXG3[3:0] Result
				X 0000 0 dB gain or attenuation is applied to the receive path.
				0 0001 0.1 dB gain is applied to the receive path.
				0 :
				0 1111 1.5 dB gain is applied to the receive path.
				1 0001 0.1 dB attenuation is applied to the receive path.
				1 :
				1 1111 1.5 dB attenuation is applied to the receive path.
		*/
	}
}


/*******************************************************************************
* FUNCTION:     si3050_set_lowpwr_path
*
********************************************************************************
* DESCRIPTION:  set data path on onhook line monitor state to transfer CID
*
*******************************************************************************/
void Si3050_Set_Lowpwr_Path(void)
{
    gpio_spi_write(SI3050_REG_DAA_CONTROL1, 0x08);
}

/*******************************************************************************
* FUNCTION:     si3050_clear_lowpwr_path
*
********************************************************************************
* DESCRIPTION:  disable low power data path on onhook state
*
*******************************************************************************/
void Si3050_Clear_Lowpwr_Path(void)
{
    gpio_spi_write(SI3050_REG_DAA_CONTROL1, 0x00);
}


void Modem_DialTelNum(char dial_num)
{

        _DEBUG("Dial Number :  %c",dial_num);
        return ;
}

void XW_Si3050_DAA_System_Init(void)
{
    //unsigned char regCfg = 0;

    SPS_SYSTEM_INFO_T *DTSystemInfo = XW_Global_InitSystemInfo();
    //si3050_hw_reset(); //reset for power up 
    _DEBUG("Reset si3050 complete...");

    
    
    // Check the Version to make sure SPI Conmunication is OK
    si3050_get_ver_info();

    
    //Si3050_Power_Up_Si3019();

    return ;
}

void *XW_Pthread_ModemCtrlDeamon(void *args)
{
	char *sock_send_msg = NULL;
        thread_arg *pthread_client = NULL; 
    
        MspSendCmd_t cmdData;	//��Ϣ���д���ṹ	
	STATE_PREVIEW *p;
        PTHREAD_BUF send_buf;
        SPS_SYSTEM_INFO_T *DTSystemInfo = XW_Global_InitSystemInfo();
        
        p = (STATE_PREVIEW *)XW_ManagePthread_GetPthreadState(PTHREAD_MODEM_CTRL_ID, 0);
        p->power = PTHREAD_POWER_ON;
        p->state = ALIVE;
        
        while(p->power == PTHREAD_POWER_ON)
        {
                XW_ManagePthread_ReadSignal(&send_buf, PTHREAD_MODEM_CTRL_ID, HI_TRUE);                
                if(send_buf.start_id != PTHREAD_CLIENT_MANAGE_ID || strlen(send_buf.m_buffer) == 0)
                {
                        if (send_buf.start_id == PTHREAD_MAIN_ID && send_buf.m_signal == EXIT)
                        {      
                                p->state = EXIT;
                                break;
                        }    
                        _DEBUG("start_id = %d, strlen(send_buf.m_buffer) = %d",
                                            send_buf.start_id, strlen(send_buf.m_buffer));
                        sleep(2);
                        continue;
                }

                //TODO:
                pthread_client = (thread_arg *)(send_buf.m_args);
                if(pthread_client == NULL)
                {
                        _ERROR("client[%d] address transfer faild...",send_buf.m_value);
                }
                else
                {
                        _DEBUG("[Client] IP=%s,  Msg=%s", 
                                         inet_ntoa(pthread_client->caddr.sin_addr), send_buf.m_buffer);
                }
                
                //Paser data which is transfer from phone
                if(strncmp(send_buf.m_buffer, "help", 4)==0) 
                {
        		sock_send_msg = "This is help message\n";
        		//_send(send_buf.m_args->connfd, sock_send_msg, strlen(sock_send_msg));
        	}
                else if(strcmp(send_buf.m_buffer, "hangup")==0) 
                {        		
                        if(pthread_client->busy == 1)
                        {
        		        //Modem_Hangup();
        		        sock_send_msg ="ok\n";
         		        pthread_client->busy = 0;
        		        _send(pthread_client->connfd, sock_send_msg, strlen(sock_send_msg));
                        }
                        else
                        {
                                continue;
                        }
        	}
                else if(strncmp(send_buf.m_buffer, "dial:", 5)==0)// dial:12345
                { 
        		if(pthread_client->busy == 1)
                        {
        			sock_send_msg = "busy\n"; //  關閉撥號狀態
        			_send(pthread_client->connfd, sock_send_msg, strlen(sock_send_msg));
        			continue;
        		}
                        pthread_client->busy = 1;

                        //Parse dial number
                        unsigned char i = 0;
                        char *dial_str =&send_buf.m_buffer[5];
                        for(i = 0; i < (strlen(send_buf.m_buffer)-6); i++)
                                Modem_DialTelNum(dial_str[i]);
                        
        		//pthread_t id;
        		//_pthread_create(&id, (void*)off_hook, &arg_hook);
        		//pthread_join(id, NULL);
        		//printf("dial ended, send on_hook\n");
        		//sock_send_msg = "on_hook\n";
        		//_send(pthread_client->connfd, sock_send_msg, strlen(sock_send_msg));
                        //pthread_client->busy = 0;
        	}
        	else if(strncmp(send_buf.m_buffer, "list", 4)==0)
                {
        		char buf2[MAX+1][100];
        		strcpy(buf2[0],  "Client list:\n");
        		//j=1;
        		//for(i=0;i<MAX;i++) {
        		//	if(send_buf.m_args->connfd != 0) {
        		//		sprintf(buf2[j], "%d: %s:%d\n", j, 
        		//				inet_ntoa(clients[i]->caddr.sin_addr), ntohs(clients[i]->caddr.sin_port));
        		//		j++;
        		//	}
        		//}
        		//len = 0;
        		//for(i=0;i<j;i++) {
        		//	len += strlen(buf2[i]);
        		//}
        		//sock_send_msg = calloc(1, len); // init to 0
        		//for(i=0;i<j;i++) {
        		//	strcat(sock_send_msg, buf2[i]);
        		//}
        		//_send(arg->connfd, sock_send_msg, strlen(sock_send_msg));
        	}
                /*        	
        	else if(strncmp(send_buf.m_buffer, "key:", 4)==0) { // 通話中的按鍵:0~9, *, #
        		if(busy==0) {
        			sock_send_msg = "no communication\n"; // 還在掛機狀態, 不能用這個指令
        			_send(arg->connfd, sock_send_msg, strlen(sock_send_msg));
        			return;
        		}
        		modem_mute = 1; // 靜音, 避免干擾 dtmf tone
        		usleep(500000); // delay, 因為 modem 出聲音本來就有延遲
        		//char number[50];
        		//strncpy(number, buf+5, strlen(buf)-5);
        		char buf2[3] = {0x21, 0x1, 0};
        		if(send_buf.m_buffer[4]>=49 && send_buf.m_buffer[4]<=57) { // 1~9直接送
        			buf2[2] = send_buf.m_buffer[4]-48;
        		}else if(send_buf.m_buffer[4]=='*')  {
        			buf2[2] = 0xb;
        		}else if(send_buf.m_buffer[4]=='#') {
        			buf2[2] = 0xc;
        		}else if(send_buf.m_buffer[4]=='0') {
        			buf2[2] = 0xa;
        		}
        		//send_pstn(3, buf2);
        		sock_send_msg = "key_ok\n";
        		_send(arg->connfd, sock_send_msg, strlen(sock_send_msg));
        		usleep(500000); // delay, 因為 modem 出聲音本來就有延遲
        		modem_mute = 0;
        		//exit(-1);
        	}        	
        	else if(strncmp(send_buf.m_buffer, "test_ring:", 10)==0) { // test_ring:12345 測試來電
        		sock_send_msg ="ok\n";
        		char buf2[100];
        		sprintf(buf2, "external:%s\n", send_buf.m_buffer+10);
        		//broadcast_clients(buf2);
        		_send(arg->connfd, sock_send_msg, strlen(sock_send_msg));
        	}
        	else if(strcmp(send_buf.m_buffer, "ring_end")==0) { // 響鈴停止: 來電停止, 或來電已被接起通知其他人關閉dialog
        			sock_send_msg ="ok\n";
        			broadcast_clients("ring_end\n");
        			_send(arg->connfd, sock_send_msg, strlen(sock_send_msg));
        	}
        	else if(strcmp(send_buf.m_buffer, "internal_end")==0) { // 結束內線通話
        		sock_send_msg = "internal_end\n";
        		if(arg->peer != NULL) {
        			_send(arg->peer->connfd, sock_send_msg, strlen(sock_send_msg));
        			(*arg->peer).peer = NULL; // 清除對方
        			arg->peer = NULL; // 清除自己紀錄
        			arg->busy=0;
        			printf("id=%d",arg->id);
        		} else {
        			printf("peer is null??\n");
        		}
        		sock_send_msg ="ok\n";
        		_send(arg->connfd, sock_send_msg, strlen(sock_send_msg));
        	}
        	else if(strcmp(send_buf.m_buffer, "pick_up")==0) { // 外線有人接起了
        		//broadcast_clients("ring_end\n");
        		//thread_arg_hook arg_hook;
        		//arg_hook.caddr = arg->caddr;
        		//arg_hook.number = NULL;
        		pthread_t id;
        		//_pthread_create(&id, (void*)off_hook, &arg_hook);
        		//pthread_join(id, NULL);
        		sock_send_msg = "on_hook\n";
        		//_send(arg->connfd, sock_send_msg, strlen(sock_send_msg));
        	}
        	else if(strncmp(send_buf.m_buffer, "internal:", 9)==0) { // test_dial:12345 內線呼叫
        		// find client with the id
        		for(i=0;i<MAX;i++) {
        			printf("enter internal top\n,internal id =%d\n",clients[i]->id);
        			if(clients[i]->id == atoi(send_buf.m_buffer+9)) { // atoi 會自動忽略無法轉的字元
        				// 對方忙線
        				if(clients[i]->peer != NULL || clients[i]->busy==1) {
        					sock_send_msg ="busy\n";
        					_send(arg->connfd, sock_send_msg, strlen(sock_send_msg));
        					return;
        				}
        				char buf2[32];
        				// 回傳對方 ip
        				sprintf(buf2, "internal_ip:%s\n", inet_ntoa(clients[i]->caddr.sin_addr));
        				_send(arg->connfd, buf2, strlen(buf2));
        				arg->peer = clients[i]; // 紀錄通話對象
        				
        				// 通知對方有人找他
        				sprintf(buf2, "internal:%d,%s\n", 
        						arg->id, 
        						inet_ntoa(arg->caddr.sin_addr));
        				_send(clients[i]->connfd, buf2, strlen(buf2));
        				clients[i]->peer = arg;
        				
        				return;
        			}
        		}
        		sock_send_msg ="not found\n";
        		_send(arg->connfd, sock_send_msg, strlen(sock_send_msg));
        	}
        	else if(strncmp(send_buf.m_buffer, "register:", 9)==0) { // test_dial:12345 註冊分機號碼
        		// check exist
        		for(i=0;i<MAX;i++) {
        			if(clients[i]->id == atoi(send_buf.m_buffer+9)) { // atoi 會自動忽略無法轉的字元
        				char *buf2 = "register_exist\n";
        				_send(arg->connfd, buf2, strlen(buf2));
        				return;
        			}
        		}
        		// 沒重複, 登記成功
        		arg->id = atoi(send_buf.m_buffer+9);
        		sock_send_msg ="register_ok\n";
        		_send(arg->connfd, sock_send_msg, strlen(sock_send_msg));
        	}
        	else if(strncmp(send_buf.m_buffer, "deny", 4)==0) { // 拒接外線
        		// 拿起再馬上掛掉
        		char buf2[2] = {0x12, 0}; // off-hook
        		send_pstn(2, buf2);
        		
        		buf2[0] = 0x13; // on-hook
        		send_pstn(2, buf2);
        		
        		sock_send_msg ="ok\n";
        		_send(arg->connfd, sock_send_msg, strlen(sock_send_msg));
        	}
        	else if(strncmp(send_buf.m_buffer, "switch", 6)==0) { // 插接
        		if(busy==1) {
        			//pstn_switch(); // 掛掉再馬上拿起來
        			
        			sock_send_msg = "switch_ok\n";
        			_send(arg->connfd, sock_send_msg, strlen(sock_send_msg));
        			busy = 1; // FIXME: 強制再指定成1, 要找是那邊把他變成 0 的
        		} else {
        			sock_send_msg = "no communication\n"; // 還在掛機狀態, 不能用這個指令
        			_send(arg->connfd, sock_send_msg, strlen(sock_send_msg));
        		}
        	}
        	else {
        		sock_send_msg = "unknown command\n";
        		_send(arg->connfd, sock_send_msg, strlen(sock_send_msg));
        	}
                */
                //sleep(10);
        }

        p->state = EXIT;
        PTHREAD_BUF  signal;
        if(XW_ManagePthread_SendSignal(&signal, PTHREAD_MODEM_CTRL_ID) == false)
        {
                _ERROR("PTHREAD_MODEM_CTRL_ID[%d] error !'\n", PTHREAD_MODEM_CTRL_ID);
        }
        
	_DEBUG("modem control thread exit !");

        return 0;
} 

