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

#ifndef __serial_port_h__
#define __serial_port_h__

#include "msg_buf.h"


////////////////////////////////////////////////////////////
class SerialPort
{
public:
    SerialPort(void);
    ~SerialPort(void);

    // p_parity
    //   false: N81 (none, 8 data, 1 stop)
    //   true:  E71 (even, 7 data, 1 stop)
    bool init(const char* p_device, const uint16_t p_baud, const bool p_parity);
    void close(void);
    bool read(MsgBuf& p_msgBuf) const;
    bool write(MsgBuf& p_msgBuf) const;
};

#endif // __serial_port_h__
