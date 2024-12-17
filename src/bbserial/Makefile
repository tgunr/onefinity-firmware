CROSS := arm-linux-gnueabihf-
DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
obj-m += bbserial.o
ccflags-y := -std=gnu99 -Wno-declaration-after-statement

KPKG = raspberrypi-kernel_1.20171029-1.tar.gz
KURL = https://github.com/dbrgn/linux-rpi/archive/$(KPKG)
KDIR = linux-rpi-raspberrypi-kernel_1.20171029-1
export KERNEL = kernel7

# Add config options needed for module building
EXTRA_CFLAGS := -DCONFIG_MODVERSIONS
KCONFIG := $(KDIR)/.config

KOPTS = ARCH=arm CROSS_COMPILE=$(CROSS) -C $(KDIR)

all: $(KDIR) prepare
	$(MAKE) $(KOPTS) \
		CONFIG_DEBUG_SECTION_MISMATCH=y \
		CONFIG_MODVERSIONS=y \
        CONFIG_MODULES=y \
        CONFIG_MODULE_UNLOAD=y \
        CONFIG_MODULE_FORCE_UNLOAD=y \
        M=$(DIR) modules

prepare: $(KDIR)
	$(MAKE) $(KOPTS) bcm2709_defconfig
	$(MAKE) $(KOPTS) oldconfig
	$(MAKE) $(KOPTS) modules_prepare
	$(MAKE) $(KOPTS) scripts

$(KDIR): $(KPKG)
	tar xf $(KPKG)

$(KPKG):
	wget $(KURL)

clean:
	$(MAKE) $(KOPTS) M=$(DIR) clean
	rm -rf $(KDIR) $(KPKG)
