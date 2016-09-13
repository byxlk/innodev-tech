#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h> // read/write
#include <fcntl.h>
#include <string.h>
#include <stdlib.h> // exit
#include <netinet/in.h>// for sockaddr_in
#include <arpa/inet.h> // inet_addr
#include <pthread.h>
#include <sched.h> // sched_yield
//#include <signal.h> // pthread_kill
// terminal setting
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/usbdevice_fs.h> // usb reset
#include <stdarg.h> // va_list
#include <time.h>
#include <math.h> // ceil
#include <time.h> // clock_gettime
#include <pulse/simple.h>
#include <pulse/error.h>
#include "usbModem.h"

#define TTY "/dev/ttyACM0"
//#define TTY "/dev/ttyACM1"
//#define PSTN "/dev/ttyAMA0"
#define PSTN "/dev/ttyS2"
//#define PSTN "/dev/ttyUSB0"

//#define NO_MODEM

//char *modem_tty = TTY;
char ok[6]={0x0d,0x0a,0x4f,0x4b,0x0d,0x0a};
int modem_fd, running, hangingup, pstn_fd;
extern int busy; // 與 tcp_server 共用忙線訊號

typedef struct _my_arg {
	struct sockaddr_in caddr;
	int sock;
} my_arg;

int found_ok = 0; // modem 是否停止語音傳輸
int _pstn_switch = 0; // 紀錄目前是否是處理 插接 0=正常, 1=插接模式, 2=modem完成斷線, 3=off_hook完成,可關掉 modem_to_pa
int modem_mute = 0; // 當送出 dtmf tone 時, 暫停送出聲音給 modem
pa_simple *sp; // play

#ifdef NO_MODEM
int modem_vtr = 0; // 模擬數據機忙碌時
#endif

/* translation of dtmf codes into text */
char *dtran2[] = {
	"0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
	"*", "#", "A", "B", "C", "D", 
	"+C11 ", "+C12 ", " KP1+", " KP2+", "+ST ",
	" 2400 ", " 2600 ", " 2400+2600 ",
	" DIALTONE ", " RING ", " BUSY ","" };

int w(char *msg)
{
#ifndef NO_MODEM
	char buf[128];
	
	strcpy(buf, msg);
	int l = strlen(msg);
	buf[l] = 0x0d; // \r
	buf[l+1] = 0;
	//printf("write1: %s\n", msg);
	int size = write(modem_fd, buf, strlen(buf));
	printf("write2=%d,\t%s\n", size, msg);
	return size;
#endif
}

int r(char *out)
{
#ifndef NO_MODEM
	char buf[64];
	bzero(buf, 64);
	int i;
	usleep(300000); // wait for full response
	int size = read(modem_fd, buf, 64);
	
	// 過濾掉頭尾 \r\n
	
	/*
	for(i=0;i<size;i++) {
		printf("0x%02x ", buf[i]);
	}
	printf("\n");
	*/
	
	memcpy(buf, buf+2, size-4);
	buf[size-4]=0;
	
	/*
	for(i=0;i<strlen(buf);i++) {
		printf("0x%02x ", buf[i]);
	}
	printf("\n");
	*/
	
	printf("read=%d, %s\n", size, buf);
	//strcpy(out, buf);
	out="no data, update r()\n";
	return i;
#endif
}

