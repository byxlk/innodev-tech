/**
 * UDP broadcast
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>//for sockaddr_in
#include <arpa/inet.h>//for socket
#include <pthread.h>

void udp_broadcast() {
#ifndef MIPS
	if( pthread_setname_np(pthread_self(), "udp_broadcast") ) {
		perror("pthread_setname_np");
		return;
	}
#endif
	
	int sock;
	if( (sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		perror("socket : ");
		pthread_exit(NULL);
	}
	
    int broadcast = 1;
	if( setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) != 0 )
	{
		perror("setsockopt : ");
		close(sock);
		pthread_exit(NULL);
	}
	
	char *ip = "255.255.255.255";
	char * msg = "Hello 你好 World!";
	
	struct sockaddr_in si;
	si.sin_family = AF_INET;
	si.sin_port   = htons( 4444 );
	si.sin_addr.s_addr = inet_addr(ip);
	//inet_aton( ip, &si.sin_addr.s_addr );
	
	while(1) {
		/* send data */
		size_t nBytes = sendto(sock, msg, strlen(msg), 0, 
					   (struct sockaddr*) &si, sizeof(si));
		
		//printf("Sent msg: %s, %d bytes with socket %d to %s\n", msg, nBytes, sock, ip);
		
		sleep(1);
	}
	
	return;
}

int main0() {
	udp_broadcast();
}