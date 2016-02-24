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
MAC2BIN="${MYDIR}/../../bin/mac2bin"
GENSERIAL="${MYDIR}/../../bin/serialnum"
MAC1A_ADDR=$((0x40004))
MAC1B_ADDR=$((0x40028))
MAC2_ADDR=$((0x4002e))
SERIAL_ADDR=$((0x40400))


#
# update mac address <mac> <target>
#
update_mac() {
    local mac1="$1"
    local target="$2"

    echo "${mac1}" | grep -iq '[0-9a-f]\{12\}'
    if [ $? -ne 0 ]; then
        echo
        echo "error: The MAC address \"${mac1}\" is invalid. A valid MAC address must be 12 hex chars: mac=aabbccddeeff"
        echo
        exit 2
    fi
    local mac2=$(printf "%x" $((0x${mac1} + 1)))

    echo
    echo "  Updating MAC addresses for image file: ${target}"
    echo "    MAC 1: ${mac1}"
    echo "    MAC 2: ${mac2}"
    echo
    echo
    "${MAC2BIN}" "${mac1}" | dd bs=1 of="${target}" count=6 seek=$MAC1A_ADDR conv=notrunc
    "${MAC2BIN}" "${mac1}" | dd bs=1 of="${target}" count=6 seek=$MAC1B_ADDR conv=notrunc
    "${MAC2BIN}" "${mac2}" | dd bs=1 of="${target}" count=6 seek=$MAC2_ADDR conv=notrunc
}


#
# update device id <serial> <target>
#
update_devid() {
    local serial="$1"
    local target="$2"

    echo "${serial}" | grep -iq '[0abcdefghjkmnpqrstuvwxyz12346789]\{9\}'
    if [ $? -ne 0 ]; then
        echo
        echo "error: The device id \"${serial}\" is invalid. A valid device id must be 9 chars: mac=abc123xyz"
        echo
        exit 3
    fi
    local new_target="$(echo "${target}" | sed "s/_[[:alnum:]]\{9\}.bin/_${serial}.bin/")"

    echo
    echo "  Updating device id for image file: ${target}"
    echo "    Serial Number: ${serial}"
    echo "    New File Name: ${new_target}"
    echo
    echo

    echo "${serial}" | dd bs=1 of="${target}" count=9 seek=$SERIAL_ADDR conv=notrunc
    mv "${target}" "${new_target}"
}


#
# report device id and mac addresses <target>
#
report_info() {
    local target="$1"

    local serial=$(od -An -v -ta -j262407 -N9 "${target}" | sed 's/ //g')
    local mac1=$(od -An -v -t x1 -j262184 -N6 "${target}" | sed 's/ //g')
    local mac2=$(od -An -v -t x1 -j262190 -N6 "${target}" | sed 's/ //g')

    echo
    echo "     Image File: ${target}"
    echo "  Serial Number: ${serial}"
    echo "          MAC 1: ${mac1}"
    echo "          MAC 2: ${mac2}"
    echo
}


#
# help
#
usage() {
cat << EOF

Usage:
 $(basename "$0") [options] <target>

Options:
 -d, --devid <serialnum>  set device id: -d aj3cmxeu1 (use "" generate a new serial number)
 -g, --genserial          generate a new serial number and exit
 -h, --help               display this help text and exit
 -m, --mac <address>      set mac address: -m aabbccddeeff
 -r, --report             report device id and mac addresses then exit

EOF
}


#
# parse command line
#
MAC=
SERIAL_NUM=
REPORT=
while [ $# -gt 0 ]; do
  case "$1" in
    -d|--devid)
      SERIAL_NUM="$2"
      shift
      if [ -z "${SERIAL_NUM}" ]; then
        SERIAL_NUM=$("${GENSERIAL}")
        echo
        echo "  generating new serial number: ${SERIAL_NUM}"
      fi
      ;;
    -g|--genserial)
      eval "${GENSERIAL}"
      exit 0
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    -m|--mac)
      MAC="$2"
      shift
      ;;
    -r|--report)
      REPORT=1
      ;;
    -*)
      echo
      echo "***"
      echo "***  ERROR: unknown option: $1"
      echo "***"
      usage
      exit 1
      ;;
    *)
      TARGET="$1"
      ;;
  esac
  shift
done

if [ -z "${TARGET}" ]; then
    echo
    echo "***"
    echo "***  ERROR: <target> must be specified"
    echo "***"
    usage
    exit 1
fi

if [ ! -f "${TARGET}" ] || [ "$(stat -c %s "${TARGET}")" -lt "327680" ]; then
    echo
    echo "***"
    echo "***  ERROR: <target> is not a valid image file"
    echo "***"
    usage
    exit 1
fi

if [ -n "${REPORT}" ]; then
    report_info "${TARGET}"
    exit 0
fi

if [ -n "${MAC}" ]; then
    update_mac "${MAC}" "${TARGET}"
elif [ -n "${SERIAL_NUM}" ]; then
    update_devid "${SERIAL_NUM}" "${TARGET}"
else
    echo
    echo "***"
    echo "***  ERROR: at least one [option] must be specified"
    echo "***"
    usage
    exit 1
fi

