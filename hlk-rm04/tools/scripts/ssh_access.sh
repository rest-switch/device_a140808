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
OWRT_ROOT=$(readlink -f "${MYDIR}/../../openwrt-master")
PW_FILE="${OWRT_ROOT}/package/base-files/files/etc/shadow"
SSH_CERT_BITS=4096
SSH_CERT_PUB="${OWRT_ROOT}/package/network/services/dropbear/files/authorized_keys"
SSH_CERT_PVT=$(readlink -f "${OWRT_ROOT}/../a140808_rsa.private")


#
# ssh certificate auth: etc/dropbear/authorized_keys
#
ssh_cert() {
    local opt="$1"
    local pvt_cert=
    local pvt_cert_bytes=0

    case "$opt" in
        none)
            # remove the public key
            umask 077; touch "${SSH_CERT_PUB}.tmp" && mv "${SSH_CERT_PUB}.tmp" "${SSH_CERT_PUB}"
            if [ $? -ne 0 ]; then
                errormsg "failed to remove ssh certificate (code: $?)"
                exit 10
            fi
            printf "ssh certificate authentication removed\n"
            return 0
            ;;
        gen)
            # generate private key
            printf "generating %s bit rsa ssh certificate...\n" "$SSH_CERT_BITS"
            if [ -e "${SSH_CERT_PVT}" ]; then
                rm "${SSH_CERT_PVT}"
            fi

            ssh-keygen -q -N "" -t rsa -b $SSH_CERT_BITS -f "${SSH_CERT_PVT}"
            if [ $? -ne 0 ]; then
                errormsg "failed to generate ssh private certificate (code: $?)"
                exit 11
            fi

            if [ -e "${SSH_CERT_PVT}.pub" ]; then
                rm "${SSH_CERT_PVT}.pub"
            fi

            pvt_cert="${SSH_CERT_PVT}"
            ;;
        *)
            # supplied private key
            pvt_cert="${opt}"
            pvt_cert_bytes=$(stat -c '%s' "${pvt_cert}" 2>/dev/null)
            if [ $? -ne 0 ]; then
                errormsg "failed to stat specified ssh private certificate: ${pvt_cert} (code: $?)"
                exit 12
            fi

            if [ "$pvt_cert_bytes" -lt "1600" ]; then
                errormsg "invalid ssh private key: specify the path to a private of at least 2048 bits"
                exit 13
            fi
            ;;
    esac

    if [ -z "$pvt_cert" ]; then
        return 0  # nothing to do
    fi

    # extract public key from private key
    if [ -e "${SSH_CERT_PUB}.tmp" ]; then
        rm "${SSH_CERT_PUB}.tmp"
    fi

    umask 077; touch "${SSH_CERT_PUB}.tmp"
    if [ $? -ne 0 ]; then
        errormsg "failed to create ssh public certificate temp file (code: $?)"
        exit 14
    fi

    ssh-keygen -y -f "${pvt_cert}" > "${SSH_CERT_PUB}.tmp"
    if [ $? -ne 0 ]; then
        errormsg "failed to extract ssh public certificate to temp file (code: $?)"
        exit 15
    fi

    mv "${SSH_CERT_PUB}.tmp" "${SSH_CERT_PUB}"
    if [ $? -ne 0 ]; then
        errormsg "failed to move temp ssh public certificate to ${SSH_CERT_PUB} (code: $?)"
        exit 16
    fi

    printf "ssh access enabled --> private certificate: %s\n" "${pvt_cert}"
    return 0
}


