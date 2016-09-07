#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h> // read/write
#include <fcntl.h>
#include <string.h>
#include <stdlib.h> // exit
#include <netinet/in.h>// for sockaddr_in
#include <arpa/inet.h> // inet_addr
#include <pthread.h>
//#include <sched.h> // sched_yield
//#include <signal.h> // pthread_kill
// terminal setting
#include <fcntl.h>
#include <sys/ioctl.h>
#include <alsa/asoundlib.h>
#include <stdint.h> // uint8_t
#include "speex/speex_echo.h"
#include "speex/speex_preprocess.h"
#include "usbModem.h"

//#define TTY "/dev/ttyACM0"
//#define TTY "/dev/ttyACM1"
//#define PSTN "/dev/ttyS0"

#ifdef __mips__
#define PSTN "/dev/ttyS0"
#else
#define PSTN "/dev/ttyUSB0"
#endif

// TODO: 撥號前的並線摘機

#define BUF_SIZE 256
SpeexEchoState *st;

int running, pstn_fd, synced=0; // MCU同步用
extern int busy; // 與 tcp_server 共用忙線訊號
pthread_mutex_t mutex;

typedef struct _my_arg {
	struct sockaddr_in caddr;
	int sock;
} my_arg;

//int modem_mute = 0; // 當送出 dtmf tone 時, 暫停送出聲音給 modem

/* translation of dtmf codes into text */
char *dtran2[] = {
	"0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
	"*", "#", "A", "B", "C", "D", 
	"+C11 ", "+C12 ", " KP1+", " KP2+", "+ST ",
	" 2400 ", " 2600 ", " 2400+2600 ",
	" DIALTONE ", " RING ", " BUSY ","" };

int send_pstn(unsigned char cmd, unsigned char data)
{
	unsigned char buf[5];
	int i, n;
	//printf("cmd sizeof=%lu\n", sizeof(cmd));
	
	buf[0] = 0x5a;
	buf[1] = cmd;
	buf[2] = data;
	buf[3] = cmd+data+0x11;
	buf[4] = 0xa5;
	
	n = write(pstn_fd, buf, 5);
	//printf("write length=%d\n", i);
	for(i=0;i<n;i++) {
		//printf("0x%02x ", buf[i]);
	}
	//printf("\n");
	usleep(100000); // 100ms MCU need time to handle command
}

int read_pstn()
{
	unsigned char buf[32], cksum, number[20];
	int i, n=0, t, dtmf_length = 0;
	
	while(n>=0) {
		bzero(buf, 32);
		n = cksum = 0;
		while(1) {
			t = read(pstn_fd, buf+n, 1);
			if(t<0) {
				perror("Fail to read tty");
				exit(-1);
				pthread_exit(NULL);
			}
			if(buf[0]!=0x5a) continue; // 0x5a 為正確指令的開頭碼
			//printf("read size=%d data=0x%02x cksum=0x%02x\n", t, buf[n], cksum ^ buf[n]);
			//if(buf[n] == cksum) {
			//	found_ck = 1;
			//}
			if(buf[n]==0xa5) { // && found_ck==1) { // 0xa5 為結束碼
				//已到結尾
				n += t;
				break;
			}
			//cksum += buf[n];
			n += t;
		}
		printf("pstn read = ");
		for(i=0;i<n;i++) {
			//if(i<n-2) cksum ^= buf[i]; // only cksum 0~n-2 byte
			printf("0x%02x ", buf[i]);
		}
		printf("\n");
		cksum = buf[1]+buf[2]+0x11;
		if(cksum != buf[3]) {
			printf("cksum error, calc=0x%02x, recv=0x%02x\n", cksum, buf[3]);
			continue;
		}
		
		switch(buf[1]) {
		case 0xff: // 工作狀態
			if(buf[2]==0xaa) {
				printf("狀態: 忙線中\n");
				busy = 1;
			}
			if(buf[2]==0xff) {
				printf("狀態: OK\n");
				busy = 0;
			}
			if(buf[2]==0x00) printf("狀態: Err\n");
			break;
		case 0xee: // 錯誤
			printf("錯誤代碼: %d\n", buf[2]);
			break;
		case 0x01: // ring
			if(buf[2]==0xff) {
				printf("響鈴停止\n");
				broadcast_clients("ring_end\n");
			} else if(buf[2]==0xfe) {
				printf("並線掛機\n");
			} else {
				printf("收到震鈴, count=%d\n", buf[2]);
				broadcast_clients("ringing\n");
			}
			break;
		case 0x02: // DTMF
			printf("來電號碼, num=%d\n", buf[2]);
			// dtmf 規則: 0 number1 number2 number3 ... 15
			if(busy==0) {
				if(buf[2]==0) { // dtmf start
					dtmf_length = 0;
					bzero(number, 20);
				} else if(buf[2]==15) { // dtmf end
					number[dtmf_length] = 0;
					sprintf(buf, "external:%s\n", number);
					printf("broadcast_clients: %s, length=%d\n", buf, strlen(number));
					broadcast_clients(buf);
				} else {
					// 號碼為 1~9,10
					if(buf[2]==10) {
						number[dtmf_length++] = '0';
					} else {
						number[dtmf_length++] = '0' + buf[2];
					}
				}
			}
			break;
		case 0x03: // off-hook
			if(buf[2]==0xff) printf("OK\n");
			if(buf[2]==0x00) printf("Err\n");
			break;
		case 0x04: // on-hook
			if(buf[2]==0xff) printf("OK\n");
			if(buf[2]==0x00) printf("Err\n");
			break;
		default:
			printf("未知指令: 0x%02x\n", buf[1]);
		} // switch
		//printf("\n"); // printf flush the buffer when \n
	} // while
}

