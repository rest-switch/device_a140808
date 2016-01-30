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

#ifndef __ring_buffer_h__
#define __ring_buffer_h__

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>  // printf
#include <stdlib.h>

#include "util.h"


struct ring_buf_data
{
    uint8_t* buff;   // the internal buffer used for storing elements in the ring buffer
    uint8_t* end;    // the internal buffer's end (end of the storage space)
    uint8_t* first;  // the virtual beginning of the ring buffer
    uint8_t* last;   // the virtual end of the ring buffer (one behind the last element)
    uint8_t  size;   // the number of items currently stored in the ring buffer
};


////////////////////////////////////////
inline bool rb_init(struct ring_buf_data* p_pd, const uint8_t p_capacity)
{
    p_pd->end = 0;
    p_pd->first = 0;
    p_pd->last = 0;
    p_pd->size = 0;

    p_pd->buff = (uint8_t*)malloc(p_capacity);
    if(0 == p_pd->buff)
    {
        // crit error, TODO: syslog this
        printf("malloc failed for capacity: [%d]\n", p_capacity);
        return(false);
    }

    p_pd->end = (p_pd->buff + p_capacity);
    p_pd->first = p_pd->last = p_pd->buff;

    return(true);
}

////////////////////////////////////////
inline void rb_free(struct ring_buf_data* p_pd)
{
    p_pd->end = 0;
    p_pd->first = 0;
    p_pd->last = 0;
    p_pd->size = 0;

    if(0 != p_pd->buff)
    {
        free(p_pd->buff);
        p_pd->buff = 0;
    }
}

////////////////////////////////////////
inline void rb_clear(struct ring_buf_data* p_pd)
{
    p_pd->first = p_pd->last = p_pd->buff;
    p_pd->size = 0;
}

////////////////////////////////////////
inline uint8_t rb_size(struct ring_buf_data* p_pd)
{
    return(p_pd->size);
}

////////////////////////////////////////
inline bool rb_empty(struct ring_buf_data* p_pd)
{
    return(p_pd->size < 1);
}

////////////////////////////////////////
inline uint8_t rb_capacity(struct ring_buf_data* p_pd)
{
    return(p_pd->end - p_pd->buff);
}

////////////////////////////////////////
inline bool rb_full(struct ring_buf_data* p_pd)
{
    return(rb_capacity(p_pd) == p_pd->size);
}

////////////////////////////////////////
inline void rb_push_back(struct ring_buf_data* p_pd, const uint8_t p_item)
{
    if(rb_full(p_pd))
    {
        if(rb_empty(p_pd))
        {
            return;  // no capacity
        }
        *p_pd->last = p_item;
        // increment
        if(++p_pd->last == p_pd->end) p_pd->last = p_pd->buff;
        p_pd->first = p_pd->last;
    }
    else
    {
        *p_pd->last = p_item;
        // increment
        if(++p_pd->last == p_pd->end) p_pd->last = p_pd->buff;
        ++p_pd->size;
    }
}

////////////////////////////////////////
inline uint8_t rb_pop_front(struct ring_buf_data* p_pd)
{
    if(rb_empty(p_pd))
    {
        return(0); // error (or no such data)
    }
    const uint8_t item = *p_pd->first;
    // increment
    if(++p_pd->first == p_pd->end) p_pd->first = p_pd->buff;
    --p_pd->size;
    return(item);
}

////////////////////////////////////////
inline uint8_t rb_pop_back(struct ring_buf_data* p_pd)
{
    if(rb_empty(p_pd))
    {
        return(0); // error (or no such data)
    }
    // decrement
    if(p_pd->last == p_pd->buff) p_pd->last = p_pd->end;
    --p_pd->last;

    --p_pd->size;
    return(*p_pd->last);
}

////////////////////////////////////////
inline uint8_t rb_at(struct ring_buf_data* p_pd, const uint8_t p_index)
{
    if(p_index >= p_pd->size)
    {
        return(0); // error
    }

    if(p_index < (p_pd->end - p_pd->first))
    {
        return(*(p_pd->first + p_index));
    }
    return(*(p_pd->first + (p_index - rb_capacity(p_pd))));
}

////////////////////////////////////////
inline void rb_set_data(struct ring_buf_data* p_pd, const void* p_data, const uint8_t p_len)
{
    const uint8_t* pdata = (const uint8_t*)p_data;

    // copy the whole buffer (will ignore overflow)
    for(uint8_t i=0; i<p_len; ++i)
    {
        const uint8_t b = pdata[i];
        rb_push_back(p_pd, b);
    }
}

////////////////////////////////////////
inline uint8_t rb_get_data(struct ring_buf_data* p_pd, void* p_data, const uint8_t p_len)
{
    uint8_t* pdata = (uint8_t*)p_data;

    // copy as much as we can
    const uint8_t len = min(p_pd->size, p_len);
    for(uint8_t i=0; i<len; ++i)
    {
        pdata[i] = rb_pop_front(p_pd);
    }

    return(len);
}

#endif // __ring_buffer_h__
