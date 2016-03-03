#!/usr/bin/env python
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

import datetime
import getopt
import os
import re
import subprocess
import sys

MYDIR = os.path.dirname(os.path.abspath(__file__))
MINIPRO = os.path.realpath(os.path.join(MYDIR, '../minipro/minipro'))
IMAGE_FILE_PATH = os.path.realpath(os.path.join(MYDIR, '../../bin'))
IMAGE_FILE_BYTES = 0x400000



#
# exception classes
#
class FlashLookupError(Exception):
    def __init__(self, msg): super(FlashLookupError, self).__init__(msg)
class FlashUnknownError(Exception):
    def __init__(self, msg): super(FlashUnknownError, self).__init__(msg)
class FlashInvalidError(Exception):
    def __init__(self, msg): super(FlashInvalidError, self).__init__(msg)
class ImageUnknownError(Exception):
    def __init__(self, msg): super(ImageUnknownError, self).__init__(msg)
class ImageInvalidError(Exception):
    def __init__(self, msg): super(ImageInvalidError, self).__init__(msg)


#
# get_image_file(path) - get image filepath
#
def get_image_file(path, size):
    matches = []
    for file in os.listdir(path):
        fs = os.stat(os.path.join(path, file))
        if fs.st_size == size: matches.append((fs.st_ctime, file))

    if len(matches) == 0:
        return ''

    if len(matches) == 1:
        file = matches[0][1]
        image_file = os.path.join(path, file)
        return image_file

    i = 0
    base = os.path.basename(path)
    matches.sort(key=lambda k:k[0], reverse=True)
    print('\nSelect the image file to flash:\n')
    for m in matches:
        i += 1
        ft = datetime.datetime.fromtimestamp(m[0]).strftime('%Y-%m-%d %H:%M:%S')
        fp = os.path.join(base, m[1])
        print('  {0:>2}) {1} {2}'.format(i, ft, fp))
    print('   q) quit\n')
    try: idx = parse_uint(raw_input('choice: '), len(matches))
    except KeyboardInterrupt: idx = 0
    if idx < 1: return ''

    file = matches[idx-1][1]
    image_file = os.path.join(path, file)
    return image_file


#
# get_flash_id() - autodetect the connected flash memory id
#
def get_flash_id():
    try:
        mp_out = subprocess.check_output([MINIPRO, '-q'])
        match = re.search(r'^Device Id: (0x[a-f0-9]*)$', mp_out, re.MULTILINE)
        found = match.group(1)
        val = int(found, 16)
        if val == 0: raise
        return val
    except:
        raise IOError('flash device programming failed, check connections and try again')


#
# get_flash_name(flash_id) - return the flash memory name for the specified memory id
#
def get_flash_name(flash_id):
    if flash_id == 0x9d467f:
        return 'PM25LQ032C'
    if flash_id == 0xc22016:
        return 'MX25L3206E'
    if flash_id == 0xef4016:
        return 'W25Q32BV'
    raise FlashLookupError('flash name lookup failed (flash is not one of: PM25LQ032C, MX25L3206E, W25Q32BV)')


#
# print_flash_info() - detect and display flash id and name
#
def print_flash_info():
    flash_id = get_flash_id()
    print('Flash device id: 0x{0:x}'.format(flash_id))
    flash_name = get_flash_name(flash_id)
    print('Flash device name: {0}'.format(flash_name))


#
# write_flash([image_file], [flash_name]) - write the image file to the specified flash memory
#
def write_flash(image_file, flash_name):
    if not image_file:
        image_file = get_image_file(IMAGE_FILE_PATH, IMAGE_FILE_BYTES)
        if not image_file: raise ImageUnknownError('specify the binary image file to write to flash')

    if not os.path.isfile(image_file):
        raise ImageUnknownError('cannot find the binary image file specified: {0}'.format(image_file))

    file_size = os.path.getsize(image_file)
    if file_size != IMAGE_FILE_BYTES:
        raise ImageInvalidError('incorrect binary file size - expected: {0} got {1}'.format(IMAGE_FILE_BYTES, file_size))

    if not flash_name:
        flash_name = get_flash_name(get_flash_id())

    print('')
    print('Writing image file to flash:')
    print('   Flash device name: {0}'.format(flash_name))
    print('   Image file name:   {0}'.format(image_file))
    print('')
    sp = subprocess.Popen([MINIPRO, '-p',flash_name, '-w',image_file], stderr=subprocess.PIPE)
    err = sp.communicate()[1]
    if sp.returncode != 0:
        err = err.strip().lower()
        if err.startswith('unknown device'): raise FlashUnknownError('unknown flash device name specified')
        if err.startswith('invalid device'): raise FlashInvalidError('invalid flash device name specified: {0}'.format(err[err.find('expect'):]))
        raise IOError('flash device programming failed, check connections and try again')

#
# parse_uint(val, max) - bounded uint parse (0 <= x < max)
#
def parse_uint(val, max):
    try: res = int(val)
    except: res = 0
    if res < 0: res = 0
    elif res > max: res = 0
    return res

#
# usage() - command line help
#
def usage(appname):
    appname = os.path.basename(appname)
    print('\nWrite a binary image file to the specified flash memory.\n')
    print('Usage:')
    print('  {0} [options]'.format(appname))
    print('')
    print('Options:')
    print('')
    print('  -d, --detect            auto-detect flash memory id and name')
    print('  -f, --file <filename>   binary image filename')
    print('  -h, --help              display this help text and exit')
    print('  -p, --part <partname>   flash memory part name - eg: MX25L3206E, PM25LQ032C, or W25Q32BV')
    print('')


#
# main() - program entry point
#
def main(argv):
    opts, extra = getopt.getopt(argv[1:],'df:hp:',['detect','file=','help','part='])

    show_help = False
    detect_flash = False
    image_file = ''
    flash_name = ''
    for opt, arg in opts:
        if opt in ('-h', '--help'):
            show_help = True
        elif opt in ('-d', '--detect'):
            detect_flash = True
        elif opt in ('-f', '--file'):
            image_file = arg.strip()
        elif opt in ('-p', '--part'):
            flash_name = arg.strip()

    if show_help:
        usage(argv[0])
    elif detect_flash:
        print_flash_info()
    else:
        write_flash(image_file, flash_name)


#
# entry point
#
if __name__ == '__main__':
    try:
        main(sys.argv)
    except getopt.GetoptError:
        usage(sys.argv[0])
        sys.exit(1)
    except FlashLookupError as ex:
        sys.stderr.write('error 2: {0}\n'.format(ex.message))
        sys.exit(2)
    except FlashUnknownError as ex:
        sys.stderr.write('error 3: {0}\n'.format(ex.message))
        sys.exit(3)
    except FlashInvalidError as ex:
        sys.stderr.write('error 4: {0}\n'.format(ex.message))
        sys.exit(4)
    except ImageUnknownError as ex:
        sys.stderr.write('error 5: {0}\n'.format(ex.message))
        sys.exit(5)
    except ImageInvalidError as ex:
        sys.stderr.write('error 6: {0}\n'.format(ex.message))
        sys.exit(6)
    except IOError as ex:
        sys.stderr.write('error 8: {0}\n'.format(ex.message))
        sys.exit(8)
    except:
        sys.stderr.write('error 9: programming failed\n')
        sys.exit(9)

