#!/bin/sh
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


MYFILE=$(readlink -f "$0")
MYDIR=$(dirname "${MYFILE}")
MAC2BIN="${MYDIR}/../../../bin/mac2bin"


main() {
    if [ $# -ne 2 ]; then
        echo
        echo "  $(basename $0) <filename> <mac>"
        echo
        exit 1
x    fi
    local target=$1
    local mac1=$2

    echo "${mac1}" | grep -iq '[0-9a-f]\{12\}'
    if [ $? -ne 0 ]; then
        echo
        echo "error: The MAC address \"${mac1}\" is invalid. A valid MAC address must be 12 hex chars: mac=aabbccddeeff"
        echo
        exit 2
    fi
    local mac2=$(printf "%x" $((0x${mac1} + 1)))

    echo
    echo "  building image file..."
    echo "    Image File: ${target}"
    echo "         MAC 1: ${mac1}"
    echo "         MAC 2: ${mac2}"
    echo
    echo
    ${MAC2BIN} ${mac1} | dd bs=1 of="${target}" count=6 seek=262148 conv=notrunc
    ${MAC2BIN} ${mac1} | dd bs=1 of="${target}" count=6 seek=262184 conv=notrunc
    ${MAC2BIN} ${mac2} | dd bs=1 of="${target}" count=6 seek=262190 conv=notrunc
}

main $1 $2

