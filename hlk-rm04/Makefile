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

OWRT_ROOT        := openwrt-master
OWRT_SRC_URL     := https://github.com/rest-switch/openwrt/archive/master.tar.gz
OWRT_CFG         := $(OWRT_ROOT)/.config
OWRT_FEEDS       := $(OWRT_ROOT)/feeds
OWRT_VER         := $(OWRT_ROOT)/version
OWRT_CACERT_SRC  := ca*certs/*_ca_public.cer
OWRT_CACERT_TGT  := $(OWRT_ROOT)/package/system/a140808/files
OWRT_TGT         := $(OWRT_ROOT)/bin/ramips/openwrt-ramips-rt305x-hlk-rm04-squashfs-sysupgrade.bin
SSH_TOOL         := tools/scripts/ssh_access.sh
UBOOT            := firmware/hlk-rm04_uboot-50000.bin
GEN_CFG          := tools/scripts/gen_config.sh
OUTBIN           := bin
SERNUM           := $(OUTBIN)/serialnum
MAC              := $(strip $(mac))
MAC2BIN          := $(OUTBIN)/mac2bin
TARGET           := $(OUTBIN)/openwrt-ramips-rt305x-hlk-rm04-squashfs-sysupgrade.bin
MINIPRO_ROOT     := tools/minipro
MINIPRO          := $(MINIPRO_ROOT)/minipro
MINIPRO_SRC_URL  := https://github.com/inindev/minipro/archive/master.tar.gz
#MINIPRO_SRC_URL  := https://github.com/vdudouyt/minipro/archive/master.tar.gz


.DEFAULT all: target

target: config cacerts ssh_check
	$(MAKE) -C "$(OWRT_ROOT)"
	@if [ ! -d "$(OUTBIN)" ]; then mkdir "$(OUTBIN)"; fi
	@cp "$(OWRT_TGT)" "$(OUTBIN)"

image: | $(TARGET) $(SERNUM) $(MAC2BIN)
    ifeq ("","$(MAC)")
	@echo "********************************************************************************"
	@echo " error: Image target requires a MAC address to be specified: mac=aabbccddeeff"
	@echo "********************************************************************************"
	exit 10
    endif

    ifneq (0,$(shell echo "$(MAC)" | grep -iq '[0-9a-f]\{12\}'; echo $$?))
	@echo "********************************************************************************"
	@echo " error: The MAC address "$(MAC)" is invalid."
	@echo "        A valid MAC address must be 12 hex chars: mac=aabbccddeeff"
	@echo "********************************************************************************"
	exit 12
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
	"$(MAC2BIN)" "$(MAC)"  | dd bs=1 of="$(OUTBIN)/$(IMGFILE)" count=6 seek=262148 conv=notrunc
	"$(MAC2BIN)" "$(MAC)"  | dd bs=1 of="$(OUTBIN)/$(IMGFILE)" count=6 seek=262184 conv=notrunc
	"$(MAC2BIN)" "$(MAC2)" | dd bs=1 of="$(OUTBIN)/$(IMGFILE)" count=6 seek=262190 conv=notrunc
	# serial number
	@echo -n $(DEVID) | dd bs=1 of="$(OUTBIN)/$(IMGFILE)" count=9 seek=262407 conv=notrunc
	cat "$(TARGET)" >> "$(OUTBIN)/$(IMGFILE)"
	# padding
	@tr '\000' '\377' < /dev/zero | dd bs=1 count=$$((0x400000 - $$(stat -c '%s' "$(OUTBIN)/$(IMGFILE)"))) >> "$(OUTBIN)/$(IMGFILE)"

program: minipro
   ifneq ("","$(strip $(sshcert))")
	@echo "********************************************************************************"
	@echo " error: Flash device not specified: flash=W25Q32BV"
	@echo "********************************************************************************"
	exit 14
   endif
     ifeq ("","$(filter $(strip $(flash)), MX25L3206E W25Q32BV)")
	@echo "********************************************************************************"
	@echo " error: The flash device must be either MX25L3206E or W25Q32BV"
	@echo "          i.e.  flash=MX25L3206E or flash=W25Q32BV"
	@echo "********************************************************************************"
	exit 14
     endif

   ifeq ("","$(strip $(image))")
	@echo "********************************************************************************"
	@echo " error: Image file not specified: image=<a140808.bin>"
	@echo "********************************************************************************"
	exit 16
   else
     ifeq ("", "$(shell find '$(strip $(image))' -type f -size 4M)")
	@echo "********************************************************************************"
	@echo " error: Image file \"$(strip $(image))\""
	@echo "        not found or incorrect size."
	@echo "********************************************************************************"
	exit 16
     endif
   endif
	@echo writing image file "$(strip $(image))" to device "$(strip $(flash))"
	"$(MINIPRO)" -p "$(strip $(flash))" -w "$(strip $(image))"

$(OWRT_ROOT):
	@echo
	@echo fetching openwrt...
	wget -O- "$(OWRT_SRC_URL)" | tar xz

feeds: | $(OWRT_ROOT) $(OWRT_FEEDS)
$(OWRT_FEEDS):
	@echo
	@echo applying feeds to openwrt...
	"$(OWRT_ROOT)/scripts/feeds" update -a
	"$(OWRT_ROOT)/scripts/feeds" install -a
	@rm -f "$(OWRT_CFG)" # remove .config file that the feeds gen

patch: | $(OWRT_ROOT) $(OWRT_VER)
$(OWRT_VER):
	@echo
	@echo applying a140808 updates to openwrt...
	cp -R "files/." "$(OWRT_ROOT)"

cacerts: | patch
    ifneq ($(wildcard $(OWRT_CACERT_SRC)),)
	@echo copying CA certificate files...
	@mkdir -p "$(OWRT_CACERT_TGT)"
	@$(foreach ca_file,$(wildcard $(OWRT_CACERT_SRC)),cp $(ca_file) "$(OWRT_CACERT_TGT)/$(basename $(notdir $(ca_file))).pem";)
    else
	@echo no CA certificate files present
    endif

config: | feeds patch $(OWRT_CFG)
$(OWRT_CFG):
	@echo
	@echo generating openwrt .config file...
	"$(GEN_CFG)"

$(SERNUM):
	$(MAKE) -C "tools/serialnum"
	mv "tools/serialnum/serialnum" "$(OUTBIN)"

$(MAC2BIN):
	$(MAKE) -C "tools/mac2bin"
	mv "tools/mac2bin/mac2bin" "$(OUTBIN)"

.PHONY: minipro
minipro: | $(MINIPRO)
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
	@if [ -d "$(OWRT_ROOT)" ]; then $(MAKE) -C "$(OWRT_ROOT)" $@; fi
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
	echo "********************************************************************************" &&\
	echo " error: No root password or cert file specified." &&\
	echo "        ssh access will not be possible without a root password or" &&\
	echo "        RSA certificate file. Specify "make ssh=none" if this is desired." &&\
	echo &&\
	echo "        options" &&\
	echo "        ---------" &&\
	echo "          no ssh access:" &&\
	echo "            ssh=none" &&\
	echo "          no ssh password access:" &&\
	echo "            sshpw=none" &&\
	echo "          ssh password access (prompt):" &&\
	echo "            sshpw=pmt" &&\
	echo "          ssh password access:" &&\
	echo "            sshpw=<password>" &&\
	echo "          no ssh certificate access:" &&\
	echo "            sshcert=none" &&\
	echo "          ssh certificate access, create new cert:" &&\
	echo "            sshcert=gen" &&\
	echo "          ssh certificate access, use existing cert:" &&\
	echo "            sshcert=<filespec>" &&\
	echo &&\
	echo "********************************************************************************" &&\
	exit 18 )

.PHONY: all target image program feeds patch config minipro clean distclean ssh_none ssh_cert ssh_pw ssh_safe

