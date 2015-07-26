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

#ifndef __msg_processor_h__
#define __msg_processor_h__

#include "msg_buf.h"
#include "serial.h"


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
class MsgProcessor;
void on_poll(MsgProcessor& p_mp);
void on_pong(MsgProcessor& p_mp, const uint8_t p_param1, const uint8_t p_param2, const uint8_t p_param3);
void on_read_register(MsgProcessor& p_mp, const uint8_t p_registerAddress);
void on_write_register(MsgProcessor& p_mp, const uint8_t p_registerAddress, const uint8_t p_value, const uint8_t p_mask);
void on_write_register_bit(MsgProcessor& p_mp, const uint8_t p_registerAddress, const uint8_t p_bit, const bool p_state);
void on_pulse_register_bit(MsgProcessor& p_mp, const uint8_t p_registerAddress, const uint8_t p_bit, const uint8_t p_durationMs);
void on_subscribe_register(MsgProcessor& p_mp, const uint8_t p_registerAddress, const uint8_t p_value, const bool p_cancel);



////////////////////////////////////////////////////////////
class MsgProcessor
{
public:
    ////////////////////////////////////////
    MsgProcessor(void)
    {
    }

    ////////////////////////////////////////
//    ~MsgProcessor(void)
//    {
//    }

    ////////////////////////////////////////
    // p_parity
    //   false: N81 (none, 8 data, 1 stop)
    //   true:  E71 (even, 7 data, 1 stop)
    bool init(const char* p_device, const uint16_t p_baud, const bool p_parity)
    {
        return(m_serialPort.init(p_device, p_baud, p_parity));
    }

    ////////////////////////////////////////
    bool dispatch_ping(const uint8_t p_param1, const uint8_t p_param2, const uint8_t p_param3)
    {
        return(dispatch_message(MSG_PING, p_param1, p_param2, p_param3));
    }

    ////////////////////////////////////////
    bool dispatch_read_register(const uint8_t p_registerAddress)
    {
        return(dispatch_message(MSG_READ_REGISTER, p_registerAddress, 0x00, 0x00));
    }

    ////////////////////////////////////////
    bool dispatch_write_register(const uint8_t p_registerAddress, const uint8_t p_value=0x00, const uint8_t p_mask=0xff)
    {
        return(dispatch_message(MSG_WRITE_REGISTER, p_registerAddress, p_value, p_mask));
    }

    ////////////////////////////////////////
    bool dispatch_write_register_bit(const uint8_t p_registerAddress, const uint8_t p_bit, const bool p_state)
    {
        if(p_bit > 0x07)
        {
            return(false);
        }
        return(dispatch_message(MSG_WRITE_REGISTER_BIT, p_registerAddress, p_bit, (p_state ? 0xff : 0x00)));
    }

    ////////////////////////////////////////
    bool dispatch_pulse_register_bit(const uint8_t p_registerAddress, const uint8_t p_bit, const uint8_t p_durationMs)
    {
        if(p_bit > 0x07)
        {
            return(false);
        }
        return(dispatch_message(MSG_PULSE_REGISTER_BIT, p_registerAddress, p_bit, p_durationMs));
    }

    ////////////////////////////////////////
    bool dispatch_subscribe_register(const uint8_t p_registerAddress, const uint8_t p_value=0, const bool p_cancel=false)
    {
        return(dispatch_message(MSG_SUBSCRIBE_REGISTER, p_registerAddress, p_value, p_cancel));
    }

    ////////////////////////////////////////
    bool dispatch_message(const uint8_t p_type, const uint8_t p_param1, const uint8_t p_param2, const uint8_t p_param3)
    {
        m_msgBuf.set_bytes(p_type, p_param1, p_param2, p_param3);
        return(m_serialPort.write(m_msgBuf));
    }

    ////////////////////////////////////////
    void poll(void)
    {
        if(m_serialPort.read(m_msgBuf))
        {
            uint8_t type;
            uint8_t param1;
            uint8_t param2;
            uint8_t param3;
            if(!m_msgBuf.get_bytes(type, param1, param2, param3))
            {
                return;
            }

            process_message(type, param1, param2, param3);
        }

        on_poll(*this);
    }

private:
    MsgBuf m_msgBuf;
    SerialPort m_serialPort;

    ////////////////////////////////////////
    void process_message(const uint8_t p_type, const uint8_t p_param1, const uint8_t p_param2, const uint8_t p_param3)
    {
        switch(p_type)
        {
            case MSG_PING:
            {
                dispatch_message(MSG_PONG, p_param1, p_param2, p_param3);
                break;
            }

            case MSG_PONG:
            {
                on_pong(*this, p_param1, p_param2, p_param3);
                break;
            }

            case MSG_READ_REGISTER:
            {
                // param1: register address (0-255)
                // void on_read_register(MsgProcessor& p_mp, const uint8_t p_registerAddress);
                on_read_register(*this, p_param1);
                break;
            }

            case MSG_WRITE_REGISTER:
            {
                // param1: register address (0-255)
                // param2: value (0-255)
                // param3: mask (0-255)
                // void on_write_register(MsgProcessor& p_mp, const uint8_t p_registerAddress, const uint8_t p_value, const uint8_t p_mask);
                on_write_register(*this, p_param1, p_param2, p_param3);
                break;
            }

            case MSG_WRITE_REGISTER_BIT:
            {
                // param1: register address (0-255)
                // param2: bit num (0-7)
                // param3: value (false, true)
                // void on_write_register_bit(MsgProcessor& p_mp, const uint8_t p_registerAddress, const uint8_t p_bit, const bool p_state);
                if(p_param2 < 0x08)
                {
                    on_write_register_bit(*this, p_param1, p_param2, (0x00 != p_param3));
                }
                break;
            }

            case MSG_PULSE_REGISTER_BIT:
            {
                // param1: register address (0-255)
                // param2: bit num (0-7)
                // param3: duration  (0-255ms)
                // void on_pulse_register_bit(MsgProcessor& p_mp, const uint8_t p_registerAddress, const uint8_t p_bit, const bool p_duration);
                if(p_param2 < 0x08)
                {
                    on_pulse_register_bit(*this, p_param1, p_param2, p_param3);
                }
                break;
            }

            case MSG_SUBSCRIBE_REGISTER:
            {
                // param1: register address (0-255)
                // param2: value (0-255)
                // param3: cancel (false, true)
                // void on_subscribe_register(MsgProcessor& p_mp, const uint8_t p_registerAddress, const uint8_t p_value, const bool p_cancel);
                on_subscribe_register(*this, p_param1, p_param2, (0x00 != p_param3));
                break;
            }

            default:
            {
                break;
            }

        }
    }
};

#endif // __msg_processor_h__
