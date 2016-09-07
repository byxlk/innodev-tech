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
#include <net/if.h> // for IFNAMSIZ, ifreq, etc
#include <sys/ioctl.h>
#include <pthread.h>

#include "udp_broadcast.h"

void get_brd_addr(char *brd_addr)
{
	int fd;
	struct ifreq ifr;
	
	bzero(&ifr, sizeof(struct ifreq));
	
	fd = socket(AF_INET, SOCK_DGRAM, 0);

	/* I want to get an IPv4 IP address */
	ifr.ifr_addr.sa_family = AF_INET;

	/* I want IP address attached to "eth0" */
	strncpy(ifr.ifr_name, "br-lan", IFNAMSIZ-1);

	char *ip;
	do {
		ioctl(fd, SIOCGIFBRDADDR, &ifr);
		ip = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
		if(strcmp(ip, "4.0.0.0")==0) {
			sleep(1); // 介面尚未設定好
		} else {
			break; // 找到 ip
		}
	} while(1);
	
	strcpy(brd_addr, ip);
	close(fd);
}

void* udp_broadcast(void) 
{
#ifndef __mips__
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
	char * msg = "Hello 你好 World!";

#ifndef __mips__00
	//char *ip = "255.255.255.255";
	char *ip = "192.168.50.255";
#else
	// 必須指定明確的介面廣播(ex: 192.168.50.255), 
	// 不然 255.255.255.255 會從第一個滿足的介面廣播
	char ip[16];
	get_brd_addr(ip);
	printf("  udp broadcast addr: %s\n", ip);
#endif
	
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

#ifndef MAIN
int main() {
	udp_broadcast();
}
#endif
