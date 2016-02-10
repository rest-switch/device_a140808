#!/bin/sh
#
# Copyright 2015-2016 The REST Switch Authors
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
MINIPRO="${MYDIR}/../minipro/minipro"
PROJBIN="${MYDIR}/../../bin"
BIN_FILE_BYTES=$((0x400000))
RETVAL=''


main() {
    local bin_file="${1}"
    local device_name="$2"

    if [ -z "$bin_file" ]; then
        find_imgfile; bin_file="$RETVAL"
        if [ -z "$bin_file" ]; then
            echo "error: specify the binary file to write to flash"
            return 1
        fi
    fi

    local file_bytes
    file_bytes=$(stat -c '%s' "$bin_file")
    if [ "$?" -ne "0" ]; then
        echo "failed to get file stats: [$bin_file]"
        return 2
    fi

    if [ "$file_bytes" -ne "$BIN_FILE_BYTES" ]; then
        echo "incorrect binary file size - expected: $BIN_FILE_BYTES  got: $file_bytes"
        return 3
    fi

    if [ -z "$device_name" ]; then
        device_name="W25Q32BV"
    fi

    write_device "$bin_file" "$device_name"
    return $?
}


find_imgfile() {
    RETVAL=''

    local projbin_leaf
    projbin_leaf=$(basename "$PROJBIN")
    local files
    files=$(find "$PROJBIN" -maxdepth 1 -type f -size "$BIN_FILE_BYTES"'c' -printf "%TY-%Tm-%Td %TH:%TM:%0.2TS $projbin_leaf/%P %p\n" | sort -r)
    local file_count
    file_count=$(echo "$files" | wc -l)

    if [ "$file_count" -lt "1" ]; then
        return
    fi

    local file
    if [ "$file_count" -eq "1" ]; then
        file=$(echo "$files" | cut -d' ' -f4)
        RETVAL=$(realpath "$file")
        return
    fi

    echo ""
    echo "Select the image file to flash:"
    echo ""
    for i in $(seq 1 "$file_count")
    do
        file=$(echo "$files" | sed -n "$i p" | cut -d' ' -f1-3)
        echo "  $i) $file"
    done
    echo "  q) quit"
    echo ""
    read -p "choice: " num
    echo ""

    case "$num" in
        ''|*[!0-9]*) ;;
        0) ;;
        *)
            file=$(echo "$files" | sed -n "$num p" | cut -d' ' -f4)
            RETVAL=$(realpath "$file")
            ;;
    esac
}


write_device() {
    local bin_file="$1"
    local device_name="$2"

    echo "programming flash - file: $bin_file"
    echo "programming flash - device: $device_name"
    local output
    output=$("$MINIPRO" -p "$device_name" -w "$bin_file" 3>&1 1>&2 2>&3)
    if [ "$?" -eq "0" ]; then
        return 0
    fi

    # auto detect
    local device_id
    device_id=$(echo "$output" | sed -rn 's/.*got (0x[a-f0-9]*).*/\1/p')
    case $device_id in
        0x9d467f)
            device_name="PM25LQ032C"
            ;;
        0xc22016)
            device_name="MX25L3206E"
            ;;
        0xef4016)
            device_name="W25Q32BV"
            ;;
        *)
            echo "unknown device: [$device_id]"
            return 11
            ;;
    esac

    echo "programming flash - device: $device_name"
    "$MINIPRO" -p "$device_name" -w "$bin_file"
    if [ "$?" -ne "0" ]; then
        echo "programming failed"
        return 12
    fi

    return 0
}

main "$@"
return $?

