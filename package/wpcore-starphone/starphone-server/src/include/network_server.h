#ifndef NETWORK_SERVER_H
#define NETWORK_SERVER_H

#define BRIDGE_NW_CARD "br-lan"

void XW_pthread_create(pthread_t *id, void*(routine)(void*), void *arg) ;
void *XW_Pthread_Udp_Broadcast(void *args);
void *XW_Pthread_ClientConnectManage(void *args);
void *XW_Pthread_ClientApplicationManage(void *args);
void *XW_Pthread_ModemCtrlDeamon(void *args);



#endif
