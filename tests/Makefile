# Copyright 2023 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0

export EXTERNAL_BUILD_DIR ?= /tmp/dse.clib
GDB_CMD ?= valgrind -q --leak-check=full --track-origins=yes --error-exitcode=808
#GDB_CMD ?= gdb -q -ex='set confirm on' -ex=run -ex=quit

default: build

external:
	$(MAKE) -C ../extra/external

setup:
	mkdir build;
	cd build; cmake ..

build: external
# Build from scratch if no build dir.
	if [ ! -d "build" ]; then make setup; fi
# Build.
	cd build; make
	cd build; make install
	@echo ""
	@echo "Sandbox files: - $$(pwd)/build/_out"
	@echo "--------------"
	@find build/_out/ -type f -name '*' -exec ls -sh --color=auto {} \;

run:
	$(GDB_CMD) build/_out/bin/test_collections
	cd build/_out; $(GDB_CMD) bin/test_data
	cd build/_out; $(GDB_CMD) bin/test_functional
	$(GDB_CMD) build/_out/bin/test_util
	$(GDB_CMD) build/_out/bin/test_mdf
	$(GDB_CMD) build/_out/bin/test_ini
	$(GDB_CMD) build/_out/bin/test_schedule

clean:
	rm -rf build

cleanall: clean

.PHONY: default build run all clean cleanall
