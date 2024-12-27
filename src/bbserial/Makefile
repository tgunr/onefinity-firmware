DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
ccflags-y := -std=gnu99
ccflags-y += -fno-pic -fno-pie
EXTRA_CFLAGS := -fno-stack-protector
KBUILD_CFLAGS_MODULE := -fno-stack-protector
KBUILD_MODFILE := bbserial

obj-m += bbserial.o

KERNEL_SRC := /usr/src/linux-headers-6.1.21-v8+
ARCH ?= arm64
CROSS_COMPILE ?= aarch64-linux-gnu-
DTC ?= dtc

all: bbserial.ko overlays

bbserial.ko:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERNEL_SRC) \
		CONFIG_STACKPROTECTOR_PER_TASK=n \
		KBUILD_MODNAME=bbserial \
		M=$(DIR) modules

overlays: $(DIR)/overlays/bbserial.dtbo

$(DIR)/overlays/bbserial.dtbo: $(DIR)/overlays/bbserial-overlay.dts
	mkdir -p $(DIR)/overlays
	$(DTC) -@ -I dts -O dtb -o $@ $<

clean:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERNEL_SRC) M=$(DIR) clean
	rm -rf $(DIR)/overlays
	rm -rf *.o *.ko *.mod.* .*.cmd Module.symvers modules.order .tmp_versions

.PHONY: all clean overlays bbserial.ko
