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

#ifndef __msg_buf_h__
#define __msg_buf_h__

#include "ring_buffer.h"


//
// message format
// to be valid, we need 14 chars
//
// | [ | x | x | x | x | x | x | x | x | c | c | c | c | ] |
// +---+---+---+---+---+---+---+---+---+---+---+---+---|---+
// | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | a | b | c | d |
//
//   [        = begin message
//   xxxxxxxx = message payload, 8 chars  (hex 0-9, a-f)
//   cccc     = crc of bytes 1-8 (hex 0-9, a-f)
//   ]        = end message
//
#define RING_BUF_COUNT 14


// error codes
#define S_OK                  0
#define S_INCOMPLETE_BUFFER   1
#define E_BAD_FRAME          -1
#define E_BAD_CRC            -2

#define MSG_BEGIN_CHAR '['
#define MSG_END_CHAR   ']'

#define DEC2HEX(dc)  ((uint8_t)(((dc)>=0 && (dc)<=9) ? (dc)+'0' : (((dc)>=10 && (dc)<=15) ? (dc)-10+'a': 'z')))
#define HEX2DEC(hx)  ((uint8_t)(((hx)>='0' && (hx)<='9') ? (hx)-'0' : (((hx)>='A' && (hx)<='F') ? (hx)-'A'+10 : (((hx)>='a' && (hx)<='f') ? (hx)-'a'+10 : 0))))
#define ISHEXCH(ch)  (((ch)>='0' && (ch)<='9') || ((ch)>='A' && (ch)<='F') || ((ch)>='a' && (ch)<='f'))



////////////////////////////////////////////////////////////
class MsgBuf : public RingBuffer
{
public:
    ////////////////////////////////////////
    MsgBuf(void) : RingBuffer(RING_BUF_COUNT)
    {
    }

    ////////////////////////////////////////
    ~MsgBuf(void)
    {
    }

    ////////////////////////////////////////
    bool get_bytes(uint8_t& p_val0, uint8_t& p_val1, uint8_t& p_val2, uint8_t& p_val3) const
    {
        p_val0 = 0;
        p_val1 = 0;
        p_val2 = 0;
        p_val3 = 0;

        if(S_OK != validate())
        {
            return(false);
        }

        p_val0 = ((HEX2DEC(at(1)) << 4) | HEX2DEC(at(2)));
        p_val1 = ((HEX2DEC(at(3)) << 4) | HEX2DEC(at(4)));
        p_val2 = ((HEX2DEC(at(5)) << 4) | HEX2DEC(at(6)));
        p_val3 = ((HEX2DEC(at(7)) << 4) | HEX2DEC(at(8)));

        return(true);
    }

    ////////////////////////////////////////
    void set_bytes(const uint8_t p_val0, const uint8_t p_val1, const uint8_t p_val2, const uint8_t p_val3)
    {
        clear();
        push_back(MSG_BEGIN_CHAR);                  // byte  0

        push_back(DEC2HEX((p_val0>>4) & 0x0f));     // byte  1
        push_back(DEC2HEX( p_val0     & 0x0f));     // byte  2
        push_back(DEC2HEX((p_val1>>4) & 0x0f));     // byte  3
        push_back(DEC2HEX( p_val1     & 0x0f));     // byte  4
        push_back(DEC2HEX((p_val2>>4) & 0x0f));     // byte  5
        push_back(DEC2HEX( p_val2     & 0x0f));     // byte  6
        push_back(DEC2HEX((p_val3>>4) & 0x0f));     // byte  7
        push_back(DEC2HEX( p_val3     & 0x0f));     // byte  8

        const uint16_t crc = compute_crc();
        push_back(DEC2HEX((crc  >>12) & 0x0f));     // byte  9
        push_back(DEC2HEX((crc  >> 8) & 0x0f));     // byte 10
        push_back(DEC2HEX((crc  >> 4) & 0x0f));     // byte 11
        push_back(DEC2HEX( crc        & 0x0f));     // byte 12

        push_back(MSG_END_CHAR);                    // byte 13
    }

    ////////////////////////////////////////
    uint8_t validate(void) const
    {
        // to be valid, we need 14 chars
        if(size() < 14)
        {
            // not a big deal as the message may still be coming in
            return(S_INCOMPLETE_BUFFER);
        }

        // check begin and end markers
        if((MSG_BEGIN_CHAR != at(0)) || (MSG_END_CHAR != at(13)))
        {
            return(E_BAD_FRAME);
        }

        // check crc
        if(get_crc() != compute_crc())
        {
            return(E_BAD_CRC);
        }

        return(S_OK);
    }

private:
    ////////////////////////////////////////
    uint16_t update_crc16(const uint16_t p_crc, const uint8_t p_ch) const
    {
        uint16_t crc = (p_crc ^ (uint16_t)p_ch);
        crc = ((0 == (crc & 0x0001)) ? (crc >> 1) : ((crc >> 1) ^ 0xa001));
        crc = ((0 == (crc & 0x0001)) ? (crc >> 1) : ((crc >> 1) ^ 0xa001));
        crc = ((0 == (crc & 0x0001)) ? (crc >> 1) : ((crc >> 1) ^ 0xa001));
        crc = ((0 == (crc & 0x0001)) ? (crc >> 1) : ((crc >> 1) ^ 0xa001));
        crc = ((0 == (crc & 0x0001)) ? (crc >> 1) : ((crc >> 1) ^ 0xa001));
        crc = ((0 == (crc & 0x0001)) ? (crc >> 1) : ((crc >> 1) ^ 0xa001));
        crc = ((0 == (crc & 0x0001)) ? (crc >> 1) : ((crc >> 1) ^ 0xa001));
        crc = ((0 == (crc & 0x0001)) ? (crc >> 1) : ((crc >> 1) ^ 0xa001));
        return(crc);
    }
    ////////////////////////////////////////
    uint16_t get_crc(void) const
    {
        // stored in bytes 9-12
        return( (((uint16_t)HEX2DEC(at( 9))) << 12) |
                (((uint16_t)HEX2DEC(at(10))) <<  8) |
                (((uint16_t)HEX2DEC(at(11))) <<  4) |
                 ((uint16_t)HEX2DEC(at(12)))        );
    }
    ////////////////////////////////////////
    uint16_t compute_crc(void) const
    {
        // crc of bytes 1-8
        uint16_t crc = 0xffff;
        crc = update_crc16(crc, at(1));
        crc = update_crc16(crc, at(2));
        crc = update_crc16(crc, at(3));
        crc = update_crc16(crc, at(4));
        crc = update_crc16(crc, at(5));
        crc = update_crc16(crc, at(6));
        crc = update_crc16(crc, at(7));
        crc = update_crc16(crc, at(8));
        return(crc);
    }
};

#endif // __msg_buf_h__