void reset_modem()
{
#ifndef NO_MODEM
	// turn off DTR
	int iFlags = TIOCM_DTR;
	int i;
	hangingup = 1;
	for(i=0;i<5;i++) {
		ioctl(modem_fd, TIOCMBIC, &iFlags);
		usleep(100000);
	}
#endif
}
void hangup()
{
#ifndef NO_MODEM
	reset_modem();
	
	// on-hook pstn board
	unsigned char buf[2] = {0x13, 0};
	send_pstn(2, buf);
#else
	modem_vtr = 0;
#endif
	// reset modem on ARM
#ifdef __arm__
/*
	//close(sock);
	close(modem_fd);
	printf("Resetting USB device 001/001 (roothub)\n");
	int fd = open("/dev/bus/usb/001/001", O_WRONLY);
	int rc = ioctl(fd, USBDEVFS_RESET, 0);
	if (rc < 0) {
		perror("Error in ioctl");
		// return; // 目前測試都會 error
	}
	printf("Reset successful\n");
	close(fd);
	sleep(2); // wait for reset
	printf("Reopen modem...\n");
	modem_fd = open(modem_tty, O_RDWR);
	if(modem_fd < 0) {
		perror("reopen modem ttyACM0");
		// try ttyACM1
		modem_tty = "/dev/ttyACM1";
		modem_fd = open(modem_tty, O_RDWR);
		if(modem_fd < 0) {
			perror("fail to open modem: ttyACM1\n");
		} else {
			printf("ok\n");
		}
	}
*/
	//exit(0); // FIXME: 重開程式, 因為 modem 會卡住
	
	//char buf[32];
	//w("ATZ"); // reset
	//r(buf);
	
#endif
}

// 處理 modem 插接, 由 pstn 模組負責切換, 這邊只是關閉 modem 語音傳輸
void pstn_switch()
{
	int count=0;
	//struct timespec spec1,spec2;
	
	_pstn_switch = 1; // 處理插接
	// 先 modem 
	printf(" > stop modem...\n");
	reset_modem();
	
	// 等待 modem hangup
	printf(" > wait modem hangup...\n");
	while(_pstn_switch!=2 && count < 100) {
		usleep(50000);
		count++;
	}
	
	printf(" > start pstn board switching...\n");
	// on-hook pstn board
	//clock_gettime(CLOCK_REALTIME, &spec1);
	unsigned char buf[2] = {0x13, 0};
	send_pstn(2, buf);
	// 等待0.2秒再接起, 即會切換電話, 本來 send_pstn 已經有 0.1 delay
	usleep(100000);
	
	// off-hook pstn board, 用 dial 指令確保一定可拿起電話
	// update: dial 比較慢,　還是用 off-hook, 但需在線路上沒有人 off-hook　時才有效
	buf[0] = 0x12;
	send_pstn(2, buf);
	//clock_gettime(CLOCK_REALTIME, &spec2);
	//long ms = (spec2.tv_sec - spec1.tv_sec)*1.0e3 + (spec2.tv_nsec-spec1.tv_nsec)/1.0e6;
	//printf("PSTN switch time=%d ms\n", ms);
	printf(" > done switch\n");
	
	// 重新啟動 modem
	init_modem();
	w("AT+VLS=1"); // off-hook
	r(buf);
	
	w("AT+VTR"); // duplex voice
	sleep(1);
	r(buf);
	
	_pstn_switch=0; // 完成切換
}

/**
 * init modem
 * @return modem_fd
 */
void init_modem()
{
#ifndef NO_MODEM
	char buf[128];
	
	close(modem_fd);
	//system("echo AT > /dev/ttyACM0");
	//modem_fd = open(modem_tty, O_RDWR);
	modem_fd = open(TTY, O_RDWR|O_NOCTTY);
	//modem_fd2 = open("/tmp/a.alaw", O_WRONLY|O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	//w("AT");
	//r(buf);
	
	//reset_modem(); // 先處理掛斷, 避免上次錯誤還在通訊狀態
	
	w("ATZ"); // reset
	r(buf);
	
	w("ATE0"); // 關閉 ECHO
	r(buf);
	
	w("AT+FCLASS=8"); // voice mode
	r(buf);
	
	//w("AT+VSM=132"); // A-Law
	w("AT+VSM=131"); // mu-Law
	//w("AT+VSM=1"); // u8
	r(buf);
	
	w("AT+VNH=1"); // disable auto hangsup
	r(buf);
	
	//w("AT+VTD=50"); // DTMF duration: 0.5s
	//r(buf);
#endif
}

