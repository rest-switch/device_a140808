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
PW_FILE      := $(OWRT_ROOT)/package/base-files/files/etc/shadow
SSH_PVT      := a140808_rsa.private
SSH_PUB      := $(OWRT_ROOT)/package/network/services/dropbear/files/authorized_keys
GEN_CFG      := src/util/scripts/gen_config.sh
UBOOT        := firmware/hlk-rm04_uboot-50000.bin
OUTBIN       := bin
SERNUM       := $(OUTBIN)/serialnum
MAC2BIN      := $(OUTBIN)/mac2bin
TARGET       := $(OUTBIN)/openwrt-ramips-rt305x-hlk-rm04-squashfs-sysupgrade.bin
MAC          := "$(strip $(mac))"


.DEFAULT all: target

target: config ssh_check
	$(MAKE) -C "$(OWRT_ROOT)"
	@test -d "$(OUTBIN)" || mkdir "$(OUTBIN)"
	@cp "$(OWRT_TGT)" "$(OUTBIN)"

image: | $(TARGET) $(SERNUM) $(MAC2BIN)
    ifeq ("","$(MAC)")
	@echo "********************************************************************************"
	@echo " error: Image target requires a MAC address to be specified: mac=aabbccddeeff"
	@echo "********************************************************************************"
	exit 11
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
	$"(MAC2BIN)" "$(MAC)"  | dd bs=1 of="$(OUTBIN)/$(IMGFILE)" count=6 seek=262148 conv=notrunc
	$"(MAC2BIN)" "$(MAC)"  | dd bs=1 of="$(OUTBIN)/$(IMGFILE)" count=6 seek=262184 conv=notrunc
	$"(MAC2BIN)" "$(MAC2)" | dd bs=1 of="$(OUTBIN)/$(IMGFILE)" count=6 seek=262190 conv=notrunc
	# serial number
	@echo -n $(DEVID)  | dd bs=1 of="$(OUTBIN)/$(IMGFILE)" count=9 seek=262407 conv=notrunc
	cat "$(TARGET)" >> "$(OUTBIN)/$(IMGFILE)"

$(SERNUM):
	$(MAKE) -C "src/util/serialnum"
	mv "src/util/serialnum/serialnum" "$(OUTBIN)"

$(MAC2BIN):
	$(MAKE) -C "src/util/mac2bin"
	mv "src/util/mac2bin/mac2bin" "$(OUTBIN)"

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
	rm "$(OWRT_CFG)" # remove .config file that the feeds gen

patch: | $(OWRT_ROOT) $(OWRT_VER)
$(OWRT_VER):
	@echo
	@echo applying a140808 updates to openwrt...
	cp -R "files/." "$(OWRT_ROOT)"

config: | feeds patch $(OWRT_CFG)
$(OWRT_CFG):
	@echo
	@echo generating openwrt .config file...
	"$(GEN_CFG)"

clean:
	$(MAKE) -C "$(OWRT_ROOT)" clean
	$(MAKE) -C "src/util/serialnum" clean
	$(MAKE) -C "src/util/mac2bin" clean
	rm -rf "$(OUTBIN)" "$(OWRT_CFG)"

distclean:
	$(MAKE) -C "src/util/serialnum" clean
	$(MAKE) -C "src/util/mac2bin" clean
	rm -rf "$(OUTBIN)" "$(OWRT_ROOT)"

ifneq ("none","$(strip $(ssh))")
   ssh_check: ssh_safe
else
   ssh_check: ssh_none
endif

ssh_none:
   # no access: ssh=none
   ifeq ("none","$(strip $(ssh))")
	@echo "disabling ssh access..."

     # disable password access
     ifneq (0,$(shell awk 'BEGIN {OFS=FS=":"} $$1=="root" {$$2="x"} {print}' "$(PW_FILE)" > "$(PW_FILE).tmp" && mv "$(PW_FILE).tmp" "$(PW_FILE)"; echo $$?))
	@echo "********************************************************************************"
	@echo " error: Failed to clear the password for the root user"
	@echo "********************************************************************************"
	exit 13
     endif

	# disable certificate based auth
	umask 077; touch "$(SSH_PUB).tmp"
	mv "$(SSH_PUB).tmp" "$(SSH_PUB)"

	@echo "password and certificates based ssh authentication are now disabled"
   endif

