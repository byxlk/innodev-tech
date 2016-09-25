#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>//for sockaddr_in
#include <arpa/inet.h>//for socket
#include <pthread.h>
#include <net/if.h>

//#include <sys/socket.h>

#include "include/xw_export.h"

#define MAX 15

int busy = 0; // 是否通話中

// clients
typedef struct _thread_arg {
	struct sockaddr_in caddr;
	int connfd;
	int id; // 內線id
	struct _thread_arg *peer; // 內線通話方資訊(有代表內線忙碌)
	int busy; // 是否外線通話中
} thread_arg;

typedef struct _thread_arg2 {
	thread_arg *arg;
	char *buf;
} thread_arg2;

thread_arg *clients[MAX]; // max 10 clients

int _recv(int connfd, void* buf, int size) 
{
	int n;
    
	if( (n=recv(connfd, buf, size, 0)) < 0) 
        {
		perror("recv");
		pthread_exit(NULL);
	}
    
	return n;
}

int _send(int connfd, void* buf, int size) 
{
	int n;
    
	if( (n=send(connfd, buf, size, 0)) < 0) 
        {
		perror("send");
		pthread_exit(NULL);
	}
    
        return n;
}

int GetLocalHostBroadcastIP(char *eth, char *ipaddr)
{   
        int sockfd = -1;   
        struct ifreq ifr;
        struct sockaddr_in *addr;  
        char * address;

        if(strlen(eth) >= IFNAMSIZ)
        {
                _ERROR("device name is error. Length(%d) >= %d\n",strlen(eth), IFNAMSIZ); 
                return (-1);
        }
        strcpy( ifr.ifr_name, eth);
        
        sockfd = socket(AF_INET,SOCK_DGRAM,0);
        if(sockfd < 0)
        {
                _ERROR("create socket handler faild ");
                return -1;
        }
        
        //get inet addr              
        if(ioctl(sockfd, SIOCGIFADDR, &ifr) == -1)
        {
                _ERROR("ioctl  SIOCGIFADDR error.\n");
                return (-1);
        }        
        addr = (struct sockaddr_in *)&(ifr.ifr_addr);
        address = inet_ntoa(addr->sin_addr);
        _DEBUG("inet addr: %s",address);

        //get Mask  SIOCGIFBRDADDR        
        if( ioctl( sockfd, SIOCGIFNETMASK, &ifr) == -1)
        {
                _ERROR("ioctl SIOCGIFNETMASK error.\n");
                return (-1);
         }        
        addr = (struct sockaddr_in *)&ifr.ifr_addr;
        address = inet_ntoa(addr->sin_addr);
        _DEBUG("Mask: %s",address);

        //get Broadcast address          
        if( ioctl( sockfd, SIOCGIFBRDADDR, &ifr) == -1)
        {
                _ERROR("ioctl SIOCGIFBRDADDR error.\n");
                return (-1);
         }        
        addr = (struct sockaddr_in *)&ifr.ifr_addr;
        address = inet_ntoa(addr->sin_addr);
        _DEBUG("Broadcast: %s",address);
        sprintf(ipaddr, "%s",address);
        
        //get HWaddr 
        u_int8_t hd[6];        
        if(ioctl(sockfd, SIOCGIFHWADDR, &ifr) == -1)
        {
                _ERROR("hwaddr SIOCGIFHWADDR error.\n");
                return (-1);
        }
        memcpy( hd, ifr.ifr_hwaddr.sa_data, sizeof(hd));        
        _DEBUG("HWaddr: %02X:%02X:%02X:%02X:%02X:%02X", 
                         hd[0], hd[1], hd[2], hd[3], hd[4], hd[5]);

        return 0;
}

