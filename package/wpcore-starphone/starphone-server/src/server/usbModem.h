#ifndef _USBMODEM_H
#define _USBMODEM_H 1
typedef struct _thread_arg_hook {
	struct sockaddr_in caddr;
	char *number;
} thread_arg_hook;

void init_modem();
//void off_hook(struct sockaddr_in, char *number);
void off_hook(thread_arg_hook*);
void hangup();
void _pthread_create(pthread_t *id, void*(routine)(void*), void *arg);
void broadcast_clients(char *); // from tcp_server
void pstn_switch(); // 插接

// PSTN
void _cid_thread(); // pstn module
//int send_pstn(unsigned int, unsigned char, ...);
int send_pstn(unsigned int, unsigned char*);
int modem_mute;

// dtmf & uLaw decoder
int decode(char*);
int16_t MuLaw_Decode(int8_t);

// for uclibc, no pthread_setname_np
#ifdef MIPS
int pthread_setname_np(pthread_t thread, const char *name);
#endif

#endif