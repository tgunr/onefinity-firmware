CROSS_COMPILE := arm-linux-gnueabihf-
DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
ccflags-y := -std=gnu99 -Wno-declaration-after-statement
ccflags-y += -Wno-fallthrough

obj-m += bbserial.o

KDIR := linux-rpi-raspberrypi-kernel_1.20171029-1
KOPTS = ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE) -C $(KDIR)

all: $(KDIR) prepare
	$(MAKE) $(KOPTS) M=$(DIR) modules

prepare: $(KDIR)
	$(MAKE) $(KOPTS) bcm2709_defconfig
	$(MAKE) $(KOPTS) modules_prepare

ifeq ($(wildcard $(KDIR)),)
$(KDIR): $(KPKG)
	tar xf $(KPKG)
endif

$(KPKG):
	wget $(KURL)

clean:
	$(MAKE) $(KOPTS) M=$(DIR) clean
	rm -rf $(KDIR) $(KPKG)
