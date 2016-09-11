PATH := /home/lsl/work/openwrt-rt5350/staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin/:${PATH}
CROSS_COMPILE := mipsel-openwrt-linux-
export STAGING_DIR=/home/lsl/work/openwrt-rt5350/staging_dir/
CROSS_CFLAGS := --sysroot=/home/lsl/work/openwrt-rt5350/staging_dir/target-mipsel_24kec+dsp_musl-1.1.12/ -I/home/lsl/work/openwrt-rt5350/staging_dir/target-mipsel_24kec+dsp_musl-1.1.12/usr/include

all: main
main2: usbModem.c udp_broadcast.c tcp_server.c main.c usbModem.h
	gcc usbModem.c udp_broadcast.c tcp_server.c main.c dtmf.c uLawCodec.c -g -O2 -o main -lpthread -lm -lpulse-simple -lpulse -DMAIN -DNO_MODEM
main_mips: usbAudioPSTN.c udp_broadcast.c tcp_server.c main.c usbModem.h
	$(CROSS_COMPILE)gcc $(CROSS_CFLAGS) usbAudioPSTN.c udp_broadcast.c tcp_server.c main.c dtmf.c uLawCodec.c -g -O2 -o main -lpthread -lm -lpulse-simple -lpulse -DMIPS -DMAIN #-DNO_MODEM
