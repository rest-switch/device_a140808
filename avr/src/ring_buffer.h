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
#include <stdlib.h>


////////////////////////////////////////////////////////////
class RingBuffer
{
public:
    ////////////////////////////////////////
    RingBuffer(const uint8_t p_capacity)
      : m_buff(0), m_end(0), m_first(0), m_last(0), m_size(0)
    {
        m_buff = (uint8_t*)::malloc(p_capacity);
        if(0 != m_buff)
        {
            m_end = (m_buff + p_capacity);
            m_first = m_last = m_buff;
        }
    }

    ////////////////////////////////////////
    ~RingBuffer(void)
    {
        if(0 != m_buff)
        {
            ::free(m_buff);
            m_buff = 0;
        }
        m_end = 0;
        m_first = 0;
        m_last = 0;
        m_size = 0;
    }

    ////////////////////////////////////////
    void clear(void)
    {
        m_first = m_last = m_buff;
        m_size = 0;
    }

    ////////////////////////////////////////
    uint8_t size(void) const
    {
        return(m_size);
    }

    ////////////////////////////////////////
    bool empty(void) const
    {
        return(0 == m_size);
    }

    ////////////////////////////////////////
    bool full(void) const
    {
        return(capacity() == m_size);
    }

    ////////////////////////////////////////
    uint8_t capacity(void) const
    {
        return(m_end - m_buff);
    }

    ////////////////////////////////////////
    void push_back(const uint8_t p_item)
    {
        if(full())
        {
            if(empty())
            {
                return;  // no capacity
            }
            *m_last = p_item;
            increment(m_last);
            m_first = m_last;
        }
        else
        {
            *m_last = p_item;
            increment(m_last);
            ++m_size;
        }
    }

    ////////////////////////////////////////
    uint8_t pop_front(void)
    {
        if(empty())
        {
            return(0); // error
        }
        const uint8_t item = *m_first;
        increment(m_first);
        --m_size;
        return(item);
    }

    ////////////////////////////////////////
    uint8_t pop_back(void)
    {
        if(empty())
        {
            return(0); // error
        }
        decrement(m_last);
        --m_size;
        return(*m_last);
    }

    ////////////////////////////////////////
    uint8_t operator[](const uint8_t p_index) const
    {
        if(p_index >= m_size)
        {
            return(0); // error
        }

        if(p_index < (m_end - m_first))
        {
            return(*(m_first + p_index));
        }
        return(*(m_first + (p_index - capacity())));
//        return(*(m_first + (p_index < (m_end - m_first) ? p_index : p_index - capacity())));
    }

    ////////////////////////////////////////
    uint8_t at(const uint8_t p_index) const
    {
        return((*this)[p_index]);
    }


private:
    uint8_t* m_buff;  // the internal buffer used for storing elements in the ring buffer
    uint8_t* m_end;   // the internal buffer's end (end of the storage space)
    uint8_t* m_first; // the virtual beginning of the ring buffer
    uint8_t* m_last;  // the virtual end of the ring buffer (one behind the last element)
    uint8_t m_size;   // the number of items currently stored in the ring buffer

    // increment the pointer
    void increment(uint8_t*& p_ptr) const
    {
        if(++p_ptr == m_end)
        {
            p_ptr = m_buff;
        }
    }

    // decrement the pointer
    void decrement(uint8_t*& p_ptr) const
    {
        if(p_ptr == m_buff)
        {
            p_ptr = m_end;
        }
        --p_ptr;
    }
};

#endif // __ring_buffer_h__