void hangup()
{
	running = 0; // 結束 udp send & recv
	// on-hook pstn board
	send_pstn(0x04, 0xff);
}

// 處理 modem 插接, 由 pstn 模組負責切換, 這邊只是關閉 modem 語音傳輸
void pstn_switch()
{
	//unsigned char buf[2] = {0x13, 0}; // 掛機
	//send_pstn(2, buf);
	send_pstn(0x04, 0xff); // on-hook
	usleep(150000); // 150ms
	//buf[0] = 0x12; // 摘機
	//send_pstn(2, buf);
	send_pstn(0x03, 0xff); // off-hook
}

void pstn_status()
{
	send_pstn(0xff, 0xff);
}

/**
 * init modem
 * @return modem_fd
 */
void init_modem()
{
}

void sig_handler(int signum)
{
	printf("Received signal %d\n", signum);
	//pa_simple_free(sp); // 有時候會出錯, why
	//sp = NULL;
	// Assertion 'pthread_mutex_destroy(&m->mutex) == 0' failed at pulseaudio-8.0/src/pulsecore/mutex-posix.c:83, function pa_mutex_free(). Aborting.
	//printf(" pa_simple_free sp done\n");
	//pthread_exit(NULL);
}
void _udp_recv(void *arg_in)
{
	my_arg *arg = (my_arg*)arg_in;
	
	int8_t buf[BUF_SIZE];
	int16_t buf2[BUF_SIZE];
	int64_t nBytes;
	int i;
	
	// Set thread name
	char name[16] = "recv:";
	strcat(name, inet_ntoa(arg->caddr.sin_addr)+5); // 避免太長, ip最多15+\0
	if( pthread_setname_np(pthread_self(), name) ) {
		perror("pthread_setname_np");
		return;
	}
	
	//signal(SIGUSR1, sig_handler); // 改用 signal handler 攔截結束動作
	// 因為 pulseaudio 如果 caller thread 先結束, pulse thread 似乎會卡住
	// 改用 select 解決卡住問題, 讓 pulseaudio thread 可以正常結束
	
	int err;
	snd_pcm_t *handle;
	//snd_pcm_sframes_t frames; // number of read/write frames, long int
	
	if ((err = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
		printf("Playback open error: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}
	if ((err = snd_pcm_set_params(handle,
					SND_PCM_FORMAT_S16_LE,
					SND_PCM_ACCESS_RW_INTERLEAVED,
					1, // channels
					8000,
					1, // allow resampling
					100000)) < 0) { // 0.1sec
		printf("Playback open error: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}
	
	// select
	fd_set active_fd_set, read_fds;
	int max_fd;
	FD_ZERO(&active_fd_set);
	FD_SET(arg->sock, &active_fd_set);
	max_fd = arg->sock;
	
	FILE *fp = fopen("/tmp/play.s16", "wb");
	//while(1) { // 用 signal 跳出
	while(running) {
		//bzero(buf, 512);
		
		struct timeval timeout;
		timeout.tv_sec = 1; // 設定 select 要等待時間
		timeout.tv_usec = 0;
		int ret;
		read_fds = active_fd_set; // 因為每次 select 後, read_fds 只會包含可以處理FDs部份
		ret = select(max_fd+1, &read_fds, NULL, NULL, &timeout);
		if(ret == -1) {
			perror("select");
			exit(-1);
		} else if (ret == 0) {
			printf("select timeout, running=%d\n", running);
			continue;
		} else {
			//printf("select data available\n");
			// data available
			for(i=0; i < FD_SETSIZE; i++) {
				if(FD_ISSET(i, &read_fds)) {
					//nBytes = recv(arg->sock, buf, 512, 0);
					nBytes = recv(i, buf, sizeof(buf), 0);
					if(nBytes<0) {
						perror("recv");
						running = 0;
						break;
					}
				} // found FDs with data
			} // foreach FDs
		} // select
		
		// decode
		for(i=0;i<nBytes;i++) {
			buf2[i] = MuLaw_Decode(buf[i]);
			buf2[i] <<= 1; // 放大音量
		}
		
		fwrite(buf2, BUF_SIZE, 2, fp);
		nBytes = snd_pcm_writei(handle, buf2, nBytes);
		if(nBytes < 0) {
			nBytes = snd_pcm_recover(handle, nBytes, 0);
			if(nBytes < 0) {
				printf("snd_pcm_writei failed: %s\n", snd_strerror(nBytes));
				exit(EXIT_FAILURE);
			}
		}
		
		// 餵入 echo cancel 模組
		speex_echo_playback(st, buf2); // echo_frame
		
		//nBytes = recvfrom(sock, buf, 256, 0, 
		//			(struct sockaddr*) &s_recv, &addrlen); // 知道從哪個ip送過來的 
		//printf("udp recv=%d\n", (int)nBytes);

	}
	fclose(fp);
	// 為什麼不會到這邊, 確可結束? 因為 pthread_cancel
	printf("udp recv1 stopped\n");
	snd_pcm_close(handle);
}

void _cid_thread()
{
	// Set thread name
	char name[16] = "cid";
	if( pthread_setname_np(pthread_self(), name) ) {
		perror("pthread_setname_np");
		exit(-1);
	}
	
	printf("Starting _cid_thread for pstn module...\n");
	int ret = system("stty -F " PSTN " 38400 ignbrk -icrnl -opost -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke raw");
	pstn_fd = open(PSTN, O_RDWR);
	if(pstn_fd<0) {
		perror("open " PSTN);
		exit(-1);
	}
	
	pthread_mutex_init(&mutex, NULL);
	//TODO: Sync PSTN board!!!!

	send_pstn(0x04, 0xff); // on-hook 確保每次程式重新啟動都是掛機狀態
	
	read_pstn();
	printf("cid end\n");
	pthread_mutex_destroy(&mutex);
}

// 通話中送出 dtmf tone, 0~9,#,*
void dtmf_tone(char num)
{
	//static int timestamp = time(NULL); // second
	char buf[20];
	//printf("play dtmf tone: %c.wav\n", num);
	
	pthread_mutex_lock(&mutex); // 阻止 APP 送太快
	if(num=='#') {
		system("aplay /root/hash.wav");
	} else if(num=='*') {
		system("aplay /root/star.wav");
	} else {
		sprintf(buf, "aplay /root/%c.wav", num);
		system(buf);
	}
	//usleep(100000); // 0.1s 指令本身執行需要時間, 造成太長 delay
	pthread_mutex_unlock(&mutex);
}

void pstn_off_hook()
{
	send_pstn(0x03, 0xff); // off-hook
}

void pstn_on_hook()
{
	send_pstn(0x04, 0xff); // on-hook
}
//*******  END PSTN module ********/

/**
 * 拿起聽筒，開始通話
 */
//void off_hook(struct sockaddr_in caddr, char* number)
void off_hook(thread_arg_hook* arg_hook)
{
	int size = BUF_SIZE;
	int16_t buf_input[size], buf_output[size], buf_echo[size]; // pcm 16, 去除回音的buf, 產生回音的buf
	//int16_t buf_zero[size], *buf_ptr; // 當來不及接收到聲音時, 用靜音的 frame
	uint8_t bufu8[size]; // u8
	int8_t  buf8[size];  // mulaw
	int busy_count=0, i, last, num;
	int64_t frames;
	
	// Set thread name
	char name[16] = "send:";
	strcat(name, inet_ntoa(arg_hook->caddr.sin_addr)+5); // 避免太長, ip最多15+\0
	if( pthread_setname_np(pthread_self(), name) ) {
		perror("pthread_setname_np");
		return;
	}
	
	running = 1;
	
	int sock;
	if( (sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		perror("socket : ");
		pthread_exit(NULL);
	}
	
	send_pstn(0x03, 0xff); // off-hook
	
	char *number = arg_hook->number;
	if(number != NULL) { // 接起電話時, 不會有號碼
		printf("dial number: %s\n", number);
		for(i=0;i<strlen(number);i++) {
			dtmf_tone(number[i]);
		}
	}
	
	// TODO: 嘗試同步 mcu
	/*
	while(synced==0) {
		write(pstn_fd, buf, 1);
		usleep(100000); // 0.1s
	}
	*/
	
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
	
// 	pthread_t id;
// 	my_arg arg;
// 	//arg.caddr = caddr;
// 	arg.caddr = arg_hook->caddr;
// 	arg.sock = sock;
// 	_pthread_create(&id, (void*)_udp_recv, &arg);
	
	// init speex Echo Cancel module
	SpeexPreprocessState *den;
	int sampleRate = 8000;
	int flag = 1;
	st = speex_echo_state_init(BUF_SIZE, BUF_SIZE*2); // frame_size/filter_length in samples
	den = speex_preprocess_state_init(BUF_SIZE, sampleRate);
	speex_echo_ctl(st, SPEEX_ECHO_SET_SAMPLING_RATE, &sampleRate);
	speex_preprocess_ctl(den, SPEEX_PREPROCESS_SET_ECHO_STATE, st); // -40, active:-15
	speex_preprocess_ctl(den, SPEEX_PREPROCESS_SET_DENOISE, &flag); // -15
	//speex_preprocess_ctl(den, SPEEX_PREPROCESS_SET_AGC, &flag);
	speex_preprocess_ctl(den, SPEEX_PREPROCESS_SET_DEREVERB, &flag);
	//speex_preprocess_ctl(den, SPEEX_PREPROCESS_GET_NOISE_SUPPRESS, &flag);
	//printf("SPEEX value=%d\n", flag);
	
	int err;
	snd_pcm_t *handle_capture;
	//snd_pcm_sframes_t frames; // number of read/write frames, long int
	if ((err = snd_pcm_open(&handle_capture, "default", SND_PCM_STREAM_CAPTURE, 0)) < 0) {
		printf("Capture open error: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}
	if ((err = snd_pcm_set_params(handle_capture,
					SND_PCM_FORMAT_S16_LE,
					SND_PCM_ACCESS_RW_INTERLEAVED,
					1, // channels
					8000,
					1, // allow resampling
					500000)) < 0) { // 0.1sec
		printf("Capture open error: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}
	
	snd_pcm_t *handle_playback;
	//snd_pcm_sframes_t frames; // number of read/write frames, long int
	if ((err = snd_pcm_open(&handle_playback, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
		printf("Playback open error: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}
	if ((err = snd_pcm_set_params(handle_playback,
					SND_PCM_FORMAT_S16_LE,
					SND_PCM_ACCESS_RW_INTERLEAVED,
					1, // channels
					8000,
					1, // allow resampling
					500000)) < 0) { // 0.1sec
		printf("Playback open error: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}
	
	// select
	fd_set active_fd_set, read_fds;
	int max_fd;
	FD_ZERO(&active_fd_set);
	FD_SET(sock, &active_fd_set);
	max_fd = sock;
	
	//FILE *fp = fopen("/tmp/a.u8", "w");
	while(running) {
		//bzero(buf, 256);
		
		/******* playback/udp_recv *******/
		// udp recv select()
		struct timeval timeout;
		timeout.tv_sec = 1; // 設定 select 要等待時間
		timeout.tv_usec = 0;
		int ret, nBytes;
		read_fds = active_fd_set; // 因為每次 select 後, read_fds 只會包含可以處理FDs部份
		ret = select(max_fd+1, &read_fds, NULL, NULL, &timeout);
		if(ret == -1) {
			perror("select");
			exit(EXIT_FAILURE);
		} else if (ret == 0) {
			printf("select timeout, running=%d\n", running);
			continue;
		} else {
			//printf("select data available\n");
			// data available
			for(i=0; i < FD_SETSIZE; i++) {
				if(FD_ISSET(i, &read_fds)) {
					//nBytes = recv(arg->sock, buf, 512, 0);
					nBytes = recv(i, buf8, sizeof(buf8), 0);
					if(nBytes<0) {
						perror("recv");
						running = 0;
						break;
					}
				} // found FDs with data
			} // foreach FDs
		} // select
		
		// decode mulaw
		for(i=0;i<nBytes;i++) {
			buf_echo[i] = MuLaw_Decode(buf8[i]);
			buf_echo[i] <<= 1; // 放大音量
		}
		
		nBytes = snd_pcm_writei(handle_playback, buf_echo, nBytes);
		if(nBytes < 0) {
			printf("snd_pcm_writei underrun !!\n");
			nBytes = snd_pcm_recover(handle_playback, nBytes, 0);
			if(nBytes < 0) {
				printf("snd_pcm_writei failed: %s\n", snd_strerror(nBytes));
				exit(EXIT_FAILURE);
			}
		}
		
		/******* capture/udp_send *******/
		// block mode, 後面 size 一定是 256
		frames = snd_pcm_readi(handle_capture, buf_input, size); // return number of frames
		//printf("after pcm_read, frames=%d\n", frames);
		if(frames < 0) {
			printf("snd_pcm_readi overrun !!\n");
			frames = snd_pcm_recover(handle_capture, frames, 0);
			if(frames < 0) {
				printf("snd_pcm_readi failed: %s\n", snd_strerror(frames));
				exit(EXIT_FAILURE);
			}
		}
		
		// 餵入 Echo Cancel 模組
		//speex_echo_capture(st, buf_input, buf_output); // input_frame, output_frame
		speex_echo_cancellation(st, buf_input, buf_echo, buf_output);
		speex_preprocess_run(den, buf_output);
		
		// convert format: s16 to u8
		for(i=0;i<size;i++) {
			uint8_t tmp = (buf_input[i]>>5) + 128; // 正常應該是 >> 8, 放大 8 倍, 才偵測得到 BUSY
			bufu8[i] = tmp;
			//printf("pcm_u8=%d\n", bufu8[i]);
		}
		//fwrite(buf_input, size, 2, fp);
		
		num = decode(bufu8); // 解析 dtmf tone
		//printf("dtmf decode: i=%d, last=%d\n", i, last);
		if(num>0 && num != last) {
			printf("dtmf decode=%d, %s\n", num, dtran2[num]);
			last = num;
			if(num==26) {
				printf("busy count=%d\n", busy_count);
				// 放大後, 不會重複出現了, 因為訊號穩定?
				// busy
				if(++busy_count>3) {
					hangup(); //　偵測到3次忙音(掛掉音), 自動掛斷
				}
			}else if(num!=27) {
				busy_count=0;
			}
		}

		// encode mulaw
		for(i=0;i<size;i++) {
			buf8[i] = MuLaw_Encode(buf_output[i]); // 有濾 echo
			//buf8[i] = MuLaw_Encode(buf_input[i]); // 無濾 echo
		}
		
		// NOTE: 這邊 mulaw 每個 frame 剛好 1 byte 才可以直接用 size
		nBytes = sendto(sock, buf8, size, 0,
				(struct sockaddr*) &si, sizeof(si));
		//printf("udp send=%d\n", (int)nBytes);
	}
	//fclose(fp);
	printf("udp send stopped\n");
	//pthread_join(id, NULL);
	printf("udp recv2 stopped\n");
	close(sock);
	snd_pcm_close(handle_capture);
	speex_echo_state_destroy(st);
	speex_preprocess_state_destroy(den);
}
#ifdef __mips__
int pthread_setname_np(pthread_t thread, const char *name)
{
	return 0;
}
#endif