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

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "serial.h"
#include "ring_buffer.h"
#include "msg_buf.h"


// The Transmit Complete (TXCn) Flag bit is set one when the entire frame in the Transmit Shift
// Register has been shifted out and there are no new data currently present in the transmit buffer.
// The TXCn Flag bit is automatically cleared when a transmit complete interrupt is executed, or it
// can be cleared by writing a one to its bit location. The TXCn Flag is useful in half-duplex communication
// interfaces (like the RS-485 standard), where a transmitting application must enter
// receive mode and free the communication bus immediately after completing the transmission.
//#define USE_RS485_RTS
#ifdef USE_RS485_RTS
//#define RTS_PIN     PC5
//#define RTS_DDR     DDRC
//#define RTS_PORT    PORTC

inline void rts_init(void)
{
    RTS_DDR |= _BV(RTS_PIN);       // set rts pin as output
    RTS_PORT &= ~(_BV(RTS_PIN));   // pull pin low
    UCSRB |= _BV(TXCIE);           // enable tx complete interrupt (TXCIE)
}
inline void rts_uninit(void)
{
    RTS_PORT &= ~(_BV(RTS_PIN));   // pull pin low
    UCSRB &= ~_BV(TXCIE);          // disable tx complete interrupt (TXCIE)
}
inline void rts_high(void)
{
    RTS_PORT |= _BV(RTS_PIN);
}
inline void rts_low(void)
{
    RTS_PORT &= ~(_BV(RTS_PIN));
}

////////////////////////////////////////
// usart tx complete - see TXCIE
//SIGNAL(USART_TX_vect)
//ISR(SIG_USART_TRANS)
ISR(USART_TXC_vect)
{
    rts_low();
}
#endif // USE_RS485_RTS


// TODO?
// usart data register empty - see UDRIE
//SIGNAL(USART_UDRE_vect)
//ISR(SIG_USART_DATA)
//ISR(USART_UDRE_vect)

////////////////////////////////////////
// usart rx complete - see RXCIE
RingBuffer s_rx_buffer(128);
//SIGNAL(USART_RX_vect)
//ISR(SIG_USART_RECV)
ISR(USART_RXC_vect)
{
    unsigned char c = UDR;
    if(bit_is_clear(UCSRA, PE))
    {
        s_rx_buffer.push_back(c);
    }
}


//
// SerialPort impl
//

////////////////////////////////////////
SerialPort::SerialPort(void)
{
}

////////////////////////////////////////
SerialPort::~SerialPort(void)
{
    close();
}