void _udp_recv(void *arg_in)
{
	my_arg *arg = (my_arg*)arg_in;
	
	char buf[512];
	size_t nBytes;
	
	// Set thread name
	char name[16] = "recv:";
	strcat(name, inet_ntoa(arg->caddr.sin_addr)+5); // 避免太長, ip最多15+\0
	if( pthread_setname_np(pthread_self(), name) ) {
		perror("pthread_setname_np");
		return;
	}
	
	// aec
	int error;
	//pa_simple *sp;
	pa_sample_spec ss;
	ss.format = PA_SAMPLE_ULAW;
	//ss.format = PA_SAMPLE_U8;
	ss.channels = 1;
	ss.rate = 8000;
	
	sp = pa_simple_new(NULL,               // Use the default server.
					   "aec",           // Our application's name.
					PA_STREAM_PLAYBACK,
					NULL,               // Use the default device.
					"play",            // Description of our stream.
					&ss,                // Our sample format.
					NULL,               // Use default channel map
					NULL,               // Use default buffering attributes.
					NULL               // Ignore error code.
	);
	// END aec
	
	while(running) {
		bzero(buf, 512);
		
		nBytes = recv(arg->sock, buf, 512, 0);
		if(nBytes<0) {
			perror("recv");
			running = 0;
		}
		
		// aec: 把聲音複製一份到 aec 模組
		//usleep(100000);
		//printf("pa_simple_write1\n");
		if(pa_simple_write(sp, buf, nBytes, &error)<0) {
			printf("_udp_recv pa_simple_write error: %s\n", pa_strerror(error));
			exit(-1);
		}
		//printf("pa_simple_write2\n");
		
		//nBytes = recvfrom(sock, buf, 256, 0, 
		//			(struct sockaddr*) &s_recv, &addrlen); // 知道從哪個ip送過來的 
		//printf("udp recv=%d\n", (int)nBytes);
#ifndef NO_MODEM
		/*
		if(hangingup>0) {
			printf("Send +++ to modem...\n");
			sleep(1);
			write(modem_fd, "+++", 3);
			sleep(1);
			hangingup--;
		}
		*/
		if(_pstn_switch==0 && modem_mute==0) {
			// 非插接時才會把 app 的音訊寫入 modem
			nBytes = write(modem_fd, buf, nBytes);
		} else {
			// printf("modem mute !\n");
		}
#endif
	}
	// FIXME: 為什麼不會到這邊, 確可結束?
	printf("udp recv1 stopped\n");
	pa_simple_free(sp);
}

