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
IMAGE_FILE_BYTES=$((0x400000))
MAKEFILE_MODE=0
RETVAL=''


#
# get_image_file() - find availables target images
#
get_image_file() {
    RETVAL=''

    local projbin_leaf
    projbin_leaf=$(basename "$PROJBIN")
    local files
    files=$(find "$PROJBIN" -maxdepth 1 -type f -size "$IMAGE_FILE_BYTES"'c' -printf "%TY-%Tm-%Td %TH:%TM:%0.2TS $projbin_leaf/%P\n" | sort -r)
    local file_count
    file_count=$(echo "$files" | wc -l)

    if [ "$file_count" -lt "1" ]; then
        return
    fi

    if [ "$file_count" -eq "1" ]; then
        RETVAL=$(echo "$files" | cut -d' ' -f3)
        return
    fi

    echo ""
    echo "Select the image file to flash:"
    echo ""
    for i in $(seq 1 "$file_count")
    do
        local file
        file=$(echo "$files" | sed -n "$i p")
        echo "  $i) $file"
    done
    echo "  q) quit"
    echo ""
    read -p "choice: " num

    case "$num" in
        ''|*[!0-9]*) ;;
        0) ;;
        *)
            RETVAL=$(echo "$files" | sed -n "$num p" | cut -d' ' -f3)
            ;;
    esac
}


#
# get_flash_id() - autodetect the connected flash memory id
#
get_flash_id() {
    RETVAL=''

    local output
    output=$("$MINIPRO" -q)
    if [ "$?" -ne "0" ]; then
        return 10
    fi

    RETVAL=$(echo "$output" | sed -rn 's/^Device Id: (0x[a-f0-9]*)$/\1/p')

    return 0
}


#
# get_flash_name([flash_id]) - return the flash memory name for the specified memory id
#
get_flash_name() {
    RETVAL=''
    local flash_id="$1"

    if [ -z "$flash_id" ]; then
        get_flash_id
        if [ "$?" -ne "0" ]; then
            echo "error: failed to detect flash device id"
            return 21
        fi
        local flash_id="$RETVAL"
        RETVAL=''
    fi

    case "$flash_id" in
        0x9d467f)
            RETVAL='PM25LQ032C'
            ;;
        0xc22016)
            RETVAL='MX25L3206E'
            ;;
        0xef4016)
            RETVAL='W25Q32BV'
            ;;
        *)
            return 22
            ;;
    esac

    return 0
}


#
# display_flash_info - detect and display flash id and name
#
display_flash_info() {
    get_flash_id
    if [ "$?" -ne "0" ]; then
        echo "error: failed to detect flash device id"
        return 31
    fi
    local flash_id="$RETVAL"
    echo "Flash device id: $flash_id"

    get_flash_name "$flash_id"
    if [ "$?" -ne "0" ]; then
        echo "error: no flash device name found for part id: $flash_id"
        return 32
    fi
    local flash_name="$RETVAL"
    echo "Flash device name: $flash_name"

    return 0
}


#
# write_flash([image_file], [flash_name]) - write the image file to the specified flash memory
#
write_flash() {
    local image_file="$1"
    local flash_name="$2"

    if [ -z "$image_file" ]; then
        get_image_file
        image_file="$RETVAL"
        if [ -z "$image_file" ]; then
            if [ "$MAKEFILE_MODE" -eq "1" ]; then
                echo "********************************************************************************"
                echo " error: Image file not specified - eg: image=<a140808_a????????.bin>"
                echo "********************************************************************************"
            else
                echo "error: specify the binary file to write to flash"
            fi
            return 1
        fi
    fi

    local file_bytes
    file_bytes=$(stat -c '%s' "$image_file")
    if [ "$?" -ne "0" ]; then
        if [ "$MAKEFILE_MODE" -eq "1" ]; then
            echo "********************************************************************************"
            echo " error: Failed to get image file stats: [$image_file]"
            echo "********************************************************************************"
        else
            echo "error: failed to get image file stats: [$image_file]"
        fi
        return 2
    fi

    if [ "$file_bytes" -ne "$IMAGE_FILE_BYTES" ]; then
        if [ "$MAKEFILE_MODE" -eq "1" ]; then
            echo "********************************************************************************"
            echo " error: Image file \"$image_file\""
            echo "        incorrect file size - expected: $IMAGE_FILE_BYTES  got: $file_bytes"
            echo "********************************************************************************"
        else
            echo "error: incorrect binary file size - expected: $IMAGE_FILE_BYTES  got: $file_bytes"
        fi
        return 3
    fi

    if [ -z "$flash_name" ]; then
        get_flash_name
        if [ "$?" -ne "0" ]; then
            if [ "$MAKEFILE_MODE" -eq "1" ]; then
                echo "********************************************************************************"
                echo " error: The flash device must be either MX25L3206E, PM25LQ032C, or W25Q32BV"
                echo "             eg: flash=MX25L3206E or flash=W25Q32BV"
                echo "********************************************************************************"
            else
                echo "error: specify flash memory name - eg: MX25L3206E, PM25LQ032C, or W25Q32BV"
            fi
            return 4
        fi
        local flash_name="$RETVAL"
        RETVAL=''
    fi

    echo ""
    echo "Writing image file to flash:"
    echo "   Flash device name: $flash_name"
    echo "   Image file name: $image_file"
    echo ""

    "$MINIPRO" -p "$flash_name" -w "$image_file"
    if [ "$?" -ne "0" ]; then
        if [ "$MAKEFILE_MODE" -eq "1" ]; then
            echo "********************************************************************************"
            echo " error: Flash device programming failed."
            echo "********************************************************************************"
        else
            echo "error: flash device programming failed"
        fi
        return 5
    fi

    return 0
}


#
# usage() - command line help
#
usage() {
cat << EOF

Usage:
 $(basename "$0") [options]

Options:
 -d, --detect            auto-detect flash memory id and name
 -f, --file <filename>   binary image filename
 -h, --help              display this help text and exit
 -p, --part <partname>   flash memory part name - eg: MX25L3206E, PM25LQ032C, or W25Q32BV

EOF
}


#
# main() - program entry point
#
main() {
    local image_file=
    local flash_name=

    while [ $# -gt 0 ]; do
        case "$1" in
            -d|--detect)
                display_flash_info
                return 0
                ;;
            -f|--file)
                image_file="${2}"
                shift
                ;;
            -h|--help)
                usage
                return 0
                ;;
            --makefile_mode=1)
                MAKEFILE_MODE=1
                ;;
            -p|--part)
                flash_name="$2"
                shift
                ;;
            -*)
                errormsg "unknown option: $1"
                usage
                return 1
                ;;
            *)
                ;;
        esac
        shift
    done

    write_flash "$image_file" "$flash_name"
    return $?
}

main "$@"
return "$?"

