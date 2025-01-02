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

AVR_FIRMWARE := src/avr/bbctrl-avr-firmware.hex
GPLAN_MOD    := rpi-share/camotics/gplan.so
GPLAN_TARGET := src/py/camotics/gplan.so
GPLAN_IMG    := gplan-dev.img

RSYNC_EXCLUDE := \*.pyc __pycache__ \*.egg-info \\\#* \*~ .\\\#\*
RSYNC_EXCLUDE := $(patsubst %,--exclude %,$(RSYNC_EXCLUDE))
RSYNC_OPTS    := $(RSYNC_EXCLUDE) -rv --no-g --delete --force

VERSION  := $(shell sed -n 's/^.*"version": "\([^"]*\)",.*$$/\1/p' package.json)
PKG_NAME := bbctrl-$(VERSION)

SUBPROJECTS := avr boot pwr jig

ifndef HOST
HOST=onefinity.local
endif

ifndef PASSWORD
PASSWORD=onefinity
endif

SVELTE_SOURCES := $(shell find src/svelte-components/src -type f)
SVELTE_DIST := src/svelte-components/dist/index.js src/svelte-components/dist/style.css src/svelte-components/dist/smui.css

all: $(HTML) $(RESOURCES)
	@for SUB in $(SUBPROJECTS); do $(MAKE) -C src/$$SUB; done

# Check for required build tools and install if missing
check-deps:
	@echo "Checking build dependencies..."
	@which scons > /dev/null || (echo "Installing scons..." && apt-get install -y scons)
	@which g++ > /dev/null || (echo "Installing build-essential..." && apt-get install -y build-essential)
	@which git > /dev/null || (echo "Installing git..." && apt-get install -y git)
	@dpkg -l | grep python3-dev > /dev/null || (echo "Installing python3-dev..." && apt-get install -y python3-dev)
	@dpkg -l | grep python-dev > /dev/null || (echo "Installing python-dev..." && apt-get install -y python-dev)
	@dpkg -l | grep libssl-dev > /dev/null || (echo "Installing libssl-dev..." && apt-get install -y libssl-dev)
	@dpkg -l | grep binutils-dev > /dev/null || (echo "Installing binutils-dev..." && apt-get install -y binutils-dev)