/******** PSTN module ********/
int pstn_response=0; // 是否指令執行成功, 成功回傳指令正值, 失敗負值, 一開始 0 表示 read_pstn 不需要等待回應
int send_pstn(unsigned int size, unsigned char* cmd)
{
#ifdef NO_MODEM
	return 0;
#endif
	unsigned char 
		cksum = 0xdd ^ 0x75 ^ 0x10 ^ 0x21,
		*buf = malloc(4+size+2);
	int i, n;
	//printf("cmd sizeof=%lu\n", sizeof(cmd));
	
	pstn_response = -999;
	
	buf[0] = 0xdd;
	buf[1] = 0x75;
	buf[2] = 0x10;
	buf[3] = 0x21;
	for(i=0;i<size;i++) {
		cksum ^= cmd[i];
		buf[4+i] = cmd[i];
	}
	buf[4+i] = cksum;
	buf[4+i+1] = 0x0f;
	n = write(pstn_fd, buf, 4+size+2);
	//printf("write length=%d\n", i);
	for(i=0;i<n;i++) {
		//printf("0x%02x ", buf[i]);
	}
	//printf("send_pstn...\n");
	usleep(50000);
	/*
	while(1) {
		printf("send_pstn: wait response = %d...\n", pstn_response);
		usleep(100000);
		while(pstn_response==-999) {
			// busy wait
			sched_yield();
		}
		if(cmd[0] == pstn_response) {
			printf("pstn cmd 0x%02x is ok\n", cmd[0]);
			pstn_response = 0; // 已處理好
			return 1;
		}else if(cmd[0] == pstn_response*-1) {
			printf("pstn cmd 0x%02x is fail\n", cmd[0]);
			pstn_response = 0;
			return -1;
		}else {
			// 不是回應給該指令的回應
			pstn_response = -888; // 不是我關心的指令, 放行讓 read_pstn 可以讀取下一個回應
		}
	}
	*/
	return 1; // command finished, can go next
}
void read_pstn()
{
	unsigned char buf[32], cksum;
	int i, n=0, t, length, found_ck=0, timestamp = time(NULL)-10;
	
	while(n>=0) {
		bzero(buf, 32);
		n = cksum = found_ck = 0;
		while(1) {
			t = read(pstn_fd, buf+n, 1);
			if(t<0) {
				perror("Fail to read tty");
				pthread_exit(NULL);
			}
			//printf("read size=%d data=0x%02x cksum=0x%02x\n", t, buf[n], cksum ^ buf[n]);
			if(buf[n] == cksum) {
				found_ck = 1;
			}
			if(buf[n]==0x0f && found_ck==1) {
				//已到結尾
				n += t;
				break;
			}
			cksum ^= buf[n];
			n += t;
		}
		printf("pstn read = ");
		for(i=4;i<n-2;i++) { // 去掉頭尾
			if(i<n-2) cksum ^= buf[i]; // only cksum 0~n-2 byte
			printf("0x%02x ", buf[i]);
		}
		printf("\n");
		//printf("cksum=0x%02x\n", cksum);
		
		switch(buf[4]) {
		case 0x10:
			// caller id
			length = buf[5];
			char *number = malloc(length*2+1);
			for(i=6;i<n-2;i++) {
				number[(i-6)*2] = '0' + (buf[i]>>4 & 0xf);
				number[(i-6)*2+1] = '0' + (buf[i] & 0xf);
			}
			number[length*2+1] = 0; // \0
			printf("caller id=%s\n", number);
			
			// pstn 會送多次號碼
			if(time(NULL) - timestamp > 10) {
				// 超過10秒, 是新來電
				char buf[32];
				sprintf(buf, "external:%s\n", number);
				broadcast_clients(buf);
			}
			timestamp = time(NULL);
			break;
		case 0x11:
			broadcast_clients("ring_end\n");
			break;
		//case 0x13: // on-hook
		//case 0x12: // off-hook
		case 0x40: // 線路中有其他電話摘機
			broadcast_clients("ring_end\n");
			busy = 1; // 外線進入忙碌狀態
			break;
		case 0x41: // 線路中有其他電話掛機
			busy = 0;
			break;
		}
		
		// 沒有人關心回應, 不需等待
		if(pstn_response==0) {
			continue;
		}
		
		// 回應指令是否成功
		if(buf[5]==0x1) {
			pstn_response = (int)buf[4]; // 正確回應原指令
		} else {
			pstn_response = (int)buf[4] * -1; // 錯誤回覆 -指令
		}
		
		printf("read_pstn: pstn_response=%d\n", pstn_response);
		
		// 等待 send_pstn 處理回應
		/*
		while(1) {
			if(pstn_response==-888) {
				// 收到指令, 但不是 send_pstn 關心的, 忽略, 並讓 send_pstn 繼續等
				pstn_response = -999; 
				goto end;
			}
			if(pstn_response==0) {
				// 處理好了
				goto end;
			}
			// 還沒處理好
			sched_yield();
		}
		end:
		printf("");
		*/
	} // 等待回應
}
void _cid_thread()
{
#ifndef NO_MODEM
	// Set thread name
	char name[16] = "pstn";
	if( pthread_setname_np(pthread_self(), name) ) {
		perror("pthread_setname_np");
		return;
	}
	
	pstn_fd = open(PSTN, O_RDWR|O_NOCTTY);
	if(pstn_fd<0) {
		perror("open " PSTN);
		return;
	}
	
	// send_pstn(2, 0x34, 0x00); // ver
	
	// unsigned char buf[2] = {0x34, 0x00};
	// send_pstn(2, buf);
	
	read_pstn();
	printf("PSTN end\n");
#endif
}
//*******  END PSTN module ********/

