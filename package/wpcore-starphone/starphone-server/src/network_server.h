#ifndef NETWORK_SERVER_H
#define NETWORK_SERVER_H

void XW_pthread_create(pthread_t *id, void*(routine)(void*), void *arg) ;
void *XW_pthread_udp_broadcast(void *args);



#endif
