PATH := /home/lsl/work/openwrt-rt5350/staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin/:${PATH}
CROSS_COMPILE := mipsel-openwrt-linux-
export STAGING_DIR=/home/lsl/work/openwrt-rt5350/staging_dir/
CROSS_CFLAGS := --sysroot=/home/lsl/work/openwrt-rt5350/staging_dir/target-mipsel_24kec+dsp_musl-1.1.12/ -I/home/lsl/work/openwrt-rt5350/staging_dir/target-mipsel_24kec+dsp_musl-1.1.12/usr/include

#all: amrnb-dec-pa amrnb-enc-pa dtmf-gen usbModem-pa main
#amrnb-dec-pa: amrnb-dec-pa.c
#	gcc amrnb-dec-pa.c -O2 -o amrnb-dec-pa -lpulse -lpulse-simple -lopencore-amrnb

#amrnb-enc-pa: amrnb-enc-pa.c
#	gcc amrnb-enc-pa.c -O2 -o amrnb-enc-pa -lpulse -lpulse-simple -lopencore-amrnb

#usbModem-pa: usbModem-pa.c
#	gcc usbModem-pa.c -O2 -o usbModem-pa -lpulse -lpulse-simple

#dtmf-gen: dtmf-gen.c
#	gcc dtmf-gen.c -O2 -o dtmf-gen -Wno-deprecated-declarations -Wno-unused-result

#dtmf: dtmf.c
#	gcc dtmf.c -O2 -o dtmf

#PSTN_board2: PSTN_board2.c
#	$(CROSS_COMPILE)gcc PSTN_board2.c -o PSTN_board2_mips
#	cp PSTN_board2_mips /data/Ralink/openwrt/files/root/

#main_modem: udp_broadcast.c tcp_server.c main.c usbModem.h usbModem.c
#	gcc usbModem.c udp_broadcast.c tcp_server.c main.c dtmf.c uLawCodec.c -g -O2 -o main_modem -lpthread -lm -lpulse-simple -lpulse -DMAIN -DNO_MODEM

#main_chen:  udp_broadcast.c tcp_server.c main.c usbModem.h usbAudioChen.c
#	gcc usbAudioChen.c udp_broadcast.c tcp_server.c main.c dtmf.c uLawCodec.c -g -O2 -o main_chen -lpthread -lm -lpulse-simple -lpulse -DMAIN
#main_pa:    udp_broadcast.c tcp_server.c main.c usbModem.h usbAudioPSTN_pulse.c
#	$(CROSS_COMPILE)gcc $(CROSS_CFLAGS) usbAudioPSTN_pulse.c udp_broadcast.c tcp_server.c main.c dtmf.c uLawCodec.c -g -O2 -o main -lpthread -lm -lpulse-simple -lpulse -DMAIN
#	#mv main /data/Ralink/openwrt/files/root/

main:       udp_broadcast.c tcp_server.c main.c usbModem.h usbAudioPSTN.c
	$(CROSS_COMPILE)gcc $(CROSS_CFLAGS) usbAudioPSTN.c udp_broadcast.c tcp_server.c main.c dtmf.c uLawCodec.c -g -O2 -o main -lpthread -lm -lasound -lspeexdsp -DMAIN
	cp main /data/Ralink/openwrt/files/root/