#
# password auth for root: etc/shadow
#
ssh_pass() {
    local opt="$1"
    local passwd=

    case "$opt" in
        none)
            # remove root's hash in etc/shadow
            awk 'BEGIN {OFS=FS=":"} $1=="root" {$2="x"} {print}' "${PW_FILE}" > "${PW_FILE}.tmp" && 
            mv "${PW_FILE}.tmp" "${PW_FILE}"
            if [ $? -ne 0 ]; then
                errormsg "failed to remove root password (code: $?)"
                exit 17
            fi
            printf "password authentication removed\n"
            return 0
            ;;
        pmt)
            # prompt for password
            for i in 1 2 3
            do
                printf "\n\n\n\n##################################"
                printf "\nSet SSH password for root account:"
                stty -echo
                printf "\npassword: "
                read -r secret1
                printf "\n"
                printf "confirm: "
                read -r secret2
                printf "\n"
                stty echo
                if [ "$secret1" != "$secret2" ]; then
                    if [ "$i" -gt "2" ]; then
                        errormsg "passwords do not match, abort"
                        exit 18
                    fi
                    errormsg "passwords do not match, try again..."
                else
                    passwd="$secret1"
                    break
                fi
            done
            ;;
        *)
            # password is param
            passwd="$opt"
            ;;
    esac

    if [ -z "$passwd" ]; then
        return 0  # nothing to do
    fi

    # set root's hash in etc/shadow
    pwhash=$(openssl passwd -1 "$passwd") && 
    awk 'BEGIN {OFS=FS=":"} $1=="root" {$2="'"$pwhash"'"} {print}' "${PW_FILE}" > "${PW_FILE}.tmp" && 
    mv "${PW_FILE}.tmp" "${PW_FILE}"
    if [ $? -ne 0 ]; then
        errormsg "failed to set root password (code: $?)"
        exit 19
    fi

    printf "root password set\n"
    return 0
}


#
# has_cert: returns 0 for no valid pub cert, 1 for valid pub cert
#
has_cert() {
    local pub_cert_bytes=0

    pub_cert_bytes=$(stat -c '%s' "${SSH_CERT_PUB}" 2>/dev/null)
    if [ $? -ne 0 ]; then
        errormsg "failed to stat ssh public certificate: ${SSH_CERT_PUB} (code: $?)"
        exit 20
    fi

    if [ "$pub_cert_bytes" -lt "380" ]; then
        return 1  # has cert
    fi

    return 0  # no cert
}


#
# has_pass: returns 0 for no root hash, 1 for root hash
#
has_pass() {
    grep -iq '^root:.\?:' "${PW_FILE}"
    if [ $? -eq 0 ]; then
        return 1  # has root pw hash
    fi

    return 0  # no root pw hash
}


#
# errormsg <msg>
#
errormsg() {
    printf '\n***\n***  error: %s\n***\n' "$1"
}


#
# help
#
usage() {
cat << EOF

Usage:
 $(basename "$0") [options]

Options:
 -a, --access                         access check: exit 0: pass and/or cert access enabled, exit 1: no access
 -c, --cert <none | gen | CERT_PATH>  none=remove cert, gen=create cert, CERT_PATH=path to existing private cert
 -p, --pass <none | pmt | secret>     none=remove password, pmt=prompt, secret=set password to "secret"
 -h, --help                           display this help text and exit
 -n, --none                           no ssh access (remove both certificate and password)
 -r, --report                         report certificate and password status then exit

EOF
}


#
# parse command line
#
if [ $# -eq 0 ]; then
    usage
    exit 0
fi

while [ $# -gt 0 ]; do
    case "$1" in
    -a|--access)
        has_cert && exit 0
        has_pass && exit 0
        exit 1  # no access
        ;;
    -c|--cert)
        test $# -lt 2 && usage && exit 1
        ssh_cert "${2}"
        shift
        ;;
    -p|--pass)
        test $# -lt 2 && usage && exit 1
        ssh_pass "${2}"
        shift
        ;;
    -h|--help)
        usage
        exit 0
        ;;
    -n|--none)
        ssh_cert none
        ssh_pass none
        ;;
    -r|--report)
        has_cert && printf "cert present\n" || printf "cert not present\n"
        has_pass && printf "pass present\n" || printf "pass not present\n"
        ;;
    -*)
        errormsg "unknown option: $1"
        usage
        exit 1
        ;;
    *)
        usage
        exit 1
        ;;
    esac
    shift
done