# Clone and prepare dependencies
prepare-deps:
	@echo "Checking dependency repositories..."
	@echo "Cloning camotics..."
	@rm -rf rpi-share/camotics
	@rm -rf rpi-share/cbang
	@mkdir -p rpi-share
	@git clone https://github.com/CauldronDevelopmentLLC/camotics.git rpi-share/camotics
	@cd rpi-share/camotics && git checkout v1.2.0
	@git clone https://github.com/CauldronDevelopmentLLC/cbang.git rpi-share/cbang
	@cd rpi-share/cbang && git checkout 1.2.0
	@echo "Building cbang..."
	@cd rpi-share/cbang && scons -j 8 build_dir=../camotics/build
	@cd rpi-share/cbang && cp -r include/* ../camotics/build/include/
	@cd rpi-share/cbang && cp -r src/* ../camotics/build/include/
	@echo "Creating minimal SConstruct..."
	@cd rpi-share/camotics && \
		echo 'import os' > SConstruct && \
		echo 'env = Environment()' >> SConstruct && \
		echo 'env.Append(CCFLAGS = ["-O2", "-Wall", "-Werror", "-fPIC"])' >> SConstruct && \
		echo 'env.Append(CPPPATH = ["src", "../cbang", "build/include"])' >> SConstruct && \
		echo 'env.Append(CPPDEFINES = ["NDEBUG"])' >> SConstruct && \
		echo 'env.Append(LIBS = ["pthread", "dl", "cbang"])' >> SConstruct && \
		echo 'env.Append(LIBPATH = ["build/lib"])' >> SConstruct && \
		echo 'sources = [' >> SConstruct && \
		echo '    "src/gcode/ast/Assign.cpp",' >> SConstruct && \
		echo '    "src/gcode/ast/BinaryOp.cpp",' >> SConstruct && \
		echo '    "src/gcode/ast/Block.cpp",' >> SConstruct && \
		echo '    "src/gcode/ast/Comment.cpp",' >> SConstruct && \
		echo '    "src/gcode/ast/FunctionCall.cpp",' >> SConstruct && \
		echo '    "src/gcode/ast/NamedReference.cpp",' >> SConstruct && \
		echo '    "src/gcode/ast/Number.cpp",' >> SConstruct && \
		echo '    "src/gcode/ast/OCode.cpp",' >> SConstruct && \
		echo '    "src/gcode/ast/Operator.cpp",' >> SConstruct && \
		echo '    "src/gcode/ast/Program.cpp",' >> SConstruct && \
		echo '    "src/gcode/ast/QuotedExpr.cpp",' >> SConstruct && \
		echo '    "src/gcode/ast/Reference.cpp",' >> SConstruct && \
		echo '    "src/gcode/ast/UnaryOp.cpp",' >> SConstruct && \
		echo '    "src/gcode/ast/Word.cpp",' >> SConstruct && \
		echo '    "src/gcode/Axes.cpp",' >> SConstruct && \
		echo '    "src/gcode/Codes.cpp",' >> SConstruct && \
		echo '    "src/gcode/ControllerImpl.cpp",' >> SConstruct && \
		echo '    "src/gcode/interp/DoLoop.cpp",' >> SConstruct && \
		echo '    "src/gcode/interp/Evaluator.cpp",' >> SConstruct && \
		echo '    "src/gcode/interp/GCodeInterpreter.cpp",' >> SConstruct && \
		echo '    "src/gcode/interp/Interpreter.cpp",' >> SConstruct && \
		echo '    "src/gcode/interp/Loop.cpp",' >> SConstruct && \
		echo '    "src/gcode/interp/OCodeInterpreter.cpp",' >> SConstruct && \
		echo '    "src/gcode/interp/ProducerStack.cpp",' >> SConstruct && \
		echo '    "src/gcode/interp/ProgramProducer.cpp",' >> SConstruct && \
		echo '    "src/gcode/interp/RepeatLoop.cpp",' >> SConstruct && \
		echo '    "src/gcode/interp/SubroutineCall.cpp",' >> SConstruct && \
		echo '    "src/gcode/interp/SubroutineLoader.cpp",' >> SConstruct && \
		echo '    "src/gcode/machine/GCodeMachine.cpp",' >> SConstruct && \
		echo '    "src/gcode/machine/Machine.cpp",' >> SConstruct && \
		echo '    "src/gcode/machine/MachineLinearizer.cpp",' >> SConstruct && \
		echo '    "src/gcode/machine/MachineMatrix.cpp",' >> SConstruct && \
		echo '    "src/gcode/machine/MachinePipeline.cpp",' >> SConstruct && \
		echo '    "src/gcode/machine/MachineState.cpp",' >> SConstruct && \
		echo '    "src/gcode/machine/MachineUnitAdapter.cpp",' >> SConstruct && \
		echo '    "src/gcode/machine/MoveSink.cpp",' >> SConstruct && \
		echo '    "src/gcode/machine/PortType.cpp",' >> SConstruct && \
		echo '    "src/gcode/machine/TransMatrix.cpp",' >> SConstruct && \
		echo '    "src/gcode/ModalGroup.cpp",' >> SConstruct && \
		echo '    "src/gcode/Move.cpp",' >> SConstruct && \
		echo '    "src/gcode/MoveType.cpp",' >> SConstruct && \
		echo '    "src/gcode/parse/Parser.cpp",' >> SConstruct && \
		echo '    "src/gcode/parse/Tokenizer.cpp",' >> SConstruct && \
		echo '    "src/gcode/parse/TokenType.cpp",' >> SConstruct && \
		echo '    "src/gcode/plan/DwellCommand.cpp",' >> SConstruct && \
		echo '    "src/gcode/plan/InputCommand.cpp",' >> SConstruct && \
		echo '    "src/gcode/plan/LineCommand.cpp",' >> SConstruct && \
		echo '    "src/gcode/plan/LinePlanner.cpp",' >> SConstruct && \
		echo '    "src/gcode/plan/OutputCommand.cpp",' >> SConstruct && \
		echo '    "src/gcode/plan/PauseCommand.cpp",' >> SConstruct && \
		echo '    "src/gcode/plan/PlannerCommand.cpp",' >> SConstruct && \
		echo '    "src/gcode/plan/PlannerConfig.cpp",' >> SConstruct && \
		echo '    "src/gcode/plan/Planner.cpp",' >> SConstruct && \
		echo '    "src/gcode/plan/Runner.cpp",' >> SConstruct && \
		echo '    "src/gcode/plan/SCurve.cpp",' >> SConstruct && \
		echo '    "src/gcode/plan/SeekCommand.cpp",' >> SConstruct && \
		echo '    "src/gcode/plan/SetCommand.cpp",' >> SConstruct && \
		echo '    "src/gcode/Printer.cpp",' >> SConstruct && \
		echo '    "src/gcode/Tool.cpp",' >> SConstruct && \
		echo '    "src/gcode/ToolPath.cpp",' >> SConstruct && \
		echo '    "src/gcode/ToolShape.cpp",' >> SConstruct && \
		echo '    "src/gcode/ToolTable.cpp",' >> SConstruct && \
		echo '    "src/gcode/Units.cpp",' >> SConstruct && \
		echo ']' >> SConstruct && \
		echo 'lib = env.SharedLibrary("gplan", sources)' >> SConstruct && \
		echo 'env.Default(lib)' >> SConstruct

gplan: check-deps prepare-deps bbserial
	mkdir -p src/py/camotics
	cd rpi-share/camotics && scons -j 8
	cp rpi-share/camotics/libgplan.so src/py/camotics/gplan.so

pkg: gplan
	python3 setup.py bdist_deb
	mv dist/*.deb dist/$(PKG_NAME).deb
	tar czf dist/$(PKG_NAME).tar.bz2 -C dist $(PKG_NAME).deb scripts src/py/camotics/gplan.so

deploy: pkg
	mkdir -p /var/lib/bbctrl/firmware
	-pip3 uninstall -y bbctrl
	cp dist/$(PKG_NAME).tar.bz2 /var/lib/bbctrl/firmware/update.tar.bz2 
	find scripts -name "*.sh" -o -name "*.py" -exec chmod +x {} \;
	find scripts -name "edit-boot-config" -exec chmod +x {} \;
	systemctl stop bbctrl
	cd /tmp && \
	rm -rf bbctrl-1.4.3 && \
	tar xf /var/lib/bbctrl/firmware/update.tar.bz2 && \
	cd bbctrl-1.4.3 && \
	pip3 install -e . && \
	./scripts/install.sh

bbserial:
	$(MAKE) -C src/bbserial

$(GPLAN_TARGET): $(GPLAN_MOD)
	cp $< $@

$(GPLAN_MOD): $(GPLAN_IMG)
	./scripts/gplan-init-build.sh
	cp ./scripts/gplan-build.sh rpi-share/
	chmod +x rpi-share/gplan-build.sh
	sudo ./scripts/rpi-chroot.sh $(GPLAN_IMG) /mnt/host/gplan-build.sh

$(GPLAN_IMG):
	./scripts/gplan-init-build.sh

$(SVELTE_DIST): $(SVELTE_SOURCES) src/svelte-components/package.json src/svelte-components/tsconfig.json
	cd src/svelte-components && npm run build

.PHONY: $(AVR_FIRMWARE)
$(AVR_FIRMWARE):
	$(MAKE) -C src/avr 

update: pkg
	http_proxy= curl -i -X PUT -H "Content-Type: multipart/form-data" \
	  -F "firmware=@dist/$(PKG_NAME).tar.bz2" -F "password=$(PASSWORD)" \
	  http://$(HOST)/api/firmware/update
	@-tput sgr0 && echo # Fix terminal output

build/templates.pug: $(TEMPLS)
	mkdir -p build
	cat $(TEMPLS) >$@

node_modules: package.json
	npm install && touch node_modules

$(TARGET_DIR)/%: src/resources/%
	install -D $< $@

$(TARGET_DIR)/svelte-components/%: $(SVELTE_DIST)
	@mkdir -p $(TARGET_DIR)/svelte-components
	cp src/svelte-components/dist/* $(TARGET_DIR)/svelte-components/

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
	cp src/svelte-components/dist/* $(TARGET_DIR)/svelte-components/

	@mkdir -p $(TARGET_DIR)
	$(PUG) -O pug-opts.js -P $< -o $(TARGET_DIR) || (rm -f $@; exit 1)

clean:
	rm -rf rpi-share
	git clean -fxd

.PHONY: all install clean tidy pkg gplan lint pylint jshint bbserial deploy