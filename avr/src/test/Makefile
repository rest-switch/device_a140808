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

TARGET = a140808-openwrt

## binary paths
TOOL_BASE = $(STAGING_DIR)
CC        = $(TOOL_BASE)/bin/mipsel-openwrt-linux-gcc
CPP       = $(TOOL_BASE)/bin/mipsel-openwrt-linux-g++

## extra include dirs
INCLUDES = -I../incl -I$(STAGING_DIR)/usr/include/uClibc++ -I$(STAGING_DIR)/usr/include -I$(STAGING_DIR)/include

## options common to compile, link and assembly rules
COMMON =

## compile options common for all C compilation units
CFLAGS = $(COMMON)
FLAGS += -Wall -gdwarf-2 -Os -funsigned-char
CFLAGS += -MD -MP -MT $(*F).o -MF .dep/$(@F).d
CFLAGS += -fno-builtin -fno-rtti -nostdinc++

## linker flags
LDFLAGS  = $(COMMON)
LDFLAGS += -L$(STAGING_DIR)/usr/lib -L$(STAGING_DIR)/lib
LDFLAGS += -nodefaultlibs -luClibc++ -lgcc_s -lc

## objects that must be built in order to link
OBJECTS = main.o serial.o kbhit.o

## build
all: $(TARGET)

## compile
main.o: ./main.cpp
	$(CPP) $(INCLUDES) $(CFLAGS) -c  $<

serial.o: ./serial.cpp
	$(CPP) $(INCLUDES) $(CFLAGS) -c  $<

kbhit.o: ./kbhit.cpp
	$(CPP) $(INCLUDES) $(CFLAGS) -c  $<

## link
$(TARGET): $(OBJECTS)
	$(CPP) $(LDFLAGS) $(OBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

## clean target
.PHONY: clean
clean:
	-rm -rf $(TARGET) .dep $(OBJECTS)

## make .dep dir
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

