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

#
# a140808 websockets client application
#

include $(TOPDIR)/rules.mk

PKG_NAME:=a140808
PKG_VERSION:=0.8
PKG_RELEASE:=1

# PKG_SOURCE:=a140808-src-$(PKG_VERSION).tar.gz
# PKG_SOURCE_URL:=https://github.com/rest-switch/device_a140808
# PKG_MD5SUM:=41b0a9bca945b4ae2608421afd8f606e

include $(INCLUDE_DIR)/package.mk

define Package/a140808
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=a140808 websockets client application
	URL:=http://www.rest-switch.com
	DEPENDS:=+libwebsockets-cyassl +libjson-c
endef

define Package/a140808/description
	a140808 is a websocket client for the hlk-rm04 based device
endef

define Build/Prepare
	$(INSTALL_DIR) $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/a140808/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/a140808 $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/cloudcfg.config $(1)/etc/config/cloudcfg
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/a140808.init $(1)/etc/init.d/a140808
	$(INSTALL_DIR) $(1)/etc/ssl/certs
    ifneq ($(wildcard ./files/*.pem),)
	$(INSTALL_DATA) ./files/*.pem $(1)/etc/ssl/certs/
	$(foreach pem_file,$(wildcard ./files/*.pem),ln -sf $(notdir $(pem_file)) $(1)/etc/ssl/certs/$(shell openssl x509 -hash -noout -in $(pem_file)).0;)
    endif
endef

$(eval $(call BuildPackage,a140808))
