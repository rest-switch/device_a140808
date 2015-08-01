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

#ifndef __ticks_h__
#define __ticks_h__

#include <avr/io.h>
#include <avr/interrupt.h>


////////////////////////////////////////////////////////////
namespace ticks
{

namespace
{
    volatile uint16_t s_ticks = 0;  // 65,536ms = ~65 seconds

    ////////////////////////////////////////
    SIGNAL(TIMER0_COMPA_vect)
    {
        // reset the count value of the timer
        TCNT0 = 0x00;
        ++s_ticks;
    }
} // anonymous namespace


////////////////////////////////////////
inline uint16_t get(void)
{
    return(s_ticks);
}


////////////////////////////////////////
inline void delay(const uint16_t p_delayMs)
{
    for(const uint16_t start = ticks::get(); ((ticks::get() - start) < p_delayMs); );
}


////////////////////////////////////////
void init(void)
{
    TCCR0A = 0x00;
    TCCR0B = _BV(WGM01);  // timer 0 Clear Timer on Compare match (CTC) mode

    // reset the count value of the timer
    TCNT0 = 0x00;

    // output compare register A (OCR0A)
    // interrupt fires when this counter value is met
    OCR0A = 0xFA;  // 1 ms using a 1/64 prescaler: 16MHz/64/250 = 250,000/250 = 1000

    // enable timer/counter 0 compareA match interrupt enable
    TIMSK0 |= _BV(OCIE0A);

    // CS02 CS01 CS00  Description
    //   0    0    0   No clock source (Timer/Counter stopped)
    //   0    0    1   clkI/O/1 (No prescaling)
    //   0    1    0   clkI/O/8 (From prescaler)
    //   0    1    1   clkI/O/64 (From prescaler)
    //   1    0    0   clkI/O/256 (From prescaler)
    //   1    0    1   clkI/O/1024 (From prescaler)
    //   1    1    0   External clock source on T0 pin. Clock on falling edge.
    //   1    1    1   External clock source on T0 pin. Clock on rising edge.
    //
    // prescaler = 64
    TCCR0B |= (_BV(CS01) | _BV(CS00));
}

} // namespace ticks

#endif // __ticks_h__
