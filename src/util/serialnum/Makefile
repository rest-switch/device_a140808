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

TARGET = serialnum
TARGET_SRC = b32coder.c serialnum.c
TARGET_OBJ = $(TARGET_SRC:.c=.o)

CC = gcc
CFLAGS = -Wall -g -std=c99

# include directories in addition to /usr/include
#   -Ipath1 -Ipath2
INCLUDES = -I../include

# library directories in addition to /usr/lib
#   -Lpath1 -Lpath2
LFLAGS = -L../lib

# libraries to link
#   -lmylib for libmylib.so and -lm for libm.so
LIBS = -lcrypto

all: $(TARGET)

$(TARGET): $(TARGET_OBJ)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) $(TARGET_OBJ) $(LFLAGS) $(LIBS)
	@echo "\n\tTarget \"$(TARGET)\" has been compiled\n"

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file)
# (see the gnu make manual section about automatic variables)
.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	$(RM) *.o *~ $(TARGET)

depend: $(SRCS)
	makedepend $(INCLUDES) $^

.PHONY: depend clean

# DO NOT DELETE THIS LINE -- make depend needs it
