DIR := $(shell dirname $(lastword $(MAKEFILE_LIST)))

NODE_MODS  := $(DIR)/node_modules
PUG        := $(NODE_MODS)/.bin/pug
STYLUS     := $(NODE_MODS)/.bin/stylus

TARGET_DIR := build/http
HTML       := index
HTML       := $(patsubst %,$(TARGET_DIR)/%.html,$(HTML))
RESOURCES  := $(shell find src/resources -type f)
RESOURCES  := $(patsubst src/resources/%,$(TARGET_DIR)/%,$(RESOURCES))
TEMPLS     := $(wildcard src/pug/templates/*.pug)

GPLAN_MOD := rpi-share/camotics/gplan.so
GPLAN_TARGET := src/py/gplan.so
GPLAN_IMG := rpi-share/rpi-root.img
GPLAN_KURL := http://archive.raspberrypi.org/debian/pool/main/l/linux-4.9/linux-image-4.9.0-4-rpi2_4.9.65-3+rpi2_armhf.deb
GPLAN_KPKG := $(notdir $(GPLAN_KURL))

AVR_FIRMWARE := src/avr/bbctrl-avr-firmware.hex
BBSERIAL_MOD := src/bbserial/bbserial.ko
BBSERIAL_DTBO := src/bbserial/overlays/bbserial.dtbo

SUBPROJECTS := avr boot pwr jig bbserial

ifndef HOST
HOST=onefinity.local
endif

ifndef PASSWORD
PASSWORD=onefinity
endif

all: $(GPLAN_TARGET) firmware modules $(HTML) $(RESOURCES)
	@for SUB in $(SUBPROJECTS); do $(MAKE) -C src/$$SUB; done

firmware: $(AVR_FIRMWARE)

modules: $(BBSERIAL_MOD) $(BBSERIAL_DTBO)

$(GPLAN_TARGET): $(GPLAN_MOD)
	cp $< $@

$(GPLAN_MOD):
	mkdir -p rpi-share/cbang rpi-share/camotics
	git -C rpi-share/cbang init
	git -C rpi-share/cbang remote add origin https://github.com/CauldronDevelopmentLLC/cbang
	git -C rpi-share/cbang fetch --depth 1 origin 18f1e963107ef26abe750c023355a5c40dd07853
	git -C rpi-share/cbang reset --hard FETCH_HEAD
	git -C rpi-share/camotics init
	git -C rpi-share/camotics remote add origin https://github.com/CauldronDevelopmentLLC/camotics
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
	CFLAGS='-Os' CXXFLAGS='-Os' SQLITE_CFLAGS='-O1' scons -j1 -C camotics gplan.so with_gui=0 with_tpl=0

$(AVR_FIRMWARE):
	$(MAKE) -C src/avr

$(BBSERIAL_MOD) $(BBSERIAL_DTBO):
	$(MAKE) -C src/bbserial

pkg: all
	./setup.py sdist

update: pkg
	http_proxy= curl -i -X PUT -H "Content-Type: multipart/form-data" \
	  -F "firmware=@dist/$(PKG_NAME).tar.bz2" -F "password=$(PASSWORD)" \
	  http://$(HOST)/api/firmware/update

install: pkg
	cd dist && tar xf $(PKG_NAME).tar.bz2 && cd $(PKG_NAME) && \
	sudo python3 setup.py install

build/templates.pug: $(TEMPLS)
	mkdir -p build
	cat $(TEMPLS) >$@

node_modules: package.json
	npm install && touch node_modules

$(TARGET_DIR)/%: src/resources/%
	install -D $< $@

src/svelte-components/dist/%:
	cd src/svelte-components && rm -rf dist && npm run build

$(TARGET_DIR)/index.html: build/templates.pug
$(TARGET_DIR)/index.html: $(wildcard src/static/js/*)
$(TARGET_DIR)/index.html: $(wildcard src/static/css/*)
$(TARGET_DIR)/index.html: $(wildcard src/pug/templates/*)
$(TARGET_DIR)/index.html: $(wildcard src/js/*)
$(TARGET_DIR)/index.html: $(wildcard src/stylus/*)
$(TARGET_DIR)/index.html: src/resources/config-template.json
$(TARGET_DIR)/index.html: $(wildcard src/resources/onefinity*defaults.json)
$(TARGET_DIR)/index.html: $(wildcard src/svelte-components/dist/*)

FORCE:

$(TARGET_DIR)/%.html: src/pug/%.pug node_modules FORCE
	cd src/svelte-components && rm -rf dist && npm run build
	@mkdir -p $(TARGET_DIR)/svelte-components
	@cp -r src/svelte-components/dist/* $(TARGET_DIR)/svelte-components/
	@$(PUG) -O pug-opts.js -P $< -o $(TARGET_DIR) || (rm -f $@; exit 1)

node_modules: package.json
	npm cache clean --force && npm install --legacy-peer-deps && touch node_modules

clean:
	rm -rf $(GPLAN_TARGET) $(GPLAN_MOD) $(GPLAN_IMG) $(GPLAN_KPKG)
	rm -rf dist build *.egg-info
	@for SUB in $(SUBPROJECTS); do $(MAKE) -C src/$$SUB clean; done

.PHONY: all install clean firmware modules pkg update