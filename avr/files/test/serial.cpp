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

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

#include "../serial.h"
#include "../ring_buffer.h"
#include "../msg_buf.h"

static int s_fd = -1;

////////////////////////////////////////
speed_t parse_baudrate(uint32_t p_requested)
{
    speed_t baudrate = 0;
    switch(p_requested)
    {
#ifdef B50
    case 50: baudrate = B50; break;
#endif
#ifdef B75
    case 75: baudrate = B75; break;
#endif
#ifdef B110
    case 110: baudrate = B110; break;
#endif
#ifdef B134
    case 134: baudrate = B134; break;
#endif
#ifdef B150
    case 150: baudrate = B150; break;
#endif
#ifdef B200
    case 200: baudrate = B200; break;
#endif
#ifdef B300
    case 300: baudrate = B300; break;
#endif
#ifdef B600
    case 600: baudrate = B600; break;
#endif
#ifdef B1200
    case 1200: baudrate = B1200; break;
#endif
#ifdef B1800
    case 1800: baudrate = B1800; break;
#endif
#ifdef B2400
    case 2400: baudrate = B2400; break;
#endif
#ifdef B4800
    case 4800: baudrate = B4800; break;
#endif
#ifdef B9600
    case 9600: baudrate = B9600; break;
#endif
#ifdef B19200
    case 19200: baudrate = B19200; break;
#endif
#ifdef B38400
    case 38400: baudrate = B38400; break;
#endif
#ifdef B57600
    case 57600: baudrate = B57600; break;
#endif
#ifdef B115200
    case 115200: baudrate = B115200; break;
#endif
#ifdef B230400
    case 230400: baudrate = B230400; break;
#endif
#ifdef B460800
    case 460800: baudrate = B460800; break;
#endif
#ifdef B500000
    case 500000: baudrate = B500000; break;
#endif
#ifdef B576000
    case 576000: baudrate = B576000; break;
#endif
#ifdef B921600
    case 921600: baudrate = B921600; break;
#endif
#ifdef B1000000
    case 1000000: baudrate = B1000000; break;
#endif
#ifdef B1152000
    case 1152000: baudrate = B1152000; break;
#endif
#ifdef B1500000
    case 1500000: baudrate = B1500000; break;
#endif
#ifdef B2000000
    case 2000000: baudrate = B2000000; break;
#endif
#ifdef B2500000
    case 2500000: baudrate = B2500000; break;
#endif
#ifdef B3000000
    case 3000000: baudrate = B3000000; break;
#endif
#ifdef B3500000
    case 3500000: baudrate = B3500000; break;
#endif
#ifdef B4000000
    case 4000000: baudrate = B4000000; break;
#endif
    default: baudrate = 0; break;
    }

    return(baudrate);
}


////////////////////////////////////////
SerialPort::SerialPort(void)
{
}

////////////////////////////////////////
SerialPort::~SerialPort(void)
{
    this->close();
}

////////////////////////////////////////
// p_device: ignored
// p_parity
//   false: N81 (none, 8 data, 1 stop)
//   true:  E71 (even, 7 data, 1 stop)
bool SerialPort::init(const char* p_device, const uint16_t p_baud, const bool p_parity)
{
    this->close();
    ::printf("opening %s at %d baud\n", p_device, p_baud);

    const speed_t baudrate = ::parse_baudrate(p_baud);
    if(0 == baudrate)
    {
        ::perror("baudrate not supported");
        return(false);
    }

    // open serial device for reading and writing and not as controlling tty so we don't get killed by CTRL-C
    s_fd = open(p_device, O_RDWR | O_NOCTTY | O_NDELAY);
    if(s_fd < 0)
    {
        ::perror(p_device);
        return(false);
    }

    struct termios tio = { 0 };
//    ::cfsetispeed(&tio, baudrate);
//    ::cfsetospeed(&tio, baudrate);
    ::cfsetspeed(&tio, baudrate);
    if(p_parity)
    {
        tio.c_cflag |= (CS7 | PARENB | CLOCAL | CREAD);
    }
    else
    {
        tio.c_cflag |= (CS8 | CLOCAL | CREAD);
    }

    // ignore bytes with parity errors
    tio.c_iflag = IGNPAR;

    // raw output
    tio.c_oflag = 0;

    // local modes
    tio.c_lflag = 0;

//    tio.c_cc[VMIN] = 1;

    // clean the modem line and activate the settings for the port
    ::tcflush(s_fd, TCIOFLUSH);
    ::tcsetattr(s_fd, TCSANOW, &tio);

    return(true);
}

////////////////////////////////////////
void SerialPort::close(void)
{
    if(s_fd > -1)
    {
        ::close(s_fd);
        s_fd = -1;
    }
}

////////////////////////////////////////
bool SerialPort::read(MsgBuf& p_msgBuf) const
{
    for(;;)
    {
        uint8_t val = 0;
        const ssize_t bytesRead = ::read(s_fd, &val, 1);
        if(bytesRead < 0)
        {
            // error
            perror("serial read error");
            return(false);
        }

        if(0 == bytesRead)
        {
            // no data available
            break;
        }

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

    for(uint8_t i=0, imax=p_msgBuf.size(); i<imax; ++i)
    {
        const uint8_t val = p_msgBuf[i];
        const ssize_t bytesWritten = ::write(s_fd, &val, 1);
        if(1 != bytesWritten)
        {
            // error
            ::perror("serial write error");
            return(false);
        }
    }

    return(true);
}

