//#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "tcp_server.h"
#include "udp_broadcast.h"
#include "si3050_ctrl.h"


void main_thread(void* arg) {
	tcp_server();
}

void _pthread_create(pthread_t *id, void*(routine)(void*), void *arg) {
	if(pthread_create(id, NULL, (void*)routine, arg)) {
		perror("pthread_create");
		exit(-1);
	}
}

int main(int argc, char **argv) 
{
	
	pthread_t id;

        printf("Init Si3050 PSTN SoC ...");
        si3050_sys_init();
    
	printf("Start udp broadcast...\n");
	_pthread_create(&id, (void*)udp_broadcast, NULL);
	
	printf("Start tcp server...\n");
	_pthread_create(&id, (void*)tcp_server, NULL);
	
	pthread_join(id, NULL);
	return 0;
}
