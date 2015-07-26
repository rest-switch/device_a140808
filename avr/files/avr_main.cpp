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
// avr main
//

#include <avr/interrupt.h>
#include <util/delay.h>

#include "msg_processor.h"

void avr_init(void);  // from avr_impl.cpp


////////////////////////////////////////
int main(void)
{
    avr_init();

    // create the message pump
    MsgProcessor mp;
    if(!mp.init(0, 0xE100, true))  // 57,600
    {
        return(1);
    }

    // enable global interrupts
    // http://winavr.scienceprog.com/avr-gcc-tutorial/interrupt-driven-avr-usart-communication.html
    //set_sleep_mode(SLEEP_MODE_IDLE);
    sei();

    for(;;)
    {
        // check for new messages
        mp.poll();
        _delay_ms(100);
    }

    return(0);
}
