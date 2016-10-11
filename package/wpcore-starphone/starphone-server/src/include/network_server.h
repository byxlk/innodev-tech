#ifndef NETWORK_SERVER_H
#define NETWORK_SERVER_H

#define MAX 15
#define BRIDGE_NW_CARD "br-lan"

typedef struct _thread_arg {
	struct sockaddr_in caddr;
	int connfd;
	int id; // 內線id
	struct _thread_arg *peer; // 內線通話方資訊(有代表內線忙碌)
	int busy; // 是否外線通話中
} thread_arg;

int sock_recv(int connfd, void* buf, int size);
int sock_send(int connfd, void* buf, int size);

void XW_pthread_create(pthread_t *id, void*(routine)(void*), void *arg) ;
void *XW_Pthread_Udp_Broadcast(void *args);
void *XW_Pthread_ClientConnectManage(void *args);
void *XW_Pthread_ClientApplicationManage(void *args);
void *XW_Pthread_ModemCtrlDeamon(void *args);



#endif
