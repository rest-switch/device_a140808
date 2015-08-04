#!/bin/sh
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


MYFILE=$(readlink -f "$0")
MYDIR=$(dirname "${MYFILE}")
OWRT_ROOT="${MYDIR}/../../openwrt-master"
OWRT_CFG="${OWRT_ROOT}/.config"


configure() {
    #
    # setup for the hlk-rm04
    # make menuconfig
    #     Base system
    #         +a140808
    #         -firewall
    #         -opkg
    #         ?wireless-tools
    #     Libraries
    #         SSL
    #             +libcyassl
    #         +libwebsockets
    #     Network
    #         -ppp (-ppp-mod-ppoe)
    #         Firewall
    #             -iptables
    #     Utilities
    #         Editors
    #             +nano
    #
    echo 'CONFIG_TARGET_ramips=y' > "${OWRT_CFG}"
    echo 'CONFIG_TARGET_ramips_rt305x=y' >> "${OWRT_CFG}"
    echo 'CONFIG_TARGET_ramips_rt305x_HLK_RM04=y' >> "${OWRT_CFG}"
    make -C "${OWRT_ROOT}" defconfig || error "$?" "make defconfig failed"

    cfg_enable  'CONFIG_PACKAGE_a140808'
    cfg_disable 'CONFIG_PACKAGE_firewall'
    cfg_disable 'CONFIG_PACKAGE_iptables'
    cfg_disable 'CONFIG_PACKAGE_opkg'
    cfg_disable 'CONFIG_PACKAGE_ppp'
    cfg_enable  'CONFIG_PACKAGE_nano'
#    cfg_enable  'CONFIG_PACKAGE_uhttpd'

    # new items
    cfg_disable 'CONFIG_PACKAGE_iptables-snmp'
    cfg_disable 'CONFIG_PACKAGE_libwebsockets-examples'
    cfg_disable 'CONFIG_PACKAGE_ppp-multilink'
    cfg_disable 'CONFIG_PACKAGE_uhttpd_debug'
    cfg_disable 'CONFIG_PACKAGE_uhttpd-mod-lua'
    cfg_disable 'CONFIG_PACKAGE_uhttpd-mod-tls'
    cfg_disable 'CONFIG_PACKAGE_uhttpd-mod-ubus'

    make -C "${OWRT_ROOT}" oldconfig || error "$?" "make oldconfig failed"
}


cfg_enable() {
    echo "enabling: $1"
    grep -q "$1" "${OWRT_CFG}" || echo "$1" >> "${OWRT_CFG}"
    sed -i "s/.*$1.*/$1=y/g" "${OWRT_CFG}"
}

cfg_disable() {
    echo "disabling: $1"
    grep -q "$1" "${OWRT_CFG}" || echo "$1" >> "${OWRT_CFG}"
    sed -i "s/.*$1.*/# $1 is not set/g" "${OWRT_CFG}"
}

cfg_inject() {
    echo "injecting: $2 after $1"
    sed -i "s/\(.*$1.*\)/\1\n$2/g" "${OWRT_CFG}"
}

error() {
    echo "  ***"
    echo "  ***  ERROR $1: $(basename $0) - $2"
    echo "  ***"
    exit 1
}

configure