// 把 modem 聲音輸入 aec 模組
void modem_to_pa()
{
	// Set thread name
	char name[16] = "modem_to_pa";
	if( pthread_setname_np(pthread_self(), name) ) {
		perror("pthread_setname_np");
		exit(-1);
	}
	
	char buf[256];
	char decoded[256];
	int busy_count=0, last;
	size_t s;
	int i,j;
	int fd = open("/tmp/music.input", O_WRONLY);
	while(running) {
		s = read(modem_fd, buf, 256);
		if(s<0) {
			printf("modem_to_pa read fail: %lu\n", s);
			//pthread_exit(NULL);
			exit(-1);
		}
		
		if(s==0) break; // FIXME: modem 結束可能會送 ok 也可能不會?
		for(i=0;i<s;i++) {
			for(j=0;j<6;j++) {
				if(buf[i+j]!=ok[j]) break;
			}
			if(j==6) {
				found_ok = 1;
				printf("found OK1, is switch ? = %d\n", _pstn_switch);
				if(_pstn_switch==1) {
					// 處理插接
					_pstn_switch = 2; // modem 完成 hangup, 等待 pstn board
					printf("Modem go to wait for pstn board finishing pstn switch\n");
					while(_pstn_switch!=0) {
						// 等待 pstn board 完成任務, 會切換回 0
						usleep(10000);
					}
					printf("Modem resume communication\n");
					found_ok = 0;
				} else {
					// 正常掛斷 (程式在 off_hook)
					running = 0;
					//close(sock);
					//pthread_cancel(id); // interrupt recv block
				}
			}
			decoded[i] = (MuLaw_Decode(buf[i]) >> 6) + 128; // uLaw -> s16 -> u8, 正常應該是 >> 8
			// 放大音量 8 倍, 才偵測得到 BUSY
			if(decoded[i] < 144) { // 避免爆音
				//printf(" %d ", decoded[i]);
				decoded[i] = (decoded[i]-128)*8+128;
			}
		}
		
		i = decode(decoded); // 解析 dtmf tone
		//printf("dtmf decode: i=%d, last=%d\n", i, last);
		if(i>0 && i != last) {
			printf("dtmf decode=%d, %s\n", i, dtran2[i]);
			last = i;
			if(i==26) {
				printf("busy count=%d\n", busy_count);
				// 放大後, 不會重複出現了, 因為訊號穩定?
				// busy
				busy_count++;
				if(busy_count>2) {
					hangup(); //　偵測到2次忙音(掛掉音), 自動掛斷
					// goto end_udp; // FIXME: modem 結束不會送 ok ?
				}
			}
		}
		
		s = write(fd, buf, s); // 把 modem 聲音寫到 aec 系統
	}
	printf("modem_to_pa stopped!\n");
	
	bzero(buf, 256);
	while(_pstn_switch!=3) {
		// 一直寫 0 直到 off_hook 的 pa_simple_read 結束
		s = write(fd, buf, 256); // 多送一次, 讓 off_hook 不會卡在 pa_simple_read
		usleep(10000);
	}
}

/**
 * 拿起聽筒，開始通話
 */
