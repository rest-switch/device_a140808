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


.PHONY: all
all: patch config

openwrt-master:
	@echo
	@echo fetching openwrt...
	curl -L https://github.com/rest-switch/openwrt/archive/master.tar.gz | tar xz

.PHONY: patch
patch: openwrt-master
	@echo
	@echo applying patch files to openwrt...
	cp -R files/. openwrt-master/

.PHONY: config
config: openwrt-master
	@echo 'CONFIG_TARGET_ramips=y' > 'openwrt-master/.config'
	@echo 'CONFIG_TARGET_ramips_rt305x=y' >> 'openwrt-master/.config'
	@echo 'CONFIG_TARGET_ramips_rt305x_HLK_RM04=y' >> 'openwrt-master/.config'
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
	grep -q $(1) 'openwrt-master/.config' || echo '\# $(1)' >> 'openwrt-master/.config'; \
	sed -i.old 's/\# $(1).*/$(1)=y/g' 'openwrt-master/.config';

cfg_disable = \
	@echo disabling: $(1); \
	grep -q $(1) 'openwrt-master/.config' || echo '$(1)=y' >> 'openwrt-master/.config'; \
	sed -i.old 's/$(1)=.*/\# $(1) is not set/g' 'openwrt-master/.config';
