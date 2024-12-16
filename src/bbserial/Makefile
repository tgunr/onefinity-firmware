CROSS:=arm-linux-gnueabihf-
DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
obj-m:=bbserial.o
ccflags-y:=-std=gnu99 -Wno-declaration-after-statement

KDIR=/build/linux-rpi-raspberrypi-kernel_1.20171029-1
KOPTS=ARCH=arm CROSS_COMPILE=$(CROSS) -C $(KDIR)

all:
	$(MAKE) $(KOPTS) M=$(DIR) modules

clean:
	$(MAKE) $(KOPTS) M=$(DIR) clean
