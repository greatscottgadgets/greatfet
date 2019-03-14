#
# This file is part of GreatFET.
# This makefile is full of helper functions to get you up and running fast.
#

# By default, use the system's "python" binary.
PYTHON ?= python

# By default, use cmake as cmake.
CMAKE  ?= cmake

all: firmware
.PHONY: all firmware install full_install install_and_flash

#
# Convenience functiont to build our system
#
firmware:
	# Create a firmware build directory, and configure our build.
	@mkdir -p firmware/build
	pushd firmware/build; cmake ..; popd

	# Temporary: ensure libopencm3 is built.
	$(MAKE) -C firmware/libopencm3 -j$(nproc)

	# Build and install the relevant firmware.
	$(MAKE) -C firmware/build install -j$(nproc)


#
# Quick shortcut to flash a pre-programmed GreatFET into the latest state.
#
install_and_flash: full_install
	gf fw --autoflash


#
# Process for installing the host tools.
#
define install_host
	pushd libgreat/host; $(PYTHON) setup.py install; popd
	pushd host; $(PYTHON) setup.py install; popd
endef

#
#  Full install. Depends on firmware having installed its subcomponents.
#
full_install: firmware
	$(call install_host)

#
# Normal install -- installs without the requirement to build firmware first.
#
install: libgreat/README.md
	$(call install_host)


#
# Check out libgreat, if it's missing.
#
libgreat/README.md:
	git submodule update --init
