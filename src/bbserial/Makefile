DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
ccflags-y := -std=gnu99 -Wno-declaration-after-statement
ccflags-y += -Wno-fallthrough
ccflags-y += -fno-pic -fno-pie

obj-m += bbserial.o

KERNEL_SRC ?= /usr/src/raspberrypi-kernel-headers
ARCH ?= arm
CROSS_COMPILE ?= arm-linux-gnueabihf-
DTC ?= dtc

all: bbserial.ko overlays

bbserial.ko:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERNEL_SRC) M=$(DIR) modules

overlays: $(DIR)/overlays/bbserial.dtbo

$(DIR)/overlays/bbserial.dtbo: $(DIR)/overlays/bbserial-overlay.dts
	mkdir -p $(DIR)/overlays
	$(DTC) -@ -I dts -O dtb -o $@ $<

clean:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERNEL_SRC) M=$(DIR) clean
	rm -rf $(DIR)/overlays
	rm -rf *.o *.ko *.mod.* .*.cmd Module.symvers modules.order .tmp_versions
