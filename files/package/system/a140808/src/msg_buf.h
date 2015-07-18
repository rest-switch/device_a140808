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

#include <stdbool.h>

#include "ring_buf.h"


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
#define RING_BUF_COUNT  14


// error codes
#define S_OK                  0
#define S_INCOMPLETE_BUFFER   1
#define E_BAD_FRAME          -1
#define E_BAD_CRC            -2

#define MSG_BEGIN_CHAR '['
#define MSG_END_CHAR   ']'

#define DEC2HEX(dc)  ((uint8_t)(((dc)>=0 && (dc)<=9) ? (dc)+'0' : (((dc)>=10 && (dc)<=15) ? (dc)-10+'a' : 'z')))
#define HEX2DEC(hx)  ((uint8_t)(((hx)>='0' && (hx)<='9') ? (hx)-'0' : (((hx)>='A' && (hx)<='F') ? (hx)-'A'+10 : (((hx)>='a' && (hx)<='f') ? (hx)-'a'+10 : 0))))
#define ISHEXCH(ch)  (((ch)>='0' && (ch)<='9') || ((ch)>='A' && (ch)<='F') || ((ch)>='a' && (ch)<='f'))

inline uint8_t mb_validate(struct ring_buf_data* p_pd);
inline uint16_t mb_get_crc(struct ring_buf_data* p_pd);
inline uint16_t mb_compute_crc(struct ring_buf_data* p_pd);


////////////////////////////////////////
inline bool mb_init(struct ring_buf_data* p_pd)
{
    return(rb_init(p_pd, RING_BUF_COUNT));
}

////////////////////////////////////////
inline void mb_free(struct ring_buf_data* p_pd)
{
    rb_free(p_pd);
}

////////////////////////////////////////
inline bool mb_get_bytes(struct ring_buf_data* p_pd, uint8_t* p_val0, uint8_t* p_val1, uint8_t* p_val2, uint8_t* p_val3)
{
    *p_val0 = 0;
    *p_val1 = 0;
    *p_val2 = 0;
    *p_val3 = 0;

    if(S_OK != mb_validate(p_pd))
    {
        return(false);
    }

    *p_val0 = ((HEX2DEC(rb_at(p_pd, 1)) << 4) | HEX2DEC(rb_at(p_pd, 2)));
    *p_val1 = ((HEX2DEC(rb_at(p_pd, 3)) << 4) | HEX2DEC(rb_at(p_pd, 4)));
    *p_val2 = ((HEX2DEC(rb_at(p_pd, 5)) << 4) | HEX2DEC(rb_at(p_pd, 6)));
    *p_val3 = ((HEX2DEC(rb_at(p_pd, 7)) << 4) | HEX2DEC(rb_at(p_pd, 8)));

    return(true);
}

////////////////////////////////////////
inline void mb_set_bytes(struct ring_buf_data* p_pd, const uint8_t p_val0, const uint8_t p_val1, const uint8_t p_val2, const uint8_t p_val3)
{
    rb_clear(p_pd);
    rb_push_back(p_pd, MSG_BEGIN_CHAR);                  // byte  0

    rb_push_back(p_pd, DEC2HEX((p_val0>>4) & 0x0f));     // byte  1
    rb_push_back(p_pd, DEC2HEX( p_val0     & 0x0f));     // byte  2
    rb_push_back(p_pd, DEC2HEX((p_val1>>4) & 0x0f));     // byte  3
    rb_push_back(p_pd, DEC2HEX( p_val1     & 0x0f));     // byte  4
    rb_push_back(p_pd, DEC2HEX((p_val2>>4) & 0x0f));     // byte  5
    rb_push_back(p_pd, DEC2HEX( p_val2     & 0x0f));     // byte  6
    rb_push_back(p_pd, DEC2HEX((p_val3>>4) & 0x0f));     // byte  7
    rb_push_back(p_pd, DEC2HEX( p_val3     & 0x0f));     // byte  8

    const uint16_t crc = mb_compute_crc(p_pd);
    rb_push_back(p_pd, DEC2HEX((crc  >>12) & 0x0f));     // byte  9
    rb_push_back(p_pd, DEC2HEX((crc  >> 8) & 0x0f));     // byte 10
    rb_push_back(p_pd, DEC2HEX((crc  >> 4) & 0x0f));     // byte 11
    rb_push_back(p_pd, DEC2HEX( crc        & 0x0f));     // byte 12

    rb_push_back(p_pd, MSG_END_CHAR);                    // byte 13
}

////////////////////////////////////////
inline uint8_t mb_validate(struct ring_buf_data* p_pd)
{
    // to be valid, we need 14 chars
    if(rb_size(p_pd) < 14)
    {
        // not a big deal as the message may still be coming in
        return(S_INCOMPLETE_BUFFER);
    }

    // check begin and end markers
    if((MSG_BEGIN_CHAR != rb_at(p_pd, 0)) || (MSG_END_CHAR != rb_at(p_pd, 13)))
    {
        return(E_BAD_FRAME);
    }

    // check crc
    if(mb_get_crc(p_pd) != mb_compute_crc(p_pd))
    {
        return(E_BAD_CRC);
    }

    return(S_OK);
}

////////////////////////////////////////
inline uint16_t mb_update_crc16(const uint16_t p_crc, const uint8_t p_ch)
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
inline uint16_t mb_get_crc(struct ring_buf_data* p_pd)
{
    // stored in bytes 9-12
    return( (((uint16_t)HEX2DEC(rb_at(p_pd,  9))) << 12) |
            (((uint16_t)HEX2DEC(rb_at(p_pd, 10))) <<  8) |
            (((uint16_t)HEX2DEC(rb_at(p_pd, 11))) <<  4) |
             ((uint16_t)HEX2DEC(rb_at(p_pd, 12)))        );
}

////////////////////////////////////////
inline uint16_t mb_compute_crc(struct ring_buf_data* p_pd)
{
    // crc of bytes 1-8
    uint16_t crc = 0xffff;
    crc = mb_update_crc16(crc, rb_at(p_pd, 1));
    crc = mb_update_crc16(crc, rb_at(p_pd, 2));
    crc = mb_update_crc16(crc, rb_at(p_pd, 3));
    crc = mb_update_crc16(crc, rb_at(p_pd, 4));
    crc = mb_update_crc16(crc, rb_at(p_pd, 5));
    crc = mb_update_crc16(crc, rb_at(p_pd, 6));
    crc = mb_update_crc16(crc, rb_at(p_pd, 7));
    crc = mb_update_crc16(crc, rb_at(p_pd, 8));
    return(crc);
}

#endif // __msg_buf_h__