//void off_hook(struct sockaddr_in caddr, char* number)
void off_hook(thread_arg_hook* arg_hook)
{
	char buf[512];
// 	char decoded[256];
	int size, i, j;
// 	int busy_count=0, last;
	size_t nBytes;
	
	// Set thread name
	char name[16] = "send:";
	strcat(name, inet_ntoa(arg_hook->caddr.sin_addr)+5); // 避免太長, ip最多15+\0
	if( pthread_setname_np(pthread_self(), name) ) {
		perror("pthread_setname_np");
		return;
	}
	
	running = 1;
	hangingup = 5;
	found_ok = 0;
	_pstn_switch = 0;
	
	int sock;
	if( (sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		perror("socket : ");
		pthread_exit(NULL);
	}
	
	//init_modem();
	//w("AT+VLS=1"); // off-hook
	//r(buf);
	
	char cmd[2] = {0x13, 0}; // on-hook 撥號前再一次確定掛斷
	send_pstn(2, cmd);
	
	char *number = arg_hook->number;
	if(number != NULL) { // 接起電話時, 不會有號碼
		printf("dial number: %s\n", number);
		if(strlen(number)==0) {
			// no number, just off-hook
			char buf2[2] = {0x12, 0}; // off-hook
			send_pstn(2, buf2);
			goto end;
		}
		int length = ceil(strlen(number)/2.0); // 自動補滿偶數位, 1 byte = 2個號碼
		unsigned char *dial = (unsigned char*)malloc(length+2); // 增加最前面指令和號碼長度空間
		dial[0] = 0x20; // cmd
		dial[1] = length;
		// 號碼轉換:  09123 -> 0x09 0x12 0x3f, 0=a, *=b, #=c
		for(i=0;i<strlen(number);i++) {
			if(number[i] == '0') number[i]=0xa;
			if(number[i] == '*') number[i]=0xb;
			if(number[i] == '#') number[i]=0xc;
			if(number[i] >= '1' && number[i] <= '9') number[i] -= 48;
		}
		for(i=0;i<length;i++) {
			dial[2+i] = number[i*2] << 4; // 第一位 0x(1)2
			if(strlen(number)-i*2 == 1) {
				// 號碼奇數位, 後面要補 0x.(f)
				dial[2+i] += 0xf;
			} else {
				// 第2位 0x1(2)
				dial[2+i] += number[i*2+1];
			}
		}
		printf("pstn dial: ");
		for(i=0;i<length+2;i++) {
			printf("0x%02x ", dial[i]);
		}
		printf("\n");
		send_pstn(length+2, dial);
		/*
		char dial[64]="AT+VTS=1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0"; // send DTMF:0~9,*,#
		for(i=0;i<strlen(number);i++) {
			dial[7+i*2] = number[i];
			dial[7+i*2+1] = ',';
		}
		dial[7+i*2-1] = 0;
		printf("%s\n", dial);
		w(dial);
		usleep(300000); // 0.3s
		//sleep(1);
		r(buf);
		*/
	}
	
	end:
	init_modem();
	w("AT+VLS=1"); // off-hook
	r(buf);
	
	w("AT+VTR"); // duplex voice
	sleep(1);
	r(buf);
#ifdef NO_MODEM
	FILE *sample_fp = fopen("sample.ulaw", "r");
	modem_vtr = 1; // 模擬啟動 modem
#endif
	
	// send
	struct sockaddr_in si;
	si.sin_family = AF_INET;
	si.sin_port   = htons( 5000 );
	//si.sin_addr.s_addr = inet_addr("192.168.0.101");
	//si.sin_addr   = caddr.sin_addr;
	si.sin_addr   = arg_hook->caddr.sin_addr;
	//inet_aton( ip, &si.sin_addr.s_addr );
	
	// recv
	struct sockaddr_in si2;
	bzero(&si2, sizeof(si2));
	si2.sin_family = AF_INET;
	si2.sin_port   = htons( 5000 );
	si2.sin_addr.s_addr = htonl(INADDR_ANY);
	//si2.sin_addr.s_addr = inet_addr("192.168.2.54");
	
	// must be in the same thread as socket
	if(bind(sock, (struct sockaddr*) &si2, sizeof(si2)) < 0) {
		perror("bind_udp");
		exit(-1);
		pthread_exit(NULL);
	}
	
	pthread_t id;
	my_arg arg;
	//arg.caddr = caddr;
	arg.caddr = arg_hook->caddr;
	arg.sock = sock;
	_pthread_create(&id, (void*)_udp_recv, &arg);
	
	// aec
	pthread_t id2;
	_pthread_create(&id2, (void*)modem_to_pa, NULL);
	
	int error;
	pa_simple *sr;
	pa_sample_spec ss;
	ss.format = PA_SAMPLE_ULAW;
	//ss.format = PA_SAMPLE_U8;
	ss.channels = 1;
	ss.rate = 8000;
	
	sr = pa_simple_new(NULL,               // Use the default server.
					   "aec",           // Our application's name.
					PA_STREAM_RECORD,
					NULL,               // Use the default device.
					"record",            // Description of our stream.
					&ss,                // Our sample format.
					NULL,               // Use default channel map
					NULL,               // Use default buffering attributes.
					NULL               // Ignore error code.
	);
	// END aec
	
	while(running) {
		bzero(buf, 256);
		//printf("read 1\n");
#ifndef NO_MODEM
		//size = read(modem_fd, buf, 256);
		// 改從 aec 模組讀音訊
		//printf("pa_simple_read1\n");
		if(pa_simple_read(sr, buf, 256, &error) < 0) {
			printf("off_hook pa_simple_read error: %s\n", pa_strerror(error));
			exit(-1);
		}
		//printf("pa_simple_read2\n");
		size = 256; // pa_simple_read 會 block 直到 256
		
		//printf("read 2, size=%d\n", size);
// 		if(size==0) break; // FIXME: modem 結束可能會送 ok 也可能不會?
// 		for(i=0;i<size;i++) {
// 			for(j=0;j<6;j++) {
// 				if(buf[i+j]!=ok[j]) break;
// 			}
// 			if(j==6) {
			if(found_ok) {
 				printf("found OK2, is switch ? = %d\n", _pstn_switch);
				if(_pstn_switch==1) {
					// 處理插接
// 					_pstn_switch = 2; // modem 完成 hangup, 等待 pstn board
// 					printf("Modem go to wait for pstn board finishing pstn switch\n");
					while(_pstn_switch!=0) {
						// 等待 pstn board 完成任務, 會切換回 0
						usleep(10000);
					}
// 					printf("Modem resume communication\n");
				} else {
					// 正常掛斷
					running = 0;
					close(sock);
					pthread_cancel(id); // interrupt recv block
				}
			}
// 			decoded[i] = (MuLaw_Decode(buf[i]) >> 6) + 128; // uLaw -> s16 -> u8, 正常應該是 >> 8
// 			// 放大音量 8 倍, 才偵測得到 BUSY
// 			if(decoded[i] < 144) { // 避免爆音
// 				//printf(" %d ", decoded[i]);
// 				decoded[i] = (decoded[i]-128)*8+128;
// 			}
// 		}
// 		
// 		i = decode(decoded); // 解析 dtmf tone
// 		//printf("dtmf decode: i=%d, last=%d\n", i, last);
// 		if(i>0 && i != last) {
// 			printf("dtmf decode=%d, %s\n", i, dtran2[i]);
// 			last = i;
// 			if(i==26) {
// 				printf("busy count=%d\n", busy_count);
// 				// 放大後, 不會重複出現了, 因為訊號穩定?
// 				// busy
// 				if(busy_count++>1) {
// 					hangup(); //　偵測到2次忙音(掛掉音), 自動掛斷
// 					// goto end_udp; // FIXME: modem 結束不會送 ok ?
// 				}
// 			}
// 		}
#else
		usleep(31000);  // 8000/256=31.25
		size = fread(buf, 1, 256, sample_fp);
		//printf("fread = %d\n", size);
		if(size==0) {
			rewind(sample_fp);
		}
		
		if(modem_vtr==0) {
			printf("found OK\n");
			running = 0;
			close(sock);
			pthread_cancel(id); // interrupt recv block
		}
#endif
		//strcpy(buf, "Hello");
		//size = 5;
		//printf("sendto 1\n");
		
// 		if(modem_mute == 0) {
			nBytes = sendto(sock, buf, size, 0,
					(struct sockaddr*) &si, sizeof(si));
// 		} else {
// 			printf("modem mute (no send sound to app)\n");
// 		}
		//printf("udp send=%d\n", (int)nBytes);
	}
	//end_udp:
	_pstn_switch = 3; // off_hook 完成
	printf("udp send stopped\n");
	pthread_join(id, NULL);
	printf("udp recv2 stopped\n");
	if(sp != NULL) {
		pa_simple_free(sp);
	}
	pa_simple_free(sr);
	//pthread_exit(NULL);
}