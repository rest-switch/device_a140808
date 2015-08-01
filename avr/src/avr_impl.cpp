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

//
//  avr_impl.cpp
//
//  Created by John Clark on 1/18/14.
//

#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>

//#define USE_RS485_RTS 1
#include "msg_processor.h"


//  a140808       ATmega32
//  Opti-In 1     PORTD.5
//  Opti-In 2     PORTD.7
//  Opti-In 3     PORTC.3
//  Opti-In 4     PORTC.2
//  Opti-In 5     PORTC.4
//  Opti-In 6     PORTC.5
//  Opti-In 7     PORTC.6
//  Opti-In 8     PORTC.7
//  Relay 1       PORTA.3
//  Relay 2       PORTA.2
//  Relay 3       PORTA.1
//  Relay 4       PORTA.0
//  Relay 5       PORTA.4
//  Relay 6       PORTA.5
//  Relay 7       PORTA.6
//  Relay 8       PORTA.7
//  RX Data       PORTD.0
//  TX Data       PORTD.1
//  RS485 TX
//  SPI MOSI      PORTB.5
//  SPI MISO      PORTB.6
//  SPI SCK       PORTB.7


//                   (inverted logic)  PORTC 7:4          PORTC.2                 PORTC.3                 PORTD.7                 PORTD.5
#define READ_DIGITAL_INPUTS       ( (~PINC & 0xf0) | ((~PINC & 0x04) << 1) | ((~PINC & 0x08) >> 1) | ((~PIND & 0x80) >> 6) | ((~PIND & 0x20) >> 5) )
//                   (inverted logic)        high nibble                  --- low nibble is reversed ---
#define READ_DIGITAL_OUTPUTS      ( (~PORTA & 0xf0) | ((~PORTA & 0x08) >> 3) | ((~PORTA & 0x04) >> 1) | ((~PORTA & 0x02) << 1) | ((~PORTA & 0x01) << 3) )
//                   (inverted logic)        high nibble                  --- low nibble is reversed ---
#define WRITE_DIGITAL_OUTPUTS(b)  PORTA = ~( ((b) & 0xf0) | (((b) & 0x08) >> 3) | (((b) & 0x04) >> 1) | (((b) & 0x02) << 1) | (((b) & 0x01) << 3) )
//                   (inverted logic)                     current state               new state
#define WRITE_DIGITAL_OUTPUTS_MASKED(b, m)  WRITE_DIGITAL_OUTPUTS( (READ_DIGITAL_OUTPUTS & ~(m)) | ((b) & (m)) )
//                   (inverted logic)
#define CLEAR_DIGITAL_OUTPUT_BIT(b)   PORTA |=  (1 << ( ((b)<5) ? (4-(b)) : ((b)-1) ))
#define SET_DIGITAL_OUTPUT_BIT(b)     PORTA &= ~(1 << ( ((b)<5) ? (4-(b)) : ((b)-1) ))
#define TOGGLE_DIGITAL_OUTPUT_BIT(b)  PORTA ^=  (1 << ( ((b)<5) ? (4-(b)) : ((b)-1) ))
// 1-based relay num to bit decoder
//#define RELAY_NUM_TO_BIT(b)  (1 << (((b)<5) ? (4-(b)) : ((b)-1)))
#define IS_DIGITAL_OUTPUT_BIT_SET(x)  (_BV(x) == (READ_DIGITAL_OUTPUTS & _BV(x)))


////////////////////////////////////////
// long running async operation
// #include "one_shot_timer.h"
// activate with OneShotTimer::start(p_sec);
//void OneShotTimer::on_timer_expired(void)
//{
//}

struct Subscription
{
    Subscription(void) : m_isSubscribed(false), m_value(0) { }
    bool m_isSubscribed;
    uint8_t m_value;
};
static Subscription s_input;
static Subscription s_output;


////////////////////////////////////////
void avr_init(void)
{
    // configure A140808 inputs:
    //  Opti-In 1     PORTD.5
    //  Opti-In 2     PORTD.7
    //  Opti-In 3     PORTC.3
    //  Opti-In 4     PORTC.2
    //  Opti-In 5     PORTC.4
    //  Opti-In 6     PORTC.5
    //  Opti-In 7     PORTC.6
    //  Opti-In 8     PORTC.7
    PORTC |= 0xfc;  // set port c.2 to  c.7 to logic 1 (enable pullups)
    PORTD |= 0xa0;  // set port d.5 and d.7 to logic 1 (enable pullups)
    DDRC  &= 0x03;  // set ddr  c.2 to  c.7 to logic 0 (input)
    DDRD  &= 0x5f;  // set ddr  d.5 and d.7 to logic 0 (input)

    // configure A140808 outputs
    //  Relay 1       PORTA.3
    //  Relay 2       PORTA.2
    //  Relay 3       PORTA.1
    //  Relay 4       PORTA.0
    //  Relay 5       PORTA.4
    //  Relay 6       PORTA.5
    //  Relay 7       PORTA.6
    //  Relay 8       PORTA.7
    PORTA = 0xff;  // set port a to logic 1 (relays off)
    DDRA |= 0xff;  // set ddr  a to logic 1 (output)
}

