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

#ifndef __one_shot_timer_h__
#define __one_shot_timer_h__

#include <avr/io.h>
#include <avr/interrupt.h>


////////////////////////////////////////
// long running async operation
// activate with OneShotTimer::start(p_sec);
//void OneShotTimer::on_timer_expired(void)
//{
//}


////////////////////////////////////////////////////////////
namespace OneShotTimer
{
    // callback event
    void on_timer_expired(void);

    static uint8_t s_elapsedSec = 0;
    static uint8_t s_timeoutSec = 0;


    ////////////////////////////////////////
    void start(const uint8_t p_timeoutSec)
    {
        s_elapsedSec = 0;
        s_timeoutSec = p_timeoutSec;

        TCCR1A = 0x00;
        TCCR1B = _BV(WGM12);  // timer 1 CTC mode
        //TCCR1C = 0x00;

        // reset the count value of the timer
        TCNT1 = 0x0000;

        // output compare register A (OCR1A)
        // interrupt fires when this counter value is met
        OCR1A = 0x3D09;  // 1 second using a 1024 prescaler at 16MHz

        // enable timer/counter 1 compareA match interrupt enable
        //TIMSK1 |= _BV(OCIE1A);
        TIMSK |= _BV(OCIE1A);

        // CS12 CS11 CS10  Description
        //   0    0    0   No clock source (Timer/Counter stopped).
        //   0    0    1   clkI/O/1 (No prescaling)
        //   0    1    0   clkI/O/8 (From prescaler)
        //   0    1    1   clkI/O/64 (From prescaler)
        //   1    0    0   clkI/O/256 (From prescaler)
        //   1    0    1   clkI/O/1024 (From prescaler)
        //   1    1    0   External clock source on T1 pin. Clock on falling edge.
        //   1    1    1   External clock source on T1 pin. Clock on rising edge.
        //
        // prescaler = 1024
        // at 16MHz 15,625 ticks pass in 1 second
        TCCR1B |= (_BV(CS12) | _BV(CS10));
    }

    ////////////////////////////////////////
    void stop(void)
    {
        // timer/counter 1 stopped
        TCCR1A = 0x00;
        TCCR1B = 0x00;
        //TCCR1C = 0x00;

        // disable timer/counter 1 compare A match interrupt enable
        //TIMSK1 &= ~_BV(OCIE1A);
        TIMSK &= ~_BV(OCIE1A);

        // clear compare A match flag (output compare register A [OCR1A])
        //TIFR1 |= _BV(OCF1A);
        TIFR |= _BV(OCF1A);

        s_elapsedSec = 0;
        s_timeoutSec = 0;
    }

namespace
{
    ////////////////////////////////////////
//    SIGNAL(TIMER1_COMPA_vect)
    ISR(SIG_OUTPUT_COMPARE1A)
    {
        ++s_elapsedSec;
        if(s_elapsedSec >= s_timeoutSec)
        {
            on_timer_expired();
            stop();
        }
    }
} // anonymous namespace

} // namespace OneShotTimer

#endif // __one_shot_timer_h__
