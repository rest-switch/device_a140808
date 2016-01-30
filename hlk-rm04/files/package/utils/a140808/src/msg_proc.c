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

#include "ring_buf.h"
#include "msg_buf.h"
#include "serial.h"
#include "msg_proc.h"

static struct ring_buf_data s_msg_buf = { 0 };

void mp_process_message(const uint8_t p_type, const uint8_t p_param1, const uint8_t p_param2, const uint8_t p_param3);


////////////////////////////////////////
// p_parity
//   false: N81 (none, 8 data, 1 stop)
//   true:  E71 (even, 7 data, 1 stop)
bool mp_init(const char* p_device, const uint16_t p_baud, const bool p_parity)
{
    return(mb_init(&s_msg_buf) && sp_init(p_device, p_baud, p_parity));
}

void mp_close(void)
{
    mb_free(&s_msg_buf);
    sp_close();
}

////////////////////////////////////////
bool mp_dispatch_ping(const uint8_t p_param1, const uint8_t p_param2, const uint8_t p_param3)
{
    return(mp_dispatch_message(MSG_PING, p_param1, p_param2, p_param3));
}

////////////////////////////////////////
bool mp_dispatch_read_register(const uint8_t p_registerAddress)
{
    return(mp_dispatch_message(MSG_READ_REGISTER, p_registerAddress, 0x00, 0x00));
}

////////////////////////////////////////
bool mp_dispatch_write_register(const uint8_t p_registerAddress, const uint8_t p_value, const uint8_t p_mask)
{
    return(mp_dispatch_message(MSG_WRITE_REGISTER, p_registerAddress, p_value, p_mask));
}

////////////////////////////////////////
bool mp_dispatch_write_register_bit(const uint8_t p_registerAddress, const uint8_t p_bit, const bool p_state)
{
    if(p_bit > 0x07)
    {
        return(false);
    }
    return(mp_dispatch_message(MSG_WRITE_REGISTER_BIT, p_registerAddress, p_bit, (p_state ? 0xff : 0x00)));
}

////////////////////////////////////////
bool mp_dispatch_pulse_register_bit(const uint8_t p_registerAddress, const uint8_t p_bit, const uint8_t p_durationMs)
{
    if(p_bit > 0x07)
    {
        return(false);
    }
    return(mp_dispatch_message(MSG_PULSE_REGISTER_BIT, p_registerAddress, p_bit, p_durationMs));
}

////////////////////////////////////////
bool mp_dispatch_subscribe_register(const uint8_t p_registerAddress, const uint8_t p_value, const bool p_cancel)
{
    return(mp_dispatch_message(MSG_SUBSCRIBE_REGISTER, p_registerAddress, p_value, p_cancel));
}

////////////////////////////////////////
bool mp_dispatch_message(const uint8_t p_type, const uint8_t p_param1, const uint8_t p_param2, const uint8_t p_param3)
{
    mb_set_bytes(&s_msg_buf, p_type, p_param1, p_param2, p_param3);
    return(sp_write(&s_msg_buf));
}

////////////////////////////////////////
void mp_poll(void)
{
    if(sp_read(&s_msg_buf))
    {
        uint8_t type;
        uint8_t param1;
        uint8_t param2;
        uint8_t param3;
        if(!mb_get_bytes(&s_msg_buf, &type, &param1, &param2, &param3))
        {
            return;
        }

        mp_process_message(type, param1, param2, param3);
    }
}

////////////////////////////////////////
void mp_process_message(const uint8_t p_type, const uint8_t p_param1, const uint8_t p_param2, const uint8_t p_param3)
{
    switch(p_type)
    {
        case MSG_PING:
        {
            mp_dispatch_message(MSG_PONG, p_param1, p_param2, p_param3);
            break;
        }

        case MSG_PONG:
        {
            // param1: ping value1 (0-255)
            // param2: ping value2 (0-255)
            // param3: ping value3 (0-255)
            // void mp_on_pong(const uint8_t p_param1, const uint8_t p_param2, const uint8_t p_param3);
            mp_on_pong(p_param1, p_param2, p_param3);
            break;
        }

        case MSG_READ_REGISTER:
        {
            // param1: register address (0-255)
            // void mp_on_read_register(const uint8_t p_registerAddress);
            mp_on_read_register(p_param1);
            break;
        }

        case MSG_WRITE_REGISTER:
        {
            // param1: register address (0-255)
            // param2: value (0-255)
            // param3: mask (0-255)
            // void mp_on_write_register(const uint8_t p_registerAddress, const uint8_t p_value, const uint8_t p_mask);
            mp_on_write_register(p_param1, p_param2, p_param3);
            break;
        }

        case MSG_WRITE_REGISTER_BIT:
        {
            // param1: register address (0-255)
            // param2: bit num (0-7)
            // param3: value (false, true)
            // void mp_on_write_register_bit(const uint8_t p_registerAddress, const uint8_t p_bit, const bool p_state);
            if(p_param2 < 0x08)
            {
                mp_on_write_register_bit(p_param1, p_param2, (0x00 != p_param3));
            }
            break;
        }

        case MSG_PULSE_REGISTER_BIT:
        {
            // param1: register address (0-255)
            // param2: bit num (0-7)
            // param3: duration  (0-255ms)
            // void mp_on_pulse_register_bit(const uint8_t p_registerAddress, const uint8_t p_bit, const uint8_t p_durationMs);
            if(p_param2 < 0x08)
            {
                mp_on_pulse_register_bit(p_param1, p_param2, p_param3);
            }
            break;
        }

        case MSG_SUBSCRIBE_REGISTER:
        {
            // param1: register address (0-255)
            // param2: value (0-255)
            // param3: cancel (false, true)
            // void mp_on_subscribe_register(const uint8_t p_registerAddress, const uint8_t p_value, const bool p_cancel);
            mp_on_subscribe_register(p_param1, p_param2, (0x00 != p_param3));
            break;
        }

        default:
        {
            break;
        }

    }
}