////////////////////////////////////////
// p_device: ignored
// p_parity
//   false: N81 (none, 8 data, 1 stop)
//   true:  E71 (even, 7 data, 1 stop)
bool SerialPort::init(const char* /*p_device */, const uint16_t p_baud, const bool p_parity)
{
    //////////
    // UBRRL and UBRRH – USART Baud Rate Registers
    // bit 15: - URSEL: Register Select: This bit selects between accessing the UBRRH or the UCSRC Register. It is read as zero when reading UBRRH. The URSEL must be zero when writing the UBRRH.
    // bit 14:12 - Reserved: These bits are reserved for future use. For compatibility with future devices, these bit must be written to zero when UBRRH is written.
    // bit 11:0 - UBRR[11:0]: USART Baud Rate Register
    const uint16_t ubrr = (((F_CPU / 4 / p_baud) - 1) / 2);
    UBRRH = (ubrr >> 8);
    UBRRL = ubrr;

    //////////
    // UCSRA – USART Control and Status Register A
    // ---
    // bit 1 – U2X: Double the USART Transmission Speed
    UCSRA = _BV(U2X);
    // ---
    // bit 0 – MPCM: Multi-processor Communication Mode

    //////////
    // UCSRB – USART Control and Status Register B
    // ---
    // bit 7 – RXCIE: RX Complete Interrupt Enable
    uint8_t ucsrb = _BV(RXCIE);
    // ---
    // bit 6 – TXCIE: TX Complete Interrupt Enable
    //   see rts_init() below
    // ---
    // bit 5 – UDRIE: USART Data Register Empty Interrupt Enable
    //   TODO?
    // ---
    // bit 4 – RXEN: Receiver Enable
    ucsrb |= _BV(RXEN);
    // ---
    // bit 3 – TXEN: Transmitter Enable
    ucsrb |= _BV(TXEN);
    // ---
    // bit 2 – UCSZ2: Character Size
    // ---
    // bit 1 – RXB8: Receive Data Bit 8
    UCSRB = ucsrb;
    // RS485 (TXCIE)
    #ifdef USE_RS485_RTS
    rts_init();
    #endif // USE_RS485_RTS

    //////////
    // UCSRC – USART Control and Status Register C
    // ---
    // bit 7 – URSEL: Register Select
    // bits must be written at once with this flag set
    uint8_t ucsrc = _BV(URSEL);
    // ---
    // bit 6 - UMSEL: USART Mode Select
    // UMSEL Mode
    //    0    Asynchronous USART
    //    1    Synchronous USART
    // always asynchronous (0)
    // ---
    // Bits 5:4 - UPM1:0: Parity Mode
    // UPM1 UPM0 Parity Mode
    //  0    0   Disabled
    //  0    1   Reserved
    //  1    0   Enabled, Even Parity
    //  1    1   Enabled, Odd Parity
    if(p_parity) ucsrc |= _BV(UPM1);  // even parity
    // ucsrc |= (_BV(UPM1) | _BV(UPM0));  // odd parity
    // ---
    // bit 3 - USBS: Stop Bit Select
    // USBS Stop Bit(s)
    //  0    1-bit
    //  1    2-bit
    // always use 1 stop bit (0)
    // ---
    // bit 2:1 - UCSZ1:0: Character Size
    // UCSZ2 UCSZ1 UCSZ0 Character Size
    //   0     0     0      5-bit
    //   0     0     1      6-bit
    //   0     1     0      7-bit
    //   0     1     1      8-bit
    //   1     0     0      Reserved
    //   1     0     1      Reserved
    //   1     1     0      Reserved
    //   1     1     1      9-bit
    if(p_parity)
    {
        ucsrc |= _BV(UCSZ1);  // 7 data
    }
    else
    {
        ucsrc |= (_BV(UCSZ1) | _BV(UCSZ0));  // 8 data
    }
    // ---
    UCSRC = ucsrc;

    return(true);
}

////////////////////////////////////////
void SerialPort::close(void)
{
    #ifdef USE_RS485_RTS
    rts_uninit();
    #endif // USE_RS485_RTS

    // disable transmitter (TXEN)
    UCSRB &= ~_BV(TXEN);

    // disable receiver (RXEN) and rx complete interrupt (RXCIE)
    UCSRB &= ~(_BV(RXEN) | _BV(RXCIE));
}

////////////////////////////////////////
bool SerialPort::read(MsgBuf& p_msgBuf) const
{
    while(!s_rx_buffer.empty())
    {
        const uint8_t val = s_rx_buffer.pop_front();
        p_msgBuf.push_back(val);
        if(S_OK == p_msgBuf.validate())
        {
            // have a message
            return(true);
        }
    }
    return(false);
}

////////////////////////////////////////
bool SerialPort::write(MsgBuf& p_msgBuf) const
{
    if(S_OK != p_msgBuf.validate())
    {
        return(false);
    }

    #ifdef USE_RS485_RTS
    rts_high();
    #endif // USE_RS485_RTS

    for(uint8_t i=0, imax=p_msgBuf.size(); i<imax; ++i)
    {
        while(!(UCSRA & _BV(UDRE)));
        UDR = p_msgBuf[i];
    }
    return(true);
}

