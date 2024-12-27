DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
ccflags-y := -std=gnu99 -Wno-declaration-after-statement
ccflags-y += -Wno-fallthrough
ccflags-y += -fno-pic -fno-pie

obj-m += bbserial.o

KERNEL_SRC ?= /usr/src/linux-headers-$(shell uname -r)
ARCH ?= arm
CROSS_COMPILE ?= arm-linux-gnueabihf-

all:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERNEL_SRC) M=$(DIR) modules

clean:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERNEL_SRC) M=$(DIR) clean
	rm -rf *.o *.ko *.mod.* .*.cmd Module.symvers modules.order .tmp_versions
