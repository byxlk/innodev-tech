# !/bin/bash

make V=99
sudo rm -rf /work/tftpboot/test.bin
sudo cp -rf bin/ramips/openwrt-ramips-rt305x-sxx-8m-squashfs-sysupgrade.bin /work/tftpboot/test.bin
