/**
 * TCP Server
 */
#define _GNU_SOURCE        // for pthread_setname_np
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>    // for sockaddr_in
#include <arpa/inet.h>     // for socket
#include <netinet/tcp.h>   // for TCP_KEEP*
#include <pthread.h>

#include "tcp_server.h"

#define MAX 30
#define PORT 55555

// TODO: 把每一台機器的序號寫入 factory(mtd2) 中, 把序號當註冊時使用
int busy = 0; // 是否通話中

// clients
typedef struct _thread_arg {
	struct sockaddr_in caddr;
	int connfd;
	int id; // 內線id
	struct _thread_arg *peer; // 內線通話方資訊(有代表內線忙碌)
	int busy; // 是否外線通話中
	char device_id[64]; // device_id
} thread_arg;

typedef struct _thread_arg2 {
	thread_arg *arg;
	char *buf;
} thread_arg2;

thread_arg *clients[MAX]; // max 10 clients
void handle_command(thread_arg2*);

int _recv(int connfd, void* buf, int size) {
	int n;
	if( (n=recv(connfd, buf, size, 0)) < 0) {
		perror("recv");
		//pthread_exit(NULL);
		return 0; // close socket
	}
	return n;
}

int _send(int connfd, void* buf, int size) {
	int n;
	if( (n=send(connfd, buf, size, 0)) < 0) {
		perror("send");
		pthread_exit(NULL);
	}
}

