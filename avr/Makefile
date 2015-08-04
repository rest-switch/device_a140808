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

#######################################
# Makefile for the a140808 ATMega32A
#######################################

####################
# the minipro and the avrisp2 requires the following rule to be accessed by non-root
#
#   cat > /etc/udev/rules.d/40-usbdev.rules << EOF
#   # autoelectric.cn minipro
#   SUBSYSTEM=="usb", ATTRS{idVendor}=="04d8", ATTRS{idProduct}=="e11c", MODE="0660", GROUP="plugdev"
#
#   # atmel avrisp-mkii
#   SUBSYSTEM=="usb", ATTRS{idVendor}=="03eb", ATTRS{idProduct}=="2104", MODE="0660", GROUP="plugdev"
#   EOF
#   service udev restart
#

## mcu settings
MCU    = atmega32
MCU_HZ = 16000000UL

SRC_DIR        := src

## output targets
TARGET_DIR     := bin
TARGET_NAME    := a140808
TARGET_ELF     := $(TARGET_DIR)/$(TARGET_NAME).elf
TARGET_BIN     := $(TARGET_DIR)/$(TARGET_NAME).bin
TARGET_HEX     := $(TARGET_DIR)/$(TARGET_NAME).hex
TARGET_EEP     := $(TARGET_DIR)/$(TARGET_NAME).eep
TARGET_LSS     := $(TARGET_DIR)/$(TARGET_NAME).lss
TARGET_MAP     := $(TARGET_DIR)/$(TARGET_NAME).map
FUSES_CONF     := $(TARGET_DIR)/fuses.conf

## avr tools
AVR_TOOLS      := tools/avr
AVR_TOOLS_URL  := http://downloads.arduino.cc/arduino-1.6.5-linux64.tar.xz
AVR_DUDE       := $(AVR_TOOLS)/bin/avrdude
AVR_DUDE_CFG   := $(AVR_TOOLS)/etc/avrdude.conf
CC             := $(AVR_TOOLS)/bin/avr-gcc
CPP            := $(AVR_TOOLS)/bin/avr-g++
OBJ_CP         := $(AVR_TOOLS)/bin/avr-objcopy
OBJ_DMP        := $(AVR_TOOLS)/bin/avr-objdump
OBJ_SZ         := $(AVR_TOOLS)/bin/avr-size

## minipro
MINIPRO_ROOT     := tools/minipro
MINIPRO          := $(MINIPRO_ROOT)/minipro
MINIPRO_SRC_URL  := https://github.com/inindev/minipro/archive/master.tar.gz
#MINIPRO_SRC_URL  := https://github.com/vdudouyt/minipro/archive/master.tar.gz

## fuses and lock bits from elf
FUSE_LOW  = $$($(OBJ_CP) -j .fuse --change-section-lma .fuse=0 -O binary $(TARGET_ELF) /dev/stdout | od -An -v -t x1 -j0 -N1 | sed 's/ //g')
FUSE_HIGH = $$($(OBJ_CP) -j .fuse --change-section-lma .fuse=0 -O binary $(TARGET_ELF) /dev/stdout | od -An -v -t x1 -j1 -N1 | sed 's/ //g')
LOCK_BITS = $$($(OBJ_CP) -j .lock --change-section-lma .lock=0 -O binary $(TARGET_ELF) /dev/stdout | od -An -v -t x1 -j0 -N1 | sed 's/ //g')
# avrdude doesnt like bits 6 & 7 set, mask them off
AVRDUDE_LOCK_BITS = $$(printf '%x\n' $$((0x3f & 0x$(LOCK_BITS))))

## extra include dirs
INCLUDES =

## options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## compile options common for all C compilation units
CFLAGS  = $(COMMON)
CFLAGS += -Wall -gdwarf-2 -DF_CPU=$(MCU_HZ) -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -MD -MP -MT $(*F).o -MF "$(TARGET_DIR)/$(@F).dep"

## assembly specific flags
ASMFLAGS  = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## linker flags
LDFLAGS  = $(COMMON)
LDFLAGS += -Wl,-Map=$(TARGET_MAP)

## intel hex file production flags
HEX_FLASH_FLAGS = -j .text -j .data
#
BIN_FLASH_FLAGS = -j .text -j .data
#
HEX_EEPROM_FLAGS  = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings

## objects that must be built in order to link
OBJECTS = $(TARGET_DIR)/avr_main.o $(TARGET_DIR)/avr_impl.o $(TARGET_DIR)/serial.o

## build
all: $(TARGET_DIR) $(TARGET_ELF) $(TARGET_BIN) $(TARGET_HEX) $(TARGET_EEP) $(TARGET_LSS) $(FUSES_CONF) size

## output dir
$(TARGET_DIR):
	@test -d "$(TARGET_DIR)" || mkdir -p "$(TARGET_DIR)"

## compile
$(TARGET_DIR)/%.o: $(SRC_DIR)/%.cpp | $(TARGET_DIR)
	$(CPP) $(INCLUDES) $(CFLAGS) -o "$@" -c "$<"

## link
$(TARGET_ELF): avr-tools $(OBJECTS)
	$(CPP) $(LDFLAGS) $(OBJECTS) $(LIBDIRS) $(LIBS) -o "$(TARGET_ELF)"

%.bin: $(TARGET_ELF)
	$(OBJ_CP) $(BIN_FLASH_FLAGS) -O binary "$<" "$@"
	@tr '\000' '\377' < /dev/zero | dd bs=1 count=$$((0x8000 - $$(stat -c '%s' "$@"))) >> "$@"

