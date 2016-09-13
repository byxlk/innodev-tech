#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>//for sockaddr_in
#include <arpa/inet.h>//for socket
#include <pthread.h>

#include "network_server.h"



void XW_pthread_create(pthread_t *id, void*(routine)(void*), void *arg) 
{
	if(pthread_create(id, NULL, (void*)routine, arg)) {
		perror("pthread_create");
		exit(-1);
	}
}


void *XW_pthread_udp_broadcast(void *args)

{
	int sock;
        int broadcast = 1;
	char *ip = "255.255.255.255";
	char * msg = "Hello 你好 World!";
	
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
	
	while(1) {
		/* send data */
		size_t nBytes = sendto(sock, msg, strlen(msg), 0, (struct sockaddr*) &si, sizeof(si));             //
		
		printf("Sent msg: %s, %d bytes with socket %d to %s\n", msg, nBytes, sock, ip);
		
		sleep(1);
	}
	
}






