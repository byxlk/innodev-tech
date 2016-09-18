#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>//for sockaddr_in
#include <arpa/inet.h>//for socket
#include <pthread.h>
//#include <sys/socket.h>

#include "include/xw_export.h"

#define MAX 30

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

void *XW_Pthread_Udp_Broadcast(void *args)
{
        SPS_SYSTEM_INFO_T *DTSystemInfo = XW_Global_InitSystemInfo();
	int sock;
        int broadcast = 1;
	char *ip = "255.255.255.255";
	char *msg = "Hello 你好 World!";
	size_t nBytes = 0;
    
	struct sockaddr_in si;

        //AF_INET: using tcp/ip & ipv4
        //SOCK_DGRAM:
        //IPPROTO_UDP: using udp transfer data
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(sock < 0)
	{
		perror("socket : ");
		pthread_exit(NULL);
	}
	
	if(setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) != 0 )
	{
		perror("setsockopt : ");
		close(sock);
		pthread_exit(NULL);
	}
	
	si.sin_family = AF_INET;
	si.sin_port   = htons( 4444 );
	si.sin_addr.s_addr = inet_addr(ip);
	//inet_aton( ip, &si.sin_addr.s_addr );
	
        printf("Sent msg: %s, socket %d to %s\n", msg, sock, ip);
	while(1) {
		/* send data */
		nBytes = sendto(sock, msg, strlen(msg), 0, (struct sockaddr*) &si, sizeof(si));             //

                if(nBytes < 0)
                {
                        printf("Error: socket %d send msg %s faild\n",sock, msg);
                }
		//printf("Sent msg: %s, %d bytes with socket %d to %s\n", msg, nBytes, sock, ip);
		
		sleep(1);
	}
	
}


void *XW_Pthread_ClientConnectManage(void *args)
{
        SPS_SYSTEM_INFO_T *DTSystemInfo = XW_Global_InitSystemInfo();
        int i;
        int flag = 1;
        int listenfd;
        int connfd;
        int addr_len;
        
        //pthread_t id;
        //pthread_t id2;
        struct sockaddr_in saddr;
        struct sockaddr_in caddr;
        
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
        
        printf("Accepting connections...\n");
        
        
        for(i=0;i<MAX;i++) {
            clients[i] = (thread_arg*)malloc(sizeof(thread_arg));
            clients[i]->connfd = 0;
            clients[i]->id = 0;
            clients[i]->peer = NULL;
            printf("initial clients status\n");
        } 
        
        while(1) {
            addr_len = sizeof(caddr);
            connfd = accept(listenfd, (struct sockaddr*)(&caddr), &addr_len);
            printf("New connection: socket %d, ip:%s, port:%d\n", connfd,
                   inet_ntoa(caddr.sin_addr), ntohs(caddr.sin_port));
            
            for(i=0;i<MAX;i++) {
                printf("find free client slot: %d\n", i);
                if(clients[i]->connfd == 0) {
                    break;
                }
            }
            if(i==MAX) {
                printf("Max client reached!\n");
                char *msg = "Max client reached. Bye~\n";
                _send(connfd, msg, strlen(msg));
                close(connfd);
                continue;
            }
            //arg = malloc(sizeof(thread_arg));
            clients[i]->connfd = connfd;
            clients[i]->caddr = caddr;
            clients[i]->id = i;
            printf("client id =%d\n",clients[i]->id);
            //memcpy(&arg->caddr, &caddr, sizeof(caddr));
//            if(pthread_create(&id, NULL, (void*)thread, clients[i])) {
//                perror("pthread_create");
//                exit(-1);
//            }
//            printf("Creating new thread %u\n", (int)id);
        }
    
        close(listenfd);
        return 0;
    }

    
void *XW_Pthread_ClientApplicationManage(void *args)
{
        SPS_SYSTEM_INFO_T *DTSystemInfo = XW_Global_InitSystemInfo();
        char buf[100];
	//thread_arg arg = *(thread_arg*)t_arg;
	thread_arg *arg = (thread_arg*)args;
	int i, n;
	
	sprintf(buf, "Hello:%d\n", arg->id); // 告知 client 他的內線號碼
	_send(arg->connfd, buf, strlen(buf));
	
	while(1) {
		n = _recv(arg->connfd, buf, sizeof(buf));
		//printf("recv: n=%d\n", n);
		if(n==0) {
			printf("connection %d closed.\n", arg->connfd);
			//hangup(); // 結束 modem
			break;
		} else {
			buf[n] = 0;
			printf("recv: ip=%s, length=%d, msg=%s", inet_ntoa(arg->caddr.sin_addr), n, buf);
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

        return 0;
}