ssh_pw:
   # ssh password: sshpw=none | <password>
   ifneq ("","$(strip $(sshpw))")
     ifeq ("none","$(strip $(sshpw))")
       ifneq (0,$(shell awk 'BEGIN {OFS=FS=":"} $$1=="root" {$$2="x"} {print}' "$(PW_FILE)" > "$(PW_FILE).tmp" && mv "$(PW_FILE).tmp" "$(PW_FILE)"; echo $$?))
		@echo "********************************************************************************"
		@echo " error: Failed to clear the password for the root user"
		@echo "********************************************************************************"
		exit 13
       else
		@echo "password based ssh authentication now disabled"
       endif
     else ifneq (0,$(shell pwhash=$$(openssl passwd -1 "$(strip $(sshpw))") && awk 'BEGIN {OFS=FS=":"} $$1=="root" {$$2="'"$${pwhash}"'"} {print}' "$(PW_FILE)" > "$(PW_FILE).tmp" && mv "$(PW_FILE).tmp" "$(PW_FILE)"; echo $$?))
		@echo "********************************************************************************"
		@echo " error: Failed to set the password for the root user"
		@echo "********************************************************************************"
		exit 14
     else
		@echo "root password set"
     endif
   endif

ssh_cert:
   # ssh cert: sshcert=none | gen | <filespec>
   ifneq ("","$(strip $(sshcert))")
     ifeq ("none","$(strip $(sshcert))")
	# remove the public key
	@umask 077; touch "$(SSH_PUB).tmp"
	@mv "$(SSH_PUB).tmp" "$(SSH_PUB)"
	@echo "certificates based ssh authentication now disabled"
     else ifeq ("gen","$(strip $(sshcert))")
	# private key
	@echo "generating 4096 bit rsa ssh certificate..."
	@test -e "$(SSH_PVT)" && rm "$(SSH_PVT)" || true
	ssh-keygen -q -N "" -t rsa -b 4096 -f "$(SSH_PVT)"
	@test -e "$(SSH_PVT).pub" && rm "$(SSH_PVT).pub" || true
	# public key
	@umask 077; touch "$(SSH_PUB).tmp"
	ssh-keygen -y -f "$(SSH_PVT)" > "$(SSH_PUB).tmp"
	@mv "$(SSH_PUB).tmp" "$(SSH_PUB)"
	@echo "ssh access enabled for certificate: $(SSH_PVT)"
     else ifneq (0,$(shell test $$(stat -c '%s' "$(strip $(sshcert))") -gt "1678"; echo $$?))
	@echo "********************************************************************************"
	@echo " error: Invalid ssh private key."
	@echo "        Specify the path to an RSA private of at least 2048 bits)"
	@echo "********************************************************************************"
	exit 15
     else
	@echo "using existing rsa ssh certificate..."
	# public key
	umask 077; touch "$(SSH_PUB).tmp"
	ssh-keygen -y -f "$(strip $(sshcert))" > "$(SSH_PUB).tmp"
	mv "$(SSH_PUB).tmp" "$(SSH_PUB)"
	@echo "ssh access enabled for private access key: $(strip $(sshcert))"
     endif
   endif

ssh_safe: ssh_pw ssh_cert
	# at this point either password or cert auth should be available as it was not specifically denied
	@grep -iq '^root:.\?:' "$(PW_FILE)" && \
	test "$$(stat -c '%s' "$(SSH_PUB)" 2> /dev/null)" -lt "381"; \
	if [ $$? -eq 0 ]; then \
		echo "********************************************************************************"; \
		echo " error: No root password or cert file specified."; \
		echo "        ssh access will not be possible without a root password or"; \
		echo "        RSA certificate file. Specify "make ssh=none" if this is desired."; \
		echo; \
		echo "        options"; \
		echo "        ---------"; \
		echo "          no ssh access:"; \
		echo "            ssh=none"; \
		echo "          no ssh password access:"; \
		echo "            sshpw=none"; \
		echo "          ssh password access:"; \
		echo "            sshpw=<password>"; \
		echo "          no ssh certificate access:"; \
		echo "            sshcert=none"; \
		echo "          ssh certificate access, create new cert:"; \
		echo "            sshcert=gen"; \
		echo "          ssh certificate access, use existing cert:"; \
		echo "            sshcert=<filespec>"; \
		echo; \
		echo "********************************************************************************"; \
		exit 16; \
	fi

.PHONY: all target image feeds patch config clean distclean ssh_check ssh_none ssh_pw ssh_cert ssh_safe

