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

OUTBIN     := bin
OWRT_ROOT  := openwrt-master
OWRT_CFG   := $(OWRT_ROOT)/.config
OWRT_FEEDS := $(OWRT_ROOT)/feeds
PWFILE     := $(OWRT_ROOT)/package/base-files/files/etc/shadow
UBOOT      := firmware/hlk-rm04_uboot-50000.bin
SERNUM     := $(OUTBIN)/serialnum
MAC2BIN    := $(OUTBIN)/mac2bin
TARGET     := $(OUTBIN)/openwrt-ramips-rt305x-hlk-rm04-squashfs-sysupgrade.bin

OWRT_SRC_URL := https://github.com/rest-switch/openwrt/archive/master.tar.gz

# sanitize inputs
MAC    :=  $(strip $(mac))
ROOTPW := "$(strip $(rootpw))"


all: $(TARGET)

$(TARGET): $(OWRT_FEEDS) patch $(OWRT_CFG)
   ifneq ($(ROOTPW),"")
     ifneq (0,$(shell pwhash=$$(openssl passwd -1 "$(ROOTPW)") && awk 'BEGIN {OFS=FS=":"} $$1=="root" {$$2="'"$${pwhash}"'"} {print}' $(PWFILE) > $(PWFILE).tmp && mv $(PWFILE).tmp $(PWFILE); echo $$?))
	$(error error: Failed to set the password for the root user.)
     else
	@echo "root password set"
     endif
   endif

	make -C $(OWRT_ROOT)
	@test -d $(OUTBIN) || mkdir $(OUTBIN)
	@cp $(OWRT_ROOT)/bin/ramips/openwrt-ramips-rt305x-hlk-rm04-squashfs-sysupgrade.bin $(OUTBIN)

image: $(TARGET) $(SERNUM) $(MAC2BIN)
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
	$(MAC2BIN) $(MAC)  | dd bs=1 of=$(OUTBIN)/$(IMGFILE) count=6 seek=262148 conv=notrunc
	$(MAC2BIN) $(MAC)  | dd bs=1 of=$(OUTBIN)/$(IMGFILE) count=6 seek=262184 conv=notrunc
	$(MAC2BIN) $(MAC2) | dd bs=1 of=$(OUTBIN)/$(IMGFILE) count=6 seek=262190 conv=notrunc
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
	curl -L $(OWRT_SRC_URL) | tar xz

patch: $(OWRT_ROOT)
	@echo
	@echo applying a140808 updates to openwrt...
	cp -R files/. $(OWRT_ROOT)

$(OWRT_FEEDS): $(OWRT_ROOT)
	@echo
	@echo applying feeds to openwrt...
	$(OWRT_ROOT)/scripts/feeds update -a
	$(OWRT_ROOT)/scripts/feeds install -a

clean:
	make -C $(OWRT_ROOT) clean
	make -C src/util/serialnum clean
	make -C src/util/mac2bin clean
	rm -rf $(OUTBIN) $(OWRT_CFG)

distclean:
	make -C src/util/serialnum clean
	make -C src/util/mac2bin clean
	rm -rf $(OUTBIN) $(OWRT_ROOT)

$(OWRT_CFG): $(OWRT_ROOT)
	$(call cfg_enable,'CONFIG_TARGET_ramips')
	$(call cfg_enable,'CONFIG_TARGET_ramips_rt305x')
	$(call cfg_enable,'CONFIG_TARGET_ramips_rt305x_HLK_RM04')
	make -C openwrt-master defconfig

	$(call cfg_enable,'CONFIG_PACKAGE_a140808')
	$(call cfg_disable,'CONFIG_PACKAGE_firewall')
	$(call cfg_disable,'CONFIG_PACKAGE_iptables')
	$(call cfg_disable,'CONFIG_PACKAGE_opkg')
	$(call cfg_disable,'CONFIG_PACKAGE_ppp')
	$(call cfg_enable,'CONFIG_PACKAGE_nano')
#	$(call cfg_enable,'CONFIG_PACKAGE_uhttpd')

	# new items
	$(call cfg_disable,'CONFIG_PACKAGE_iptables-snmp')
	$(call cfg_disable,'CONFIG_PACKAGE_libwebsockets-examples')
	$(call cfg_disable,'CONFIG_PACKAGE_ppp-multilink')
	$(call cfg_disable,'CONFIG_PACKAGE_uhttpd_debug')
	$(call cfg_disable,'CONFIG_PACKAGE_uhttpd-mod-lua')
	$(call cfg_disable,'CONFIG_PACKAGE_uhttpd-mod-tls')
	$(call cfg_disable,'CONFIG_PACKAGE_uhttpd-mod-ubus')

	make -C openwrt-master oldconfig

cfg_enable = \
	@echo enabling: $(1); \
	grep -q $(1) $(OWRT_CFG) || echo '\# $(1)' >> $(OWRT_CFG); \
	sed -i.old 's/\# $(1).*/$(1)=y/g' $(OWRT_CFG);

cfg_disable = \
	@echo disabling: $(1); \
	grep -q $(1) $(OWRT_CFG) || echo '$(1)=y' >> $(OWRT_CFG); \
	sed -i.old 's/$(1)=.*/\# $(1) is not set/g' $(OWRT_CFG);

.PHONY: all target patch config clean distclean
