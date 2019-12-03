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

# If we're building a nightly, use the provided build number.
BUILD_NUMBER ?= $(TRAVIS_BUILD_NUMBER)

# Used only for deploying nightlies -- only DEPLOY_COMMAND is used in the text below.
DEPLOY_USER    ?= deploy
DEPLOY_PATH    ?= ~/nightlies/greatfet
DEPLOY_COMMAND ?= scp -q -r * $(DEPLOY_USER)@$(DEPLOY_SERVER):$(DEPLOY_PATH)

# By default, if RELEASE_VERSION is set, use it as our version.
VERSION        ?= $(RELEASE_VERSION)

# Allow for easy specification of a version suffix, if desired.
ifdef VERSION_SUFFIX
VERSION_WITH_SUFFIX = "$(VERSION)-$(VERSION_SUFFIX)"
else
VERSION_WITH_SUFFIX = $(VERSION)
endif


all: firmware
.PHONY: all firmware install full_install install_and_flash menuconfig prepare_release prepare_release_archives \
	prepare_nightly versioning clean


# Flags for creating build archives.
# These effectively tell the release tool how to modify git-archive output to create a complete build.
ARCHIVE_FLAGS = \
	$(FIRMWARE_BIN_FLAGS) $(HOST_PACKAGE_FLAGS) \
	--force-submodules --prefix=greatfet-$(VERSION)/

#
# If we have a release version, also include the version and release-notes files.
#
ifdef RELEASE_VERSION
ARCHIVE_FLAGS += --extra=VERSION --extra=RELEASENOTES
endif


# Phony target that handles anything necessary for versioning.
versioning:
ifdef USE_NIGHTLY_VERSIONING
			$(eval VERSION := $(shell date -I)-build_$(BUILD_NUMBER)-git_$(shell git rev-parse --short HEAD))
			@echo "$(VERSION_WITH_SUFFIX)" > VERSION
			@echo "$(VERSION_WITH_SUFFIX)" > libgreat/VERSION
endif
ifdef RELEASE_VERSION
			@# Tag a version before we complete this build, if requested.
			@echo Tagging release $(VERSION).
			@git tag -a v$(VERSION) -m "release $(VERSION)" $(TAG_OPTIONS)
			@git -C libgreat tag -a v$(VERSION) -m "release $(VERSION)" $(TAG_OPTIONS)
			@echo "$(VERSION)" > VERSION
			@echo "$(VERSION)" > libgreat/VERSION
endif

#
# Convenience targets for our inner build system.
#
firmware: versioning

	# Create a firmware build directory, and configure our build.
	@mkdir -p firmware/build
	pushd firmware/build; cmake .. -DVERSION_LOCAL_SUFFIX=$(VERSION_SUFFIX); popd

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
prepare_release_files: firmware
	@mkdir -p release-files/

ifndef RELEASE_VERSION
	# If we don't have a version, create a nightly-style version.
	$(eval VERSION := $(shell date -I)-build_$(BUILD_NUMBER)-git_$(shell git rev-parse --short HEAD))
endif

	@echo --- Creating our host-python distribution directories
	@rm -rf host-packages
	@rm -rf build
	@mkdir -p host-packages
	@mkdir -p build

	@#Create our python pacakges. These universal packages work for py2/py3.
	@pushd libgreat/host; $(PYTHON3) setup.py bdist_wheel --universal -d $(CURDIR)/host-packages; popd
	@pushd host; $(PYTHON3) setup.py bdist_wheel --universal -d $(CURDIR)/host-packages; popd

	@# Create files for e.g. the nightly.
	@pushd libgreat/host; $(PYTHON3) setup.py bdist_wheel -p any -b build_dist --universal -d $(CURDIR)/release-files; popd
	@pushd host; $(PYTHON3) setup.py bdist_wheel -p any --universal -d $(CURDIR)/release-files; popd
	ls $(CURDIR)/release-files

	@echo --- Creating our firmware-binary directory.
	@# Extract the firmware-binaries from the assets folder we've produced.
	@rm -rf firmware-bin
	@cp -r host/greatfet/assets firmware-bin

	@# And remove the irreleveant README/.gitignore that have carried over from the assets folder.
	@rm firmware-bin/.gitignore
	@rm firmware-bin/README


# Split the second half of the preparation phase; as our wildcards need to be executed -after- the
# previous step.
prepare_release_archives: prepare_release_files
	@echo --- Preparing the release archives.
	$(eval FIRMWARE_BIN_FLAGS := $(addprefix --extra=, $(wildcard firmware-bin/*)))
	$(eval HOST_PACKAGE_FLAGS := $(addprefix --extra=, $(wildcard host-packages/*)))
	@git-archive-all $(ARCHIVE_FLAGS) release-files/greatfet-$(VERSION).tar.xz
	@git-archive-all $(ARCHIVE_FLAGS) release-files/greatfet-$(VERSION).zip

	@# Generate hash files.
	@echo --- Preparing the relevant hashes to enable distribution.
	@pushd release-files > /dev/null; sha256sum greatfet-$(VERSION).tar.xz > greatfet-$(VERSION).tar.xz.sha256; popd > /dev/null
	@pushd release-files > /dev/null; sha256sum greatfet-$(VERSION).zip > greatfet-$(VERSION).zip.sha256; popd > /dev/null




#
# prepare_release generates the actual release, and then prints instructions.
#
prepare_release: RELEASENOTES prepare_release_archives

	@# If no tag was supplied, warn the user.
ifndef RELEASE_VERSION
		$(warning Preapring a release without tagging a version -- this likely isn't what you want!)
endif

	@echo
	@echo Archives seem to be ready in ./release-files.
	@echo If everything seems okay, you probably should push the relevant tag:
	@echo "    git push origin v$(VERSION)"
	@echo "    git -C libgreat push origin v$(VERSION)"
	@echo
	@echo And push the relevant packages to Pypi:
	@echo "    python3 -m twine upload host-packages/*"


#
# prepare_nightly is mostly a convenience stub; but it may give us a place to hook things.
#
prepare_nightly: prepare_release_archives
	$(eval NIGHTLY_FILES := $(wildcard release-files/*))
	@echo --- Nightly prepared.


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
	@cp $(NIGHTLY_FILES) $(DEPLOY_FILES_PATH)

	@echo --- Deploying files to target server.
	@pushd deploy-files; $(DEPLOY_COMMAND); popd



#
# Pseudo-target for cleaning a build.
#
clean:

	# Temporary: ensure libopencm3 is cleaned.
	$(MAKE) -C firmware/libopencm3 -j$(nproc) clean

	# Clean our firmware build.
	$(MAKE) -C firmware/build -j$(nproc) clean

	# Clean out our created files and directories.
	rm -rf VERSION firmware-bin host-packages release-files distro-packages *.egg-info CMakeFiles
	rm -rf host/greatfet/assets/*.bin
