//
// Copyright 2015 The REST Switch Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, Licensor provides the Work (and each Contributor provides its 
// Contributions) on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied, including, 
// without limitation, any warranties or conditions of TITLE, NON-INFRINGEMENT, MERCHANTABILITY, or FITNESS FOR A PARTICULAR 
// PURPOSE. You are solely responsible for determining the appropriateness of using or redistributing the Work and assume any 
// risks associated with Your exercise of permissions under this License.
//
// Author: John Clark (johnc@restswitch.com)
//

#ifndef __msg_proc_h__
#define __msg_proc_h__

#include <stdint.h>
#include <stdbool.h>

// top level messages
#define MSG_PING                 0x01
#define MSG_PONG                 0x02
#define MSG_READ_REGISTER        0x11
#define MSG_WRITE_REGISTER       0x21
#define MSG_WRITE_REGISTER_BIT   0x31
#define MSG_PULSE_REGISTER_BIT   0x41
#define MSG_SUBSCRIBE_REGISTER   0x51
// register defs
#define REG_ERR_UNKNOWN          0x9F
#define REG_INPUT_1              0xA1
#define REG_OUTPUT_1             0xD1

// event callbacks, impl by avr_impl.cpp right now
void mp_on_pong(const uint8_t p_param1, const uint8_t p_param2, const uint8_t p_param3);
void mp_on_read_register(const uint8_t p_registerAddress);
void mp_on_write_register(const uint8_t p_registerAddress, const uint8_t p_value, const uint8_t p_mask);
void mp_on_write_register_bit(const uint8_t p_registerAddress, const uint8_t p_bit, const bool p_state);
void mp_on_pulse_register_bit(const uint8_t p_registerAddress, const uint8_t p_bit, const uint8_t p_durationMs);
void mp_on_subscribe_register(const uint8_t p_registerAddress, const uint8_t p_value, const bool p_cancel);

//
bool mp_init(const char* p_device, const uint16_t p_baud, const bool p_parity);
void mp_close(void);
bool mp_dispatch_ping(const uint8_t p_param1, const uint8_t p_param2, const uint8_t p_param3);
bool mp_dispatch_read_register(const uint8_t p_registerAddress);
bool mp_dispatch_write_register(const uint8_t p_registerAddress, const uint8_t p_value, const uint8_t p_mask);
bool mp_dispatch_write_register_bit(const uint8_t p_registerAddress, const uint8_t p_bit, const bool p_state);
bool mp_dispatch_pulse_register_bit(const uint8_t p_registerAddress, const uint8_t p_bit, const uint8_t p_durationMs);
bool mp_dispatch_subscribe_register(const uint8_t p_registerAddress, const uint8_t p_value, const bool p_cancel);
bool mp_dispatch_message(const uint8_t p_type, const uint8_t p_param1, const uint8_t p_param2, const uint8_t p_param3);
void mp_poll(void);

#endif // __msg_proc_h__
