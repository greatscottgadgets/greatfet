#
# This file is part of GreatFET.
# This makefile is full of helper functions to get you up and running fast.
#

# Build using Bash script semantics.
SHELL := /bin/bash

# By default, use the system's "python" binary.
PYTHON  ?= python
PYTHON2 ?= python2
PYTHON3 ?= python3

# By default, use cmake as cmake.
CMAKE  ?= cmake

# Used only for deploying nightlies -- only DEPLOY_COMMAND is used in the text below.
DEPLOY_USER    ?= deploy
DEPLOY_PATH    ?= ~/nightlies/greatfet
DEPLOY_COMMAND ?= scp -r * $(DEPLOY_USER)@$(DEPLOY_SERVER):$(DEPLOY_PATH)

all: firmware
.PHONY: all firmware install full_install install_and_flash menuconfig

# Flags for creating build archives.
# These effectively tell the release tool how to modify git-archive output to create a complete build.
ARCHIVE_FLAGS = \
	--extra=VERSION --extra=RELEASENOTES $(FIRMWARE_BIN_FLAGS) $(HOST_PACKAGE_FLAGS) \
	--force-submodules --prefix=greatfet-$(VERSION)/
ARCHIVE_FLAGS_NIGHTLY = \
	$(FIRMWARE_BIN_FLAGS) $(HOST_PACKAGE_FLAGS) --force-submodules --prefix=greatfet-$(VERSION)/


#
# Convenience targets for our inner build system.
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


menuconfig: libgreat/README.md
	pushd firmware/build; ccmake ..; popd


#
# Check out libgreat, if it's missing.
#
libgreat/README.md:
	git submodule update --init


#
# Prepares a GreatFET release based on the VERSION arguments and based on a RELEASENOTES file.
#
prepare_release: firmware RELEASENOTES
	@mkdir -p release-files/

	@echo Tagging release $(VERSION).
	@git tag -a v$(VERSION) -m "release $(VERSION)" $(TAG_OPTIONS)
	@git -C libgreat tag -a v$(VERSION) -m "release $(VERSION)" $(TAG_OPTIONS)
	@echo "$(VERSION)" > VERSION
	@echo "$(VERSION)" > libgreat/VERSION

	@echo --- Creating our host-python distribution directories
	@rm -rf host-packages
	@mkdir -p host-packages

	@#Python 2
	@pushd libgreat/host; $(PYTHON2) setup.py bdist_wheel -d $(CURDIR)/host-packages; popd
	@pushd host; $(PYTHON2) setup.py bdist_wheel -d $(CURDIR)/host-packages; popd

	@#Python 3
	@pushd libgreat/host; $(PYTHON3) setup.py bdist_wheel -d $(CURDIR)/host-packages; popd
	@pushd host; $(PYTHON3) setup.py bdist_wheel -d $(CURDIR)/host-packages; popd

	@echo --- Creating our firmware-binary directory.
	@# Extract the firmware-binaries from the assets folder we've produced.
	@rm -rf firmware-bin
	@cp -r host/greatfet/assets firmware-bin

	@# And remove the irreleveant README/.gitignore that have carried over from the assets folder.
	@rm firmware-bin/.gitignore
	@rm firmware-bin/README

	@echo --- Preparing the release archives.
	$(eval FIRMWARE_BIN_FLAGS := $(addprefix --extra=, $(wildcard firmware-bin/*)))
	$(eval HOST_PACKAGE_FLAGS := $(addprefix --extra=, $(wildcard host-packages/*)))
	@git-archive-all $(ARCHIVE_FLAGS) release-files/greatfet-$(VERSION).tar.xz
	@git-archive-all $(ARCHIVE_FLAGS) release-files/greatfet-$(VERSION).zip

	@echo
	@echo Archives seem to be ready in ./release-files.
	@echo If everything seems okay, you probably should push the relevant tag:
	@echo "    git push origin v$(VERSION)"
	@echo "    git -C libgreat push origin v$(VERSION)"
	@echo
	@echo And push the relevant packages to Pypi:
	@echo "    python3 -m twine upload host-packages/*"



#
# Deploys a nightly build to the build repository. Intended to help with our Travis environment.
#
# Assumes you have a DEPLOY_COMMAND populated that will deploy the contents of the working directory to
# the remote server.
#
deploy_nightly: prepare_nightly
	$(eval DEPLOY_FILES_PATH := deploy-files/$(shell date +%Y/%m)/)

	@echo --- Creating our deploy files.
	@mkdir -p $(DEPLOY_FILES_PATH)
	@cp release-files/* $(DEPLOY_FILES_PATH)

	@echo --- Deploying files to target server.
	@echo $(DEPLOY_COMMAND)
	@pushd deploy-files; $(DEPLOY_COMMAND); popd

#
# Prepares a GreatFET nightly based on the bare source tree.
#
prepare_nightly: firmware
	@mkdir -p release-files/
	$(eval VERSION := $(shell date -I)-git-$(shell git rev-parse --short HEAD))
	echo $(VERSION)

	@echo --- Creating our host-python distribution directories
	@rm -rf host-packages
	@mkdir -p host-packages

	@#Python 2
	@pushd libgreat/host; $(PYTHON2) setup.py bdist_wheel -d $(CURDIR)/host-packages; popd
	@pushd host; $(PYTHON2) setup.py bdist_wheel -d $(CURDIR)/host-packages; popd

	@#Python 3
	@pushd libgreat/host; $(PYTHON3) setup.py bdist_wheel -d $(CURDIR)/host-packages; popd
	@pushd host; $(PYTHON3) setup.py bdist_wheel -d $(CURDIR)/host-packages; popd

	@echo --- Creating our firmware-binary directory.
	@# Extract the firmware-binaries from the assets folder we've produced.
	@rm -rf firmware-bin
	@cp -r host/greatfet/assets firmware-bin

	@# And remove the irreleveant README/.gitignore that have carried over from the assets folder.
	@rm firmware-bin/.gitignore
	@rm firmware-bin/README

	@echo --- Preparing the release archives.
	$(eval FIRMWARE_BIN_FLAGS := $(addprefix --extra=, $(wildcard firmware-bin/*)))
	$(eval HOST_PACKAGE_FLAGS := $(addprefix --extra=, $(wildcard host-packages/*)))
	@git-archive-all $(ARCHIVE_FLAGS_NIGHTLY) release-files/greatfet-$(VERSION).tar.xz
	@git-archive-all $(ARCHIVE_FLAGS_NIGHTLY) release-files/greatfet-$(VERSION).zip

	@echo --- Preparing the relevant hashes to enable deployment.
	@sha256sum release-files/greatfet-$(VERSION).tar.xz > release-files/greatfet-$(VERSION).tar.xz.sha256
	@sha256sum release-files/greatfet-$(VERSION).zip > release-files/greatfet-$(VERSION).zip.sha256