////////////////////////////////////////
void on_poll(MsgProcessor& p_mp)
{
    // our timeslice
    if(s_input.m_isSubscribed)
    {
        const uint8_t inputs = READ_DIGITAL_INPUTS;
        if(inputs != s_input.m_value)
        {
            // value changed
            s_input.m_value = inputs;
            p_mp.dispatch_subscribe_register(REG_INPUT_1, inputs);
        }
    }
    if(s_output.m_isSubscribed)
    {
        const uint8_t outputs = READ_DIGITAL_OUTPUTS;
        if(outputs != s_output.m_value)
        {
            // value changed
            s_output.m_value = outputs;
            p_mp.dispatch_subscribe_register(REG_OUTPUT_1, outputs);
        }
    }
}

////////////////////////////////////////
void on_pong(MsgProcessor& p_mp, const uint8_t p_param1, const uint8_t p_param2, const uint8_t p_param3)
{
    // we got our ping back but nothing to do
    // the AVR does not send pings anyway
}

////////////////////////////////////////
void on_read_register(MsgProcessor& p_mp, const uint8_t p_registerAddress)
{
    switch(p_registerAddress)
    {
        case REG_INPUT_1:
        {
            const uint8_t inputs = READ_DIGITAL_INPUTS;
            p_mp.dispatch_write_register(REG_INPUT_1, inputs);
            break;
        }
        case REG_OUTPUT_1:
        {
            const uint8_t outputs = READ_DIGITAL_OUTPUTS;
            p_mp.dispatch_write_register(REG_OUTPUT_1, outputs);
            break;
        }
        default:
        {
            p_mp.dispatch_write_register(REG_ERR_UNKNOWN);
            break;
        }
    }
}

////////////////////////////////////////
void on_write_register(MsgProcessor& p_mp, const uint8_t p_registerAddress, const uint8_t p_value, const uint8_t p_mask)
{
    switch(p_registerAddress)
    {
        case REG_OUTPUT_1:
        {
            WRITE_DIGITAL_OUTPUTS_MASKED(p_value, p_mask);
            break;
        }
        default:
        {
            break;
        }
    }
}

////////////////////////////////////////
void on_write_register_bit(MsgProcessor& p_mp, const uint8_t p_registerAddress, const uint8_t p_bit, const bool p_state)
{
    switch(p_registerAddress)
    {
        case REG_OUTPUT_1:
        {
            if(p_state)
            {
                SET_DIGITAL_OUTPUT_BIT(p_bit);
            }
            else
            {
                CLEAR_DIGITAL_OUTPUT_BIT(p_bit);
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

////////////////////////////////////////
void on_pulse_register_bit(MsgProcessor& p_mp, const uint8_t p_registerAddress, const uint8_t p_bit, const uint8_t p_durationMs)
{
    switch(p_registerAddress)
    {
        case REG_OUTPUT_1:
        {
            // p_duration is in milli-seconds
            TOGGLE_DIGITAL_OUTPUT_BIT(p_bit);
            for(uint8_t i=p_durationMs; i>0; --i) _delay_ms(1);
            TOGGLE_DIGITAL_OUTPUT_BIT(p_bit);
            break;
        }
        default:
        {
            break;
        }
    }
}

////////////////////////////////////////
void on_subscribe_register(MsgProcessor& p_mp, const uint8_t p_registerAddress, const uint8_t p_value, const bool p_cancel)
{
    switch(p_registerAddress)
    {
        case REG_INPUT_1:
        {
            const uint8_t inputs = READ_DIGITAL_INPUTS;
            s_input.m_isSubscribed = !p_cancel;
            s_input.m_value = p_cancel ? 0 : inputs;
            p_mp.dispatch_subscribe_register(REG_INPUT_1, inputs, p_cancel);
            break;
        }
        case REG_OUTPUT_1:
        {
            const uint8_t outputs = READ_DIGITAL_OUTPUTS;
            s_output.m_isSubscribed = !p_cancel;
            s_output.m_value = p_cancel ? 0 : outputs;
            p_mp.dispatch_subscribe_register(REG_OUTPUT_1, outputs, p_cancel);
            break;
        }
        default:
        {
            p_mp.dispatch_subscribe_register(REG_ERR_UNKNOWN, 0, true);
            break;
        }
    }
}

