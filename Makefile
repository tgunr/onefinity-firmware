# Version and package info
VERSION := $(shell sed -n 's/^.*"version": "\([^"]*\)",.*$$/\1/p' package.json)
PKG_NAME := bbctrl-$(VERSION)

# Build targets
GPLAN_MOD := rpi-share/camotics/gplan.so
GPLAN_TARGET := src/py/gplan.so
GPLAN_IMG := rpi-share/rpi-root.img
GPLAN_KURL := http://archive.raspberrypi.org/debian/pool/main/l/linux-4.9/linux-image-4.9.0-4-rpi2_4.9.65-3+rpi2_armhf.deb
GPLAN_KPKG := $(notdir $(GPLAN_KURL))

# Firmware targets
AVR_FIRMWARE := src/avr/bbctrl-avr-firmware.hex
BBSERIAL_MOD := src/bbserial/bbserial.ko
BBSERIAL_DTBO := src/bbserial/overlays/bbserial.dtbo

# Web targets
TARGET_DIR := src/py/bbctrl/http/
HTML := $(patsubst src/pug/%.pug,$(TARGET_DIR)/%.html,$(wildcard src/pug/[^_]*.pug))
RESOURCES := $(wildcard src/resources/*)
RESOURCES := $(patsubst src/resources/%,$(TARGET_DIR)/%,$(RESOURCES))
TEMPLS := $(wildcard src/pug/templates/*.pug)

# Subprojects
SUBPROJECTS := avr boot pwr jig bbserial

ifeq ($(shell uname), Darwin)
	MAKE=/opt/homebrew/opt/make/libexec/gnubin/make
endif

# Configuration
ifndef HOST
HOST=onefinity.local
endif

ifndef PASSWORD
PASSWORD=pmn2213
endif

# Main targets
all: $(GPLAN_TARGET) firmware modules web

# .PHONY means that the targets are not files to be generated, but rather just labels
# to be used to group dependencies. This is useful for abstract targets like "all"
# and "clean", which aren't actual files.
.PHONY: all clean firmware modules pkg update install install-ssh web version kernel-headers

version:
	@echo "Version: $(VERSION)"
	@echo "Package: $(PKG_NAME)"

web: $(HTML) $(RESOURCES)

firmware: $(AVR_FIRMWARE)
	@for SUB in avr boot pwr jig; do $(MAKE) -C src/$$SUB; done

modules: $(BBSERIAL_MOD) $(BBSERIAL_DTBO)

# GPLAN build
$(GPLAN_TARGET): $(GPLAN_MOD)
	cp $< $@

$(GPLAN_MOD):
	mkdir -p rpi-share/cbang rpi-share/camotics
	git -C rpi-share/cbang init
	git -C rpi-share/cbang remote get-url origin || git -C rpi-share/cbang remote add origin https://github.com/CauldronDevelopmentLLC/cbang
	git -C rpi-share/cbang fetch --depth 1 origin 18f1e963107ef26abe750c023355a5c40dd07853
	git -C rpi-share/cbang reset --hard FETCH_HEAD
	git -C rpi-share/camotics init
	git -C rpi-share/camotics remote get-url origin || git -C rpi-share/camotics remote add origin https://github.com/CauldronDevelopmentLLC/camotics
	git -C rpi-share/camotics fetch --depth 1 origin ec876c80d20fc19837133087cef0c447df5a939d
	git -C rpi-share/camotics reset --hard FETCH_HEAD
	cd rpi-share && CFLAGS='-Os' CXXFLAGS='-Os' SQLITE_CFLAGS='-O1' scons -j1 -C cbang disable_local="re2 libevent" && \
	export CBANG_HOME="/workspace/onefinity-firmware/rpi-share/cbang" && \
	export LC_ALL=C && \
	mkdir -p camotics/build && \
	touch camotics/build/version.txt && \
	perl -i -0pe 's/(fabs\((config\.maxVel\[axis\]) \/ unit\[axis\]\));/std::min(\2, \1);/gm' camotics/src/gcode/plan/LineCommand.cpp camotics/src/gcode/plan/LinePlanner.cpp && \
	perl -i -0pe 's/(fabs\((config\.maxJerk\[axis\]) \/ unit\[axis\]\));/std::min(\2, \1);/gm' camotics/src/gcode/plan/LineCommand.cpp camotics/src/gcode/plan/LinePlanner.cpp && \
	perl -i -0pe 's/(fabs\((config\.maxAccel\[axis\]) \/ unit\[axis\]\));/std::min(\2, \1);/gm' camotics/src/gcode/plan/LineCommand.cpp camotics/src/gcode/plan/LinePlanner.cpp && \
	CFLAGS='-m32 -Os' CXXFLAGS='-m32 -Os' SQLITE_CFLAGS='-O1' scons -j1 -C camotics gplan.so with_gui=0 with_tpl=0 arch=x86

# Firmware builds
$(AVR_FIRMWARE):
	$(MAKE) -C src/avr

$(BBSERIAL_MOD) $(BBSERIAL_DTBO):
	$(MAKE) -C src/bbserial

# Package and deployment
pkg: all
	./setup.py sdist

update: pkg
	http_proxy= curl -i -X PUT -H "Content-Type: multipart/form-data" \
	  -F "firmware=@dist/$(PKG_NAME).tar.bz2" -F "password=$(PASSWORD)" \
	  http://$(HOST)/api/firmware/update

install: pkg
	cd dist && tar xf $(PKG_NAME).tar.bz2 && cd $(PKG_NAME) && \
	sudo python3 setup.py install

install-ssh: pkg
	scp dist/$(PKG_NAME).tar.bz2 root@$(HOST):/var/lib/bbctrl/firmware/update.tar.bz2
	ssh root@$(HOST) /usr/local/bin/update-bbctrl

kernel-headers:
	$(MAKE) -C src/bbserial/linux-rpi-raspberrypi-kernel_1.20171029-1 ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- INSTALL_HEADER_PATH=~/Work/OneFinity/onefinity-firmware/src/kernel/headers headers_install 

# Web build targets
$(TARGET_DIR)/%.html: src/pug/%.pug build/templates.pug
	mkdir -p $(TARGET_DIR)
	./node_modules/.bin/pug -O "{require: require}" $< -o $(TARGET_DIR)

$(TARGET_DIR)/%: src/resources/%
	mkdir -p $(TARGET_DIR)
	cp $< $@

build/templates.pug: $(TEMPLS)
	mkdir -p build
	echo "$(TEMPLS)" | tr ' ' '\n' | sed 's/.*\/\(.*\)\.pug/include \1/' > $@

# NPM dependencies
node_modules: package.json
	npm cache clean --force && npm install --legacy-peer-deps && touch node_modules

# Cleanup
clean:
	rm -rf $(GPLAN_TARGET) $(GPLAN_MOD) $(GPLAN_IMG) $(GPLAN_KPKG)
	rm -rf dist build *.egg-info node_modules $(TARGET_DIR)
	@for SUB in $(SUBPROJECTS); do \
		echo "Cleaning src/$$SUB"; \
		$(MAKE) -C src/$$SUB clean; \
	done

