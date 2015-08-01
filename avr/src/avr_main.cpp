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
#include <avr/io.h>

#include "msg_processor.h"

void avr_init(void);  // from avr_impl.cpp

////////////////////////////////////////
FUSES =
{
    (FUSE_BODEN & FUSE_BODLEVEL),  // low fuses: 0x3f
    (FUSE_BOOTSZ0 & FUSE_BOOTSZ1 & FUSE_CKOPT & FUSE_SPIEN), // high fuses: 0xc9
};

// --------------------------------------------------------
// LB_MODE_3   (1111:1100)
// Further programming and verification of the Flash and 
// EEPROM is disabled in Parallel and SPI/JTAG Serial 
// Programming mode. The Fuse bits are locked in both 
// Serial and Parallel Programming mode.
//
// BLB0_MODE_4 (1111:0111)
// LPM executing from the Boot
// Loader section is not allowed to read from the 
// Application section. If interrupt vectors are 
// placed in the Boot Loader section, interrupts are 
// disabled while executing from the Application section.
//
// BLB1_MODE_4 (1101:1111)
// LPM executing from the Application section is not allowed 
// to read from the Boot Loader section. If interrupt vectors 
// are placed in the Application section, interrupts are 
// disabled while executing from the Boot Loader section.
//
// or LOCKBITS_DEFAULT  (1111:1111)
// --------------------------------------------------------
LOCKBITS = (LB_MODE_3 & BLB0_MODE_4 & BLB1_MODE_4); // lock: 0xd4


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