void *XW_Pthread_Udp_Broadcast(void *args)
{  
        size_t nSendToRet = 0; 
        char broadcast_ip[16] ={'\0'}; 
        char *Sock_Msg = "Hello 你好 World!";	

        MspSendCmd_t cmdData;	//��Ϣ���д���ṹ
	PTHREAD_BUF  signal;
	STATE_PREVIEW *p;
        PTHREAD_BUF send_buf;
        SPS_SYSTEM_INFO_T *DTSystemInfo = XW_Global_InitSystemInfo();
        
        p = (STATE_PREVIEW *)XW_ManagePthread_GetPthreadState(PTHREAD_UDP_BROADCAST_ID, 0);
        p->power = PTHREAD_POWER_ON;
        p->state = ALIVE;

        memset(broadcast_ip, '\0', sizeof(broadcast_ip));
        GetLocalHostBroadcastIP(BRIDGE_NW_CARD,broadcast_ip);
        if(broadcast_ip == NULL)
        {
                memcpy(broadcast_ip,"255.255.255.255",15);
                _ERROR("get broadcast ip address faild, using default value");
        }
        
        int sock = -1;
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(sock < 0)
	{
		perror("socket : ");
		pthread_exit(NULL);
	}

        const int broadcast = 1;
	if(setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *)&broadcast, sizeof(broadcast)) != 0 )
	{
		perror("setsockopt : ");
		close(sock);
		pthread_exit(NULL);
	}
        	
        struct sockaddr_in addrto;
        bzero(&addrto, sizeof(struct sockaddr_in)); 
	addrto.sin_family = AF_INET;
	addrto.sin_port   = htons(4444);
	addrto.sin_addr.s_addr = inet_addr(broadcast_ip);       //INADDR_BROADCAST    
	//addrto.sin_addr.s_addr = htonl(INADDR_BROADCAST);
        _DEBUG("Sent msg: %s, socket %d to %s\n", Sock_Msg, sock, broadcast_ip);
        
	while(p->power == PTHREAD_POWER_ON) 
        {
                XW_ManagePthread_ReadSignal(&signal, PTHREAD_UDP_BROADCAST_ID, HI_FALSE);
		if (signal.start_id == PTHREAD_MAIN_ID && signal.m_signal == EXIT)
                {      
                        p->state = EXIT;
                        break;
                }    
        
		/* send data */
		nSendToRet = sendto(sock, Sock_Msg, strlen(Sock_Msg), 0, (struct sockaddr*) &addrto, sizeof(addrto));             //
                if(nSendToRet < 0)
                {
                        _ERROR("Error: socket %d send msg %s faild\n",sock, Sock_Msg);
                        sleep(5);
                }
		//printf("Sent msg: %s, %d bytes with socket %d to %s\n", msg, nBytes, sock, ip);
		
		sleep(1);
	}

        p->state = EXIT;
        if(XW_ManagePthread_SendSignal(&signal, PTHREAD_UDP_BROADCAST_ID) == false)
        {
                _ERROR("PTHREAD_UDP_BROADCAST_ID[%d] error !'\n", PTHREAD_UDP_BROADCAST_ID);
        }
        
	_DEBUG("Udp Broadcast thread exit !");
	
}


void *XW_Pthread_ClientConnectManage(void *args)
{        
        int i;
        int flag = 1;
        int listenfd;
        int connfd;
        int addr_len;        

        struct sockaddr_in saddr;
        struct sockaddr_in caddr;

        MspSendCmd_t cmdData;	//��Ϣ���д���ṹ
	PTHREAD_BUF  signal;
	STATE_PREVIEW *p;
        PTHREAD_BUF send_buf;
        SPS_SYSTEM_INFO_T *DTSystemInfo = XW_Global_InitSystemInfo();
        
        p = (STATE_PREVIEW *)XW_ManagePthread_GetPthreadState(PTHREAD_CLIENT_CONNECT_ID, 0);
        p->power = PTHREAD_POWER_ON;
        p->state = ALIVE;
        
        listenfd = socket(AF_INET, SOCK_STREAM, 0);
        if( listenfd < 0) 
        {
            perror("socket");
            exit(-1);
        }
        
        if( setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) < 0)
        {
            perror("setsockopt");
            exit(-1);
        }

        saddr.sin_family = AF_INET;
        saddr.sin_port = htons(55555);
        saddr.sin_addr.s_addr = htonl(INADDR_ANY); 
        if( bind(listenfd, (struct sockaddr*)&saddr, sizeof(saddr)) < 0) 
        {
            perror("bind_tcp");
            exit(-1);
        }
        
        if( listen(listenfd, 5) < 0)
        {
            perror("listen");
            exit(-1);
        }
        
        _DEBUG("Accepting connections...\n");
        
        
        for(i=0;i<MAX;i++) 
        {
            clients[i] = (thread_arg*)malloc(sizeof(thread_arg));
            clients[i]->connfd = 0;
            clients[i]->id = 0;
            clients[i]->peer = NULL;
            _DEBUG("initial clients(%d) status",i);
        } 
        
        while(p->power == PTHREAD_POWER_ON)
        {
                XW_ManagePthread_ReadSignal(&signal, PTHREAD_CLIENT_CONNECT_ID, HI_FALSE);
		if (signal.start_id == PTHREAD_MAIN_ID && signal.m_signal == EXIT)
                {      
                        p->state = EXIT;
                        break;
                }    

                //TODO:
                addr_len = sizeof(caddr);
                connfd = accept(listenfd, (struct sockaddr*)&caddr, &addr_len);
                
                _DEBUG("New connection: socket %d, ip:%s, port:%d\n", connfd,
                       inet_ntoa(caddr.sin_addr), ntohs(caddr.sin_port));
                
                for(i=0;i<MAX;i++)
               {
                    _DEBUG("find free client slot: %d\n", i);
                    if(clients[i]->connfd == 0) 
                   {
                        break;
                    }
                }
                if(i==MAX) 
                {
                    _DEBUG("Max client reached!\n");
                    char *msg = "Max client reached. Bye~\n";
                    _send(connfd, msg, strlen(msg));
                    close(connfd);
                    continue;
                }
                
                //arg = malloc(sizeof(thread_arg));
                clients[i]->connfd = connfd;
                clients[i]->caddr = caddr;
                clients[i]->id = i;
                _DEBUG("Connected Client Id =%d\n",clients[i]->id);
                //memcpy(&arg->caddr, &caddr, sizeof(caddr));

                send_buf.start_id = PTHREAD_CLIENT_CONNECT_ID;
                send_buf.m_value = i;
                send_buf.m_args = clients[i];
                XW_ManagePthread_SendSignal(&send_buf, PTHREAD_CLIENT_CONNECT_ID);
		_DEBUG("Send message PTHREAD_CLIENT_CONNECT_ID to PTHREAD_CLIENT_MANAGE_ID ok !");
        }
        
        close(listenfd);

        p->state = EXIT;
        if(XW_ManagePthread_SendSignal(&signal, PTHREAD_CLIENT_CONNECT_ID) == false)
        {
               _ERROR("PTHREAD_CLIENT_CONNECT_ID[%d] error !'\n", PTHREAD_CLIENT_CONNECT_ID);
        }
        
	_DEBUG("Client Connected thread exit !");
            
            return 0;
    }

    