// client thread
void thread(void *t_arg) {
	char buf[100];
	//thread_arg arg = *(thread_arg*)t_arg;
	thread_arg *arg = (thread_arg*)t_arg;
	int i, n;
	
	// Set thread name
	if( pthread_setname_np(pthread_self(), inet_ntoa(arg->caddr.sin_addr)) ) {
		perror("pthread_setname_np");
		return;
	}
	
	sprintf(buf, "Hello:%d\n", arg->id); // 告知 client 他的內線號碼
	_send(arg->connfd, buf, strlen(buf));
	
	while(1) {
		n = _recv(arg->connfd, buf, sizeof(buf));
		//printf("recv: n=%d\n", n);
		if(n==0) {
			printf("connection %d closed.\n", arg->connfd);
			if(arg->busy) {
				hangup(); // 如果不正常斷線, 再次掛斷電話
				busy = 0; // 恢復狀態
			}
			// clear clients[i] data
			int has_client_busy = 0;
			for(i=0;i<MAX;i++) {
				if(clients[i]->connfd == arg->connfd) {
					// reset
					//printf("found client...\n");
					bzero(clients[i], sizeof(thread_arg));
					break;
				}
				has_client_busy = has_client_busy || clients[i]->busy;
			}
			if(!has_client_busy) {
				// 沒有人再忙碌了, 再次掛斷電話
				hangup();
				busy = 0;
			}
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
		_pthread_create(&id, (void*)handle_command, arg2);
	}
	close(arg->connfd);
	arg->connfd = 0;
}

void*  tcp_server(void) 
{
	if( pthread_setname_np(pthread_self(), "tcp_server") ) {
		perror("pthread_setname_np");
		pthread_exit(NULL);
	}
	
	init_modem();
	
	struct sockaddr_in saddr, caddr;
	int listenfd, connfd;
	
	if( (listenfd=socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		exit(-1);
	}
	
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(PORT);
	saddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	int flag = 1, keep_idle = 60, keep_interval = 5, keep_count = 3;
	int tcp_send_timeout = 10000; // 10s
	if( setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) < 0) {
		perror("setsockopt(SO_REUSEADDR)");
		exit(-1);
	}
	// 设置SO_KEEPALIVE选项，将这个选项设置为1，代表打开KeepAlive机制。
	if( setsockopt(listenfd, SOL_SOCKET, SO_KEEPALIVE, &flag, sizeof(flag)) < 0) {
		perror("setsockopt(SO_KEEPALIVE)");
		exit(-1);
	}
	// 如果TCP连接上有 60 秒钟没有任何数据包传输，则启动保活机制，发送TCP Keep-alive机制。默认为2小时。
	if( setsockopt(listenfd, IPPROTO_TCP, TCP_KEEPIDLE, &keep_idle, sizeof(flag)) < 0) {
		perror("setsockopt(TCP_KEEPIDLE)");
		exit(-1);
	}
	// 如果启动保活机制，则每隔 5 秒发送一个Keep-alive包。默认为75秒。
	if( setsockopt(listenfd, IPPROTO_TCP, TCP_KEEPINTVL, &keep_interval, sizeof(flag)) < 0) {
		perror("setsockopt(TCP_KEEPINTVL)");
		exit(-1);
	}
	// 如果对端对 3 次Keep-alive数据包都没有正常响应，则判断对端已经崩溃。默认为9。
	if( setsockopt(listenfd, IPPROTO_TCP, TCP_KEEPCNT, &keep_count, sizeof(flag)) < 0) {
		perror("setsockopt(TCP_KEEPCNT)");
		exit(-1);
	}
	// 如果发送方发送的数据包没有收到接收方回复的ACK数据包，则TCP Keep-alive机制就不会被启动
	// 值为数据包被发送后未接收到ACK确认的最大时长，代表如果发送出去的数据包在10秒内未收到ACK确认，
	// 则下一次调用send或者recv，则函数会返回-1，errno设置为ETIMEOUT，代表connection timeout。
	if( setsockopt(listenfd, IPPROTO_TCP, TCP_USER_TIMEOUT, &tcp_send_timeout, sizeof(flag)) < 0) {
		perror("setsockopt(TCP_USER_TIMEOUT)");
		exit(-1);
	}
	if( bind(listenfd, (struct sockaddr*)&saddr, sizeof(saddr)) < 0) {
		perror("bind_tcp");
		exit(-1);
	}
	
	if( listen(listenfd, 5) < 0) {
		perror("listen");
		exit(-1);
	}
	
	printf("Accepting connections...\n");
	
	int addr_len, i;
	pthread_t id, id2;
	//thread_arg *arg;
	
	for(i=0;i<MAX;i++) {
		clients[i] = (thread_arg*)malloc(sizeof(thread_arg));
		clients[i]->connfd = 0;
		clients[i]->id = 0;
		clients[i]->peer = NULL;
	}
	
	// 啟動 pstn module
	_pthread_create(&id2, (void*)_cid_thread, NULL);
	
	while(1) {
		addr_len = sizeof(caddr);
		connfd = accept(listenfd, (struct sockaddr*)&caddr, &addr_len);
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
		clients[i]->peer = NULL;
		clients[i]->busy = 0;
		clients[i]->id = i; // default internal_id
		//memcpy(&arg->caddr, &caddr, sizeof(caddr));
		if(pthread_create(&id, NULL, (void*)thread, clients[i])) {
			perror("pthread_create");
			exit(-1);
		}
		printf("Creating new thread %u\n", (int)id);
	}

	close(listenfd);
	return 0;
}

// 送訊息給所有 clients
void broadcast_clients(char *msg)
{
	int i;
	for(i=0;i<MAX;i++) {
		if(clients[i]->connfd != 0) {
			// 送通知給每個 clients
			// FIXME: 如果有 clients 網路不順, 可能會卡住, update: 加上 timeout 機制, 應該不會卡了?
			_send(clients[i]->connfd, msg, strlen(msg));
		}
	}
}

void handle_command(thread_arg2* arg2)
{
	char *buf = arg2->buf;
	thread_arg *arg = arg2->arg;

	// Set thread name
	if( pthread_setname_np(pthread_self(), "handle_command") ) {
		perror("pthread_setname_np");
		return;
	}
	//printf("handle command: %s, length=%lu\n", buf, strlen(buf));
	
	int i, j, len;
	char *msg = NULL;
	
	if(strcmp(buf, "help")==0) {
		msg = "This is help message\n";
		_send(arg->connfd, msg, strlen(msg));
	}
	else if(strcmp(buf, "list")==0) {
		char buf2[MAX+1][100];
		strcpy(buf2[0],  "Client list:\n");
		j=1;
		for(i=0;i<MAX;i++) {
			if(clients[i]->connfd != 0) {
				sprintf(buf2[j], "%d: %s:%d\n", j, 
						inet_ntoa(clients[i]->caddr.sin_addr), ntohs(clients[i]->caddr.sin_port));
				j++;
			}
		}
		len = 0;
		for(i=0;i<j;i++) {
			len += strlen(buf2[i]);
		}
		msg = calloc(1, len); // init to 0
		for(i=0;i<j;i++) {
			strcat(msg, buf2[i]);
		}
		_send(arg->connfd, msg, strlen(msg));
	}
	else if(strncmp(buf, "dial:", 5)==0) { // dial:12345
		// 檢查並線是否有摘機
		pstn_status();

		if(busy==1) {
			msg = "busy\n"; //  關閉撥號狀態
			_send(arg->connfd, msg, strlen(msg));
			return;
		}
		//char number[50];
		//strncpy(number, buf+5, strlen(buf)-5);
		thread_arg_hook arg_hook;
		arg_hook.caddr = arg->caddr;
		arg_hook.number = (char*)calloc(1, 20);
		strncpy(arg_hook.number, buf+5, strlen(buf)-5);
		//off_hook(arg->caddr, number);
		
		arg->busy = 1; // 把我自己設定成外線忙碌
		busy = 1; // 通話中
		pthread_t id;
		_pthread_create(&id, (void*)off_hook, &arg_hook);
		pthread_join(id, NULL);
		printf("dial ended, send on_hook\n");
		msg = "on_hook\n";
		busy = 0;
		arg->busy = 0; // 取消自己忙線狀態
		_send(arg->connfd, msg, strlen(msg));
		//exit(-1);
	}
	else if(strncmp(buf, "key:", 4)==0) { // 通話中的按鍵:0~9, *, #
		if(busy==0) {
			msg = "no communication\n"; // 還在掛機狀態, 不能用這個指令
			_send(arg->connfd, msg, strlen(msg));
			return;
		}
		//modem_mute = 1; // 靜音, 避免干擾 dtmf tone
		dtmf_tone(buf[4]);
		msg = "key_ok\n";
		_send(arg->connfd, msg, strlen(msg));
		//modem_mute = 0;
		//exit(-1);
	}
	else if(strcmp(buf, "hangup")==0) {
		msg = "ok\n";
		if(arg->busy==1) {
			hangup();
		}
		_send(arg->connfd, msg, strlen(msg));
	}
	else if(strncmp(buf, "test_ring:", 10)==0) { // test_ring:12345 測試來電
		msg = "ok\n";
		char buf2[100];
		sprintf(buf2, "external:%s\n", buf+10);
		broadcast_clients(buf2);
		_send(arg->connfd, msg, strlen(msg));
	}
	else if(strcmp(buf, "ring_end")==0) { // 響鈴停止: 來電停止, 或來電已被接起通知其他人關閉dialog
		msg = "ok\n";
		broadcast_clients("ring_end\n");
		_send(arg->connfd, msg, strlen(msg));
	}
	else if(strcmp(buf, "internal_end")==0) { // 結束內線通話
		msg = "internal_end\n";
		if(arg->peer != NULL) {
			_send(arg->peer->connfd, msg, strlen(msg));
			arg->peer->peer = NULL; // 清除對方
			arg->peer = NULL; // 清除自己紀錄
		} else {
			printf("peer is null??\n");
		}
		msg = "ok\n";
		_send(arg->connfd, msg, strlen(msg));
	}
	else if(strcmp(buf, "pick_up")==0) { // 外線有人接起了
		broadcast_clients("ring_end\n");
		thread_arg_hook arg_hook;
		arg_hook.caddr = arg->caddr;
		arg_hook.number = NULL;
		pthread_t id;
		arg->busy = 1;
		_pthread_create(&id, (void*)off_hook, &arg_hook);
		pthread_join(id, NULL);
		msg = "on_hook\n";
		_send(arg->connfd, msg, strlen(msg));
	}
	else if(strncmp(buf, "internal:", 9)==0) { // test_dial:12345 內線呼叫
		// find client with the id
		for(i=0;i<MAX;i++) {
			if(clients[i]->id == atoi(buf+9)) { // atoi 會自動忽略無法轉的字元
				// 對方忙線
				if(clients[i]->peer != NULL || clients[i]->busy==1) {
					printf(" clients[%d]: peer=%p, busy=%d\n", i, clients[i]->peer, clients[i]->busy);
					msg = "busy\n";
					_send(arg->connfd, msg, strlen(msg));
					return;
				}
				char buf2[32];
				// 回傳對方 ip
				sprintf(buf2, "internal_ip:%s\n", inet_ntoa(clients[i]->caddr.sin_addr));
				_send(arg->connfd, buf2, strlen(buf2));
				arg->peer = clients[i]; // 紀錄通話對象
				
				// 通知對方有人找他
				sprintf(buf2, "internal:%d,%s\n", 
						arg->id, 
						inet_ntoa(arg->caddr.sin_addr));
				_send(clients[i]->connfd, buf2, strlen(buf2));
				clients[i]->peer = arg;
				
				return;
			}
		}
		msg = "not found\n";
		_send(arg->connfd, msg, strlen(msg));
	}
	else if(strncmp(buf, "register:", 9)==0) { // test_dial:12345 註冊分機號碼
		// check exist
		for(i=0;i<MAX;i++) {
			if(clients[i]->id == atoi(buf+9)) { // atoi 會自動忽略無法轉的字元
				char *buf2 = "register_exist\n";
				_send(arg->connfd, buf2, strlen(buf2));
				return;
			}
		}
		// 沒重複, 登記成功
		arg->id = atoi(buf+9);
		msg = "register_ok\n";
		_send(arg->connfd, msg, strlen(msg));
	}
	else if(strncmp(buf, "register_device:", 16)==0) {
		// check exist
		for(i=0;i<MAX;i++) {
			if( (strcmp(clients[i]->device_id,buf+16)==0) && clients[i]->id!=0) {
				// 重複登記, 刪除之前的
				printf("remove old client...\n");
				close(clients[i]->connfd);
				bzero(clients[i], sizeof(thread_arg));
			}
		}
		strcpy(arg->device_id, buf+16);
		msg = "ok\n";
		_send(arg->connfd, msg, strlen(msg));
	}
	else if(strncmp(buf, "deny", 4)==0) { // 拒接外線
		// 拿起再馬上掛掉
		printf(" *** tcp_server 'deny' cmd\n");
		pstn_off_hook();
		pstn_on_hook();
		
		msg = "ok\n";
		_send(arg->connfd, msg, strlen(msg));
	}
	else if(strncmp(buf, "switch", 6)==0) { // 插接
		if(busy==1) {
			pstn_switch(); // 掛掉再馬上拿起來
			
			msg = "switch_ok\n";
			_send(arg->connfd, msg, strlen(msg));
			busy = 1; // FIXME: 強制再指定成1, 要找是那邊把他變成 0 的
		} else {
			msg = "no communication\n"; // 還在掛機狀態, 不能用這個指令
			_send(arg->connfd, msg, strlen(msg));
		}
	}
	else {
		msg = "unknown command\n";
		_send(arg->connfd, msg, strlen(msg));
	}
	
	//free(msg);
}
