#
# Copyright 2015 The REST Switch Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, Licensor provides the Work (and each Contributor provides its 
# Contributions) on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied, including, 
# without limitation, any warranties or conditions of TITLE, NON-INFRINGEMENT, MERCHANTABILITY, or FITNESS FOR A PARTICULAR 
# PURPOSE. You are solely responsible for determining the appropriateness of using or redistributing the Work and assume any 
# risks associated with Your exercise of permissions under this License.
#
# Author: John Clark (johnc@restswitch.com)
#


OUTBIN           := bin
CFG_DIFF         := patches/diffconfig
PATCHES          := $(sort $(wildcard patches/*.patch))
UBOOT            := firmware/hlk-rm04_uboot-50000.bin
MAC              := $(strip $(mac))
MAC2BIN          := $(OUTBIN)/mac2bin
SERNUM           := $(OUTBIN)/serialnum
TARGET           := $(OUTBIN)/openwrt-ramips-rt305x-hlk-rm04-squashfs-sysupgrade.bin
LEDE_ROOT        := lede-project
LEDE_SRC_URL     := https://github.com/lede-project/source/archive/master.tar.gz
LEDE_FEEDS       := $(LEDE_ROOT)/feeds
LEDE_VER         := $(shell echo $$(($$(wget -q -O- https://api.github.com/repos/lede-project/source/commits\?since\=2016-07-16T00:00:00Z | grep \"commit\":\ { | wc -l) + 1003)))
LEDE_CFG         := $(LEDE_ROOT)/.config
LEDE_CACERT_SRC  := ca*certs/*_ca_public.cer
LEDE_CACERT_TGT  := $(LEDE_ROOT)/package/utils/a140808/files
LEDE_TGT         := $(LEDE_ROOT)/bin/targets/ramips/rt305x/openwrt-$(shell cat "$(LEDE_ROOT)/version")-ramips-rt305x-hlk-rm04-squashfs-sysupgrade.bin
SSH_TOOL         := tools/scripts/ssh_access.sh
FLASH_TOOL       := tools/scripts/program_flash.py
MINIPRO_ROOT     := tools/minipro
MINIPRO          := $(MINIPRO_ROOT)/minipro
MINIPRO_SRC_URL  := https://github.com/inindev/minipro/archive/master.tar.gz
MAC1A_ADDR       := $(shell echo -n $$((0x40004)))
MAC1B_ADDR       := $(shell echo -n $$((0x40028)))
MAC2_ADDR        := $(shell echo -n $$((0x4002e)))
SERIAL_ADDR      := $(shell echo -n $$((0x40400)))

.DEFAULT all: target

target: | config cacerts ssh_check
	$(MAKE) -C "$(LEDE_ROOT)" SOURCE_DATE_EPOCH=$$(date +%s)
	@if [ ! -d "$(OUTBIN)" ]; then mkdir "$(OUTBIN)"; fi
	@cp "$(LEDE_TGT)" "$(TARGET)"

image: | $(TARGET) $(SERNUM) $(MAC2BIN)
    ifeq ("","$(MAC)")
	@echo "********************************************************************************"
	@echo " error: Image target requires a MAC address to be specified: mac=aabbccddeeff"
	@echo "********************************************************************************"
	@exit 10
    endif

    ifneq (0,$(shell echo "$(MAC)" | grep -iq '^[0-9a-f]\{12\}$$'; echo $$?))
	@echo "********************************************************************************"
	@echo " error: The MAC address "$(MAC)" is invalid."
	@echo "        A valid MAC address must be 12 hex chars: mac=aabbccddeeff"
	@echo "********************************************************************************"
	@exit 12
    endif

	$(eval MAC2 := $(shell printf "%x" $$((0x$(MAC) + 1))))
	$(eval DEVID := $(shell $(SERNUM)))
	$(eval IMGFILE := a140808_$(DEVID).bin)
	@echo
	@echo "  building image file..."
	@echo "    Image File: $(IMGFILE)"
	@echo "     Device ID: $(DEVID)"
	@echo "         MAC 1: $(MAC)"
	@echo "         MAC 2: $(MAC2)"
	@echo
	@echo

	cp "$(UBOOT)" "$(OUTBIN)/$(IMGFILE)"
	# mac addresses
	"$(MAC2BIN)" "$(MAC)"  | dd bs=1 of="$(OUTBIN)/$(IMGFILE)" count=6 seek=$(MAC1A_ADDR) conv=notrunc
	"$(MAC2BIN)" "$(MAC)"  | dd bs=1 of="$(OUTBIN)/$(IMGFILE)" count=6 seek=$(MAC1B_ADDR) conv=notrunc
	"$(MAC2BIN)" "$(MAC2)" | dd bs=1 of="$(OUTBIN)/$(IMGFILE)" count=6 seek=$(MAC2_ADDR) conv=notrunc
	# serial number
	@echo -n $(DEVID) | dd bs=1 of="$(OUTBIN)/$(IMGFILE)" count=9 seek=$(SERIAL_ADDR) conv=notrunc
	cat "$(TARGET)" >> "$(OUTBIN)/$(IMGFILE)"
	# padding
	@tr '\000' '\377' < /dev/zero | dd bs=1 count=$$((0x400000 - $$(stat -c '%s' "$(OUTBIN)/$(IMGFILE)"))) >> "$(OUTBIN)/$(IMGFILE)"

program: minipro
	@python "$(FLASH_TOOL)" -p "$(strip $(flash))" -f "$(strip $(image))" 2>/dev/null || ( rc=$$?; \
	if [ "$$rc" -eq "2" ] ; then \
	    echo "********************************************************************************"; \
	    echo " error: Detected a flash memory other than MX25L3206E, PM25LQ032C, or W25Q32BV "; \
	    echo "        manually specify the flash memory name using: flash=xxxxxx"; \
	    echo "        see: minipro/devices.h for a complete list of flash names"; \
	    echo "    *** please report the new memory name to: johnc@restswitch.com ***"; \
	    echo "********************************************************************************"; \
	    exit 2; \
	elif [ "$$rc" -eq "3" ] ; then \
	    echo "********************************************************************************"; \
	    echo " error: The flash device must be either MX25L3206E, PM25LQ032C, or W25Q32BV"; \
	    echo "             eg: flash=MX25L3206E or flash=W25Q32BV"; \
	    echo "            see: minipro/devices.h for the complete list"; \
	    echo "********************************************************************************"; \
	    exit 3; \
	elif [ "$$rc" -eq "4" ] ; then \
	    echo "********************************************************************************"; \
	    echo " error: Invalid flash name specified. Check flash= setting and try again."; \
	    echo "            see: minipro/devices.h for the complete list"; \
	    echo "********************************************************************************"; \
	    exit 4; \
	elif [ "$$rc" -eq "5" ] ; then \
	    echo "********************************************************************************"; \
	    echo " error: Image file not found or not specified - eg: image=a140808_a????????.bin"; \
	    echo "********************************************************************************"; \
	    exit 5; \
	elif [ "$$rc" -eq "6" ] ; then \
	    echo "********************************************************************************"; \
	    echo " error: Image file incorrect size: \"$(strip $(image))\""; \
	    echo "********************************************************************************"; \
	    exit 6; \
	elif [ "$$rc" -eq "8" ] ; then \
	    echo "********************************************************************************"; \
	    echo " io error: Check Minipro and device connections then try again."; \
	    echo "********************************************************************************"; \
	    exit 8; \
	else \
	    echo "********************************************************************************"; \
	    echo " error: Programming failed with python script error $$rc"; \
	    echo "        Check connections and try again."; \
	    echo "        Debug using the script directly: tools/scripts/program_flash.py"; \
	    echo "********************************************************************************"; \
	    exit 9; \
	fi )

$(LEDE_ROOT):
	@echo
	@echo fetching lede project...
	rm -rf $(LEDE_ROOT) ; mkdir $(LEDE_ROOT)
	wget -O- "$(LEDE_SRC_URL)" | tar xz -C $(LEDE_ROOT) --strip-components 1
	echo "r$(LEDE_VER)" > "$(LEDE_ROOT)/version"

feeds: $(LEDE_FEEDS)
$(LEDE_FEEDS): $(LEDE_ROOT)
	@echo
	@echo applying feeds to lede project...
	"$(LEDE_ROOT)/scripts/feeds" update -a
	"$(LEDE_ROOT)/scripts/feeds" install -a
	@rm -f "$(LEDE_CFG)" # remove .config file that the feeds gen

patch: feeds
	@echo
	@echo applying a140808 patches to lede project...
	$(foreach patch,$(PATCHES),if patch --dry-run -N -p1 -d "$(LEDE_ROOT)" < "$(patch)" 2>&1 >/dev/null; then patch -p1 -d "$(LEDE_ROOT)" < "$(patch)"; fi;)
	cp -R "files/." "$(LEDE_ROOT)"

cacerts: patch
    ifneq ($(wildcard $(LEDE_CACERT_SRC)),)
	@echo copying CA certificate files...
	@mkdir -p "$(LEDE_CACERT_TGT)"
	@$(foreach ca_file,$(wildcard $(LEDE_CACERT_SRC)),cp $(ca_file) "$(LEDE_CACERT_TGT)/$(basename $(notdir $(ca_file))).pem";)
    else
	@echo no CA certificate files present
    endif

config: $(LEDE_CFG)
$(LEDE_CFG): patch
	@echo
	@echo generating lede project .config file...
	cat "$(CFG_DIFF)" > "$(LEDE_CFG)"
	$(MAKE) -C "$(LEDE_ROOT)" defconfig

$(SERNUM):
	$(MAKE) -C "tools/serialnum"
	mv "tools/serialnum/serialnum" "$(OUTBIN)"

$(MAC2BIN):
	$(MAKE) -C "tools/mac2bin"
	mv "tools/mac2bin/mac2bin" "$(OUTBIN)"

minipro: $(MINIPRO)
$(MINIPRO):
	@if [ ! -d "$(MINIPRO_ROOT)" ]; then \
		echo; \
		echo "fetching minipro source...";\
		mkdir -p "$(MINIPRO_ROOT)"; \
		wget -O- $(MINIPRO_SRC_URL) | tar --strip-components=1 -xzvC "$(MINIPRO_ROOT)"; \
	fi
	$(MAKE) -C "$(MINIPRO_ROOT)"

clean distclean:
	$(MAKE) -C "tools/serialnum" clean
	$(MAKE) -C "tools/mac2bin" clean
	@if [ -d "$(LEDE_ROOT)" ]; then $(MAKE) -C "$(LEDE_ROOT)" $@; fi
	rm -rf "$(OUTBIN)"

ifeq ("none","$(strip $(ssh))")
   ssh_check: ssh_none
else
   ssh_check: ssh_safe
endif

ssh_none:
	@"$(SSH_TOOL)" "-n"

ssh_cert:
   ifneq ("","$(strip $(sshcert))")
	@"$(SSH_TOOL)" "-c" "$(strip $(sshcert))"
   endif

ssh_pw:
   ifneq ("","$(strip $(sshpw))")
	@"$(SSH_TOOL)" "-p" "$(strip $(sshpw))"
   endif

ssh_safe: ssh_pw ssh_cert
	@"$(SSH_TOOL)" "-a" || ( \
	echo "********************************************************************************"; \
	echo " error: No root password or cert file specified."; \
	echo "        ssh access will not be possible without a root password or"; \
	echo "        RSA certificate file. Specify "make ssh=none" if this is desired."; \
	echo ""; \
	echo "        options"; \
	echo "        ---------"; \
	echo "          no ssh access:"; \
	echo "            ssh=none"; \
	echo "          no ssh password access:"; \
	echo "            sshpw=none"; \
	echo "          ssh password access (prompt):"; \
	echo "            sshpw=pmt"; \
	echo "          ssh password access:"; \
	echo "            sshpw=<password>"; \
	echo "          no ssh certificate access:"; \
	echo "            sshcert=none"; \
	echo "          ssh certificate access, create new cert:"; \
	echo "            sshcert=gen"; \
	echo "          ssh certificate access, use existing cert:"; \
	echo "            sshcert=<filespec>"; \
	echo ""; \
	echo "********************************************************************************"; \
	exit 14 )

.PHONY: all target image program feeds patch cacerts config minipro clean distclean ssh_none ssh_cert ssh_pw ssh_safe