%.hex: $(TARGET_ELF)
	$(OBJ_CP) $(HEX_FLASH_FLAGS) -O ihex "$<" "$@"

%.eep: $(TARGET_ELF)
	$(OBJ_CP) $(HEX_EEPROM_FLAGS) -O ihex "$<" "$@"

%.lss: $(TARGET_ELF)
	$(OBJ_DMP) -h -S "$<" > "$@"

$(FUSES_CONF): $(TARGET_ELF)
	@printf 'fuses_lo = 0x00%s\n' $(FUSE_LOW) > $(FUSES_CONF)
	@printf 'fuses_hi = 0x00%s\n' $(FUSE_HIGH) >> $(FUSES_CONF)
	@printf 'lock_byte = 0x00%s\n' $(LOCK_BITS) >> $(FUSES_CONF)

.PHONY: size
size: $(TARGET_ELF)
	@echo
	@$(OBJ_SZ) -C --mcu=$(MCU) "$(TARGET_ELF)"

## clean intermediate files
.PHONY: clean
clean:
	-rm -rf "$(TARGET_DIR)"

#####################
#####  minipro  #####
#####################

## flash avr with bin
.PHONY: mp-program
mp-program: | mp-flash-write mp-fuse-write

.PHONY: mp-flash-write
mp-flash-write: $(TARGET_BIN) mp-test
	"$(MINIPRO)" -I -p "$(MCU)" -c code -w "$(TARGET_BIN)"

## read/burn avr fuses
.PHONY: mp-fuse-read mp-fuse-burn
mp-fuse-read: mp-test
	@"$(MINIPRO)" -I -p "$(MCU)" -c config -r "$(TARGET_DIR)/mp-fuses.txt"
	@echo
	@cat "$(TARGET_DIR)/mp-fuses.txt"
	@rm "$(TARGET_DIR)/mp-fuses.txt"
	@echo
mp-fuse-write: $(FUSES_CONF) mp-test
	@echo
	@echo "writing fuse settings:"
	@cat "$(FUSES_CONF)"
	@echo
	@"$(MINIPRO)" -I -p "$(MCU)" -c config -w "$(FUSES_CONF)"

.PHONY: mp-test
mp-test: minipro
	@"$(MINIPRO)" -I -p "$(MCU)" -c config -r /dev/null > /dev/null || (echo "\n\t************************************************************\n\t*** MINIPRO ERROR: $$? ***  Is the Atmel device plugged-in?\n\t************************************************************\n\n"; exit 1)

.PHONY: minipro
minipro: | $(MINIPRO)
$(MINIPRO):
	@if [ ! -d "$(MINIPRO_ROOT)" ]; then \
		echo; \
		echo "fetching minipro source...";\
		mkdir -p "$(MINIPRO_ROOT)"; \
		wget -O- $(MINIPRO_SRC_URL) | tar --strip-components=1 -xzvC "$(MINIPRO_ROOT)"; \
	fi
	$(MAKE) -C "$(MINIPRO_ROOT)"


#########################
#####  avrisp-mkii  #####
#########################

## flash avr with hex
.PHONY: avr-program
avr-program: | avr-flash-write avr-fuse-write

.PHONY: avr-flash-write
avr-flash-write: $(TARGET_HEX) avr-test
	"$(AVR_DUDE)" -C "$(AVR_DUDE_CFG)" -p "$(MCU)" -c avrisp2 -P usb -U flash:w:"$(TARGET_HEX)":i

.PHONY: avr-flash-erase
avr-flash-erase: avr-test
	@"$(AVR_DUDE)" -C "$(AVR_DUDE_CFG)" -p "$(MCU)" -c avrisp2 -P usb -e

## read/burn avr fuses
.PHONY: avr-fuse-read avr-fuse-burn
avr-fuse-read: avr-test
	@"$(AVR_DUDE)" -u -C "$(AVR_DUDE_CFG)" -p "$(MCU)" -c avrisp2 -P usb -U lfuse:r:-:h -U hfuse:r:-:h -U lock:r:-:h
avr-fuse-write: $(TARGET_ELF) avr-test
	@echo "writing fuse low:  0x$(FUSE_LOW)"
	@echo "writing fuse high: 0x$(FUSE_HIGH)"
	@echo "writing lock bits: 0x$(AVRDUDE_LOCK_BITS)"
	@"$(AVR_DUDE)" -u -C "$(AVR_DUDE_CFG)" -p "$(MCU)" -c avrisp2 -P usb -U lfuse:w:0x$(FUSE_LOW):m -U hfuse:w:0x$(FUSE_HIGH):m -U lock:w:0x$(AVRDUDE_LOCK_BITS):m

.PHONY: avr-test
avr-test: avr-tools
	@"$(AVR_DUDE)" -C "$(AVR_DUDE_CFG)" -p "$(MCU)" -c avrisp2 -P usb 2> /dev/null || (echo "\n\t************************************************************\n\t*** AVRDUDE ERROR: $$? ***  Is the Atmel device plugged-in?\n\t************************************************************\n\n"; exit 1)

.PHONY: avrtools
avr-tools: | $(AVR_TOOLS)
$(AVR_TOOLS):
	@echo
	@echo fetching avr tools...
	@test -d "$(AVR_TOOLS)" || mkdir -p "$(AVR_TOOLS)"
	@wget -O- "$(AVR_TOOLS_URL)" | tar --strip-components=4 -C "$(AVR_TOOLS)" -xJv "arduino-1.6.5/hardware/tools/avr"

## include dependencies
include $(wildcard "$(TARGET_DIR)/*.dep")

