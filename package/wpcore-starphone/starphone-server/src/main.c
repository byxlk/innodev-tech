#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h>
//#include <string.h>
#include <pthread.h>

void tcp_server();
void udp_broadcast();

void main_thread(void* arg) {
	tcp_server();
}

void _pthread_create(pthread_t *id, void*(routine)(void*), void *arg) {
	if(pthread_create(id, NULL, (void*)routine, arg)) {
		perror("pthread_create");
		exit(-1);
	}
}

int main() {
	
	pthread_t id;
	printf("Start udp broadcast...\n");
	_pthread_create(&id, (void*)udp_broadcast, NULL);
	
	printf("Start tcp server...\n");
	_pthread_create(&id, (void*)tcp_server, NULL);
	
	pthread_join(id, NULL);
	return 0;
}