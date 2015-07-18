#
# Copyright 2015 The REST Switch Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http:#www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, Licensor provides the Work (and each Contributor provides its 
# Contributions) on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied, including, 
# without limitation, any warranties or conditions of TITLE, NON-INFRINGEMENT, MERCHANTABILITY, or FITNESS FOR A PARTICULAR 
# PURPOSE. You are solely responsible for determining the appropriateness of using or redistributing the Work and assume any 
# risks associated with Your exercise of permissions under this License.
#
# Author: John Clark (johnc@restswitch.com)
#

OWRT_SRC_URL := https://github.com/rest-switch/openwrt/archive/master.tar.gz
OWRT_ROOT    := openwrt-master
OWRT_CFG     := $(OWRT_ROOT)/.config
OWRT_FEEDS   := $(OWRT_ROOT)/feeds
OWRT_VER     := $(OWRT_ROOT)/version
OWRT_TGT     := $(OWRT_ROOT)/bin/ramips/openwrt-ramips-rt305x-hlk-rm04-squashfs-sysupgrade.bin
PWFILE       := $(OWRT_ROOT)/package/base-files/files/etc/shadow
GEN_CFG      := src/util/scripts/gen_config.sh
UBOOT        := firmware/hlk-rm04_uboot-50000.bin
OUTBIN       := bin
SERNUM       := $(OUTBIN)/serialnum
MAC2BIN      := $(OUTBIN)/mac2bin
TARGET       := $(OUTBIN)/openwrt-ramips-rt305x-hlk-rm04-squashfs-sysupgrade.bin


# sanitize inputs
MAC    :=  $(strip $(mac))
ROOTPW := "$(strip $(rootpw))"


all: target

target: feeds patch config
   ifneq ($(ROOTPW),"")
     ifneq (0,$(shell pwhash=$$(openssl passwd -1 "$(ROOTPW)") && awk 'BEGIN {OFS=FS=":"} $$1=="root" {$$2="'"$${pwhash}"'"} {print}' $(PWFILE) > $(PWFILE).tmp && mv $(PWFILE).tmp $(PWFILE); echo $$?))
	$(error error: Failed to set the password for the root user.)
     else
	@echo "root password set"
     endif
   endif

	make -C $(OWRT_ROOT)
	@test -d $(OUTBIN) || mkdir $(OUTBIN)
	@cp $(OWRT_TGT) $(OUTBIN)

image: | $(TARGET) $(SERNUM) $(MAC2BIN)
    ifeq ($(MAC),"")
	$(error error: Image target requires a MAC address to be specified: mac=aabbccddeeff)
    endif

    ifneq (0,$(shell echo "$(MAC)" | grep -iq '[0-9a-f]\{12\}'; echo $$?))
	$(error error: The MAC address "$(MAC)" is invalid. A valid MAC address must be 12 hex chars: mac=aabbccddeeff)
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

	cp $(UBOOT) $(OUTBIN)/$(IMGFILE)
	# mac addresses
	$(MAC2BIN) $(MAC)  | dd bs=1 of=$(OUTBIN)/$(IMGFILE) count=6 seek=262148 conv=notrunc
	$(MAC2BIN) $(MAC)  | dd bs=1 of=$(OUTBIN)/$(IMGFILE) count=6 seek=262184 conv=notrunc
	$(MAC2BIN) $(MAC2) | dd bs=1 of=$(OUTBIN)/$(IMGFILE) count=6 seek=262190 conv=notrunc
	# serial number
	@echo -n $(DEVID)  | dd bs=1 of=$(OUTBIN)/$(IMGFILE) count=9 seek=262407 conv=notrunc
	cat $(TARGET) >> $(OUTBIN)/$(IMGFILE)

$(SERNUM):
	make -C src/util/serialnum
	mv src/util/serialnum/serialnum $(OUTBIN)

$(MAC2BIN):
	make -C src/util/mac2bin
	mv src/util/mac2bin/mac2bin $(OUTBIN)

$(OWRT_ROOT):
	@echo
	@echo fetching openwrt...
	wget -O- $(OWRT_SRC_URL) | tar xz

feeds: | $(OWRT_ROOT) $(OWRT_FEEDS)
$(OWRT_FEEDS):
	@echo
	@echo applying feeds to openwrt...
	$(OWRT_ROOT)/scripts/feeds update -a
	$(OWRT_ROOT)/scripts/feeds install -a

patch: | $(OWRT_ROOT) $(OWRT_VER)
$(OWRT_VER):
	@echo
	@echo applying a140808 updates to openwrt...
	cp -R files/. $(OWRT_ROOT)

config: | $(OWRT_ROOT) $(OWRT_CFG)
$(OWRT_CFG):
	@echo
	@echo generating openwrt .config file...
	$(GEN_CFG)

clean:
	make -C $(OWRT_ROOT) clean
	make -C src/util/serialnum clean
	make -C src/util/mac2bin clean
	rm -rf $(OUTBIN) $(OWRT_CFG)

distclean:
	make -C src/util/serialnum clean
	make -C src/util/mac2bin clean
	rm -rf $(OUTBIN) $(OWRT_ROOT)

.PHONY: all target image feeds patch config clean distclean
