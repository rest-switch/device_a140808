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

#######################################
# Makefile for the a140808 ATMega32A
#######################################

####################
# the avrisp2 requires the following rule to be accessed by non-root
#
#   cat > /etc/udev/rules.d/10-avrisp2.rules << EOF
#   # autoelectric.cn minipro
#   SUBSYSTEM=="usb", ATTRS{idVendor}=="04d8", ATTRS{idProduct}=="e11c", GROUP="plugdev", MODE="0666"
#
#   # atmel avrisp-mkii
#   SUBSYSTEM=="usb", ATTRS{idVendor}=="03eb", ATTRS{idProduct}=="2104", GROUP="plugdev", MODE="0666"
#   EOF
#   service udev restart
#

## mcu settings
MCU      := atmega32
MCU_HZ   := 16000000UL

## output targets
TARGET_DIR     := bin
TARGET_NAME    := a140808
TARGET_ELF     := $(TARGET_DIR)/$(TARGET_NAME).elf
TARGET_HEX     := $(TARGET_DIR)/$(TARGET_NAME).hex
TARGET_HEX2    := ../$(TARGET_NAME).hex
TARGET_EEP     := $(TARGET_DIR)/$(TARGET_NAME).eep
TARGET_LSS     := $(TARGET_DIR)/$(TARGET_NAME).lss
TARGET_MAP     := $(TARGET_DIR)/$(TARGET_NAME).map

## avr tools
AVR_TOOLS      := ../tools/avr
AVR_TOOLS_URL  := http://downloads.arduino.cc/arduino-1.6.5-linux64.tar.xz
AVR_DUDE       := $(AVR_TOOLS)/bin/avrdude
AVR_DUDE_CFG   := $(AVR_TOOLS)/etc/avrdude.conf
CC             := $(AVR_TOOLS)/bin/avr-gcc
CPP            := $(AVR_TOOLS)/bin/avr-g++
OBJ_CP         := $(AVR_TOOLS)/bin/avr-objcopy
OBJ_DMP        := $(AVR_TOOLS)/bin/avr-objdump
OBJ_SZ         := $(AVR_TOOLS)/bin/avr-size

## extra include dirs
INCLUDES = -I../incl

## options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## compile options common for all C compilation units
CFLAGS  = $(COMMON)
CFLAGS += -Wall -gdwarf-2 -DF_CPU=$(MCU_HZ) -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -MD -MP -MT $(*F).o -MF .dep/$(@F).d

## assembly specific flags
ASMFLAGS  = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## linker flags
LDFLAGS  = $(COMMON)
LDFLAGS += -Wl,-Map=$(TARGET_MAP)

## intel hex file production flags
HEX_FLASH_FLAGS = -R .eeprom -R .fuse -R .lock -R .signature
#
HEX_EEPROM_FLAGS  = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings

## objects that must be built in order to link
OBJECTS = $(TARGET_DIR)/avr_main.o $(TARGET_DIR)/avr_impl.o $(TARGET_DIR)/serial.o

## build
all: $(TARGET_DIR) $(TARGET_ELF) $(TARGET_HEX) $(TARGET_HEX2) $(TARGET_EEP) $(TARGET_LSS) size

## output dir
$(TARGET_DIR):
	@test -d "$(TARGET_DIR)" || mkdir -p "$(TARGET_DIR)"

## compile
$(TARGET_DIR)/%.o: %.cpp | $(TARGET_DIR)
	$(CPP) $(INCLUDES) $(CFLAGS) -o $@ -c $<

## link
$(TARGET_ELF): $(AVR_TOOLS) $(OBJECTS)
	$(CPP) $(LDFLAGS) $(OBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET_ELF)

%.hex: $(TARGET_ELF)
	$(OBJ_CP) $(HEX_FLASH_FLAGS) -O ihex $< $@

%.eep: $(TARGET_ELF)
	$(OBJ_CP) $(HEX_EEPROM_FLAGS) -O ihex $< $@

%.lss: $(TARGET_ELF)
	$(OBJ_DMP) -h -S $< > $@

.PHONY: size
size: $(TARGET_ELF)
	@echo
	@$(OBJ_SZ) -C --mcu=$(MCU) $(TARGET_ELF)

## clean intermediate files
.PHONY: clean
clean:
	-rm -rf "$(TARGET_DIR)"

## flash avr with hex
.PHONY: avr-install
avr-install: $(TARGET_ELF) avr-test
	$(AVR_DUDE) -C $(AVR_DUDE_CFG) -p $(MCU) -c avrisp2 -P usb -U flash:w:$(TARGET_HEX):i

.PHONY: avr-erase
avr-erase: avr-test
	@$(AVR_DUDE) -C $(AVR_DUDE_CFG) -p $(MCU) -c avrisp2 -P usb -e

## read/burn avr fuses
# atmega32: lfuse: 0x3f, hfuse: 0xc9, lock: 0x14
.PHONY: avr-fuse-read avr-fuse-burn
avr-fuse-read: avr-test
	@$(AVR_DUDE) -C $(AVR_DUDE_CFG) -p $(MCU) -c avrisp2 -P usb -U lfuse:r:-:h -U hfuse:r:-:h -U lock:r:-:h
avr-fuse-burn: avr-test
	@$(AVR_DUDE) -C $(AVR_DUDE_CFG) -p $(MCU) -c avrisp2 -P usb -U lfuse:w:0x3f:m -U hfuse:w:0xc9:m -U lock:w:0x14:m

.PHONY: avr-test
avr-test:
	@$(AVR_DUDE) -C $(AVR_DUDE_CFG) -p $(MCU) -c avrisp2 -P usb || (echo "\n\t************************************************************\n\t*** AVRDUDE ERROR: $$? ***  Is the Atmel device plugged-in?\n\t************************************************************\n\n"; exit 1)

## make .dep dir
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

$(AVR_TOOLS):
	@echo
	@echo fetching avr tools...
	@test -d "$(AVR_TOOLS)" || mkdir -p "$(AVR_TOOLS)"
	@wget -O- "$(AVR_TOOLS_URL)" | tar --strip-components=4 -C "$(AVR_TOOLS)" -xJv "arduino-1.6.5/hardware/tools/avr"

