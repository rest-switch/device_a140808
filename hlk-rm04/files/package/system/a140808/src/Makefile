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

TARGET = a140808

all: $(TARGET)

OBJECTS = a140808.o daemon.o log.o msg_proc.o serial.o websock.o

#CFLAGS += -std=gnu99
CFLAGS += -std=c99
LDFLAGS += -ljson -lwebsockets

# compile
a140808.o: a140808.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -c $<

daemon.o: daemon.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -c $<

log.o: log.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -c $<

msg_proc.o: msg_proc.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -c $<

serial.o: serial.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -c $<

websock.o: websock.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -c $<

# link
$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET)