void *XW_Pthread_ClientApplicationManage(void *args)
{
        int i;
        int n;
        char buf[100];
	//thread_arg *arg;

        MspSendCmd_t cmdData;	//��Ϣ���д���ṹ
	PTHREAD_BUF  signal;
	STATE_PREVIEW *p;
        PTHREAD_BUF send_buf;
        SPS_SYSTEM_INFO_T *DTSystemInfo = XW_Global_InitSystemInfo();
        
        p = (STATE_PREVIEW *)XW_ManagePthread_GetPthreadState(PTHREAD_CLIENT_MANAGE_ID, 0);
        p->power = PTHREAD_POWER_ON;
        p->state = ALIVE;

        _DEBUG("wait read client data");
        XW_ManagePthread_ReadSignal(&send_buf, PTHREAD_CLIENT_MANAGE_ID, HI_TRUE);
        
        thread_arg *arg = (thread_arg*)(send_buf.m_args);
        if(arg == NULL)
        {
                _ERROR("client[i] address transfer faild");
        }
        
	sprintf(buf, "Hello:%d\n", arg->id); // 告知 client 他的內線號碼
	_send(arg->connfd, buf, strlen(buf));
	_DEBUG("send buf data to socket");
    
	while(p->power == PTHREAD_POWER_ON)
        {
                XW_ManagePthread_ReadSignal(&signal, PTHREAD_CLIENT_MANAGE_ID, HI_FALSE);
		if (signal.start_id == PTHREAD_MAIN_ID && signal.m_signal == EXIT)
                {      
                        p->state = EXIT;
                        break;
                }    

                //TODO:
		n = _recv(arg->connfd, buf, sizeof(buf));
		//printf("recv: n=%d\n", n);
		if(n==0) {
			printf("connection %d closed.\n", arg->connfd);
			//hangup(); // 結束 modem
			break;
		} else {
			buf[n] = 0;
			_DEBUG("recv: ip=%s, length=%d, msg=%s", inet_ntoa(arg->caddr.sin_addr), n, buf);
		}
		
		// strip \r\n
		for(i=0;i<strlen(buf);i++) {
			if(buf[i] == '\r' || buf[i] == '\n') {
				buf[i] = 0;
				break;
			}
		}
		//buf[strlen(buf)-2] = 0;
		//handle_command(arg, buf);
		
		pthread_t id;
		thread_arg2 *arg2 = (thread_arg2*)malloc(sizeof(thread_arg2));
		arg2->buf = malloc(strlen(buf)+1); // for ending \0
		strcpy(arg2->buf, buf);
		arg2->arg = arg;
		//_pthread_create(&id, (void*)handle_command, arg2);
	}
	close(arg->connfd);
	arg->connfd = 0;

        p->state = EXIT;
        if(XW_ManagePthread_SendSignal(&signal, PTHREAD_CLIENT_MANAGE_ID) == false)
        {
               _ERROR("PTHREAD_CLIENT_MANAGE_ID[%d] error !'\n", PTHREAD_CLIENT_MANAGE_ID);
        }
        
	_DEBUG("Client Connected thread exit !");

        return 0;
}


