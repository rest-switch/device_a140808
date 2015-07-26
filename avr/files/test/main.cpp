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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/select.h>

#include <string>
#include <vector>

#include "../msg_processor.h"

#include "kbhit.h"



////////////////////////////////////////
void on_poll(MsgProcessor& p_mp)
{
    // our timeslice
}

////////////////////////////////////////
void on_pong(MsgProcessor& p_mp, const uint8_t p_param1, const uint8_t p_param2, const uint8_t p_param3)
{
    ::printf("\nping response: p1:[%d] p2:[%d] p3:[%d]\n", p_param1, p_param2, p_param3);
    ::printf("\nA140808>");
}

////////////////////////////////////////
void on_read_register(MsgProcessor& p_mp, const uint8_t p_registerAddress)
{
    ::printf("\non_read_register - addr: [0x%x]\n", p_registerAddress);
    ::printf("\nA140808>");
}

////////////////////////////////////////
void on_write_register(MsgProcessor& p_mp, const uint8_t p_registerAddress, const uint8_t p_value, const uint8_t p_mask)
{
    ::printf("\non_write_register - addr: [0x%x]  val: [0x%x]  mask: [0x%x]\n", p_registerAddress, p_value, p_mask);
    ::printf("\nA140808>");
}

////////////////////////////////////////
void on_write_register_bit(MsgProcessor& p_mp, const uint8_t p_registerAddress, const uint8_t p_bit, const bool p_state)
{
    ::printf("\non_write_register_bit - addr: [0x%x]  bit: [0x%d]  state: [%d]\n", p_registerAddress, p_bit, p_state);
    ::printf("\nA140808>");
}

////////////////////////////////////////
void on_pulse_register_bit(MsgProcessor& p_mp, const uint8_t p_registerAddress, const uint8_t p_bit, const uint8_t p_durationMs)
{
    ::printf("\non_pulse_register_bit - addr: [0x%x]  bit: [0x%d]  duration: [%dms]\n", p_registerAddress, p_bit, p_durationMs);
    ::printf("\nA140808>");
}

////////////////////////////////////////
void on_subscribe_register(MsgProcessor& p_mp, const uint8_t p_registerAddress, const uint8_t p_value, const bool p_cancel)
{
    switch(p_registerAddress)
    {
        case REG_INPUT_1:
        {
            ::printf("\n%son_subscribe_register, input - val: [0x%x]\n", p_cancel ? "cancel " : "", p_value);
            break;
        }
        case REG_OUTPUT_1:
        {
            ::printf("\n%son_subscribe_register, output - val: [0x%x]\n", p_cancel ? "cancel " : "", p_value);
            break;
        }
        default:
        {
            ::printf("\non_subscribe_register - ERROR! unknown register: [0x%x]  val: [0x%x]  cancel: [%d]\n", p_registerAddress, p_value, p_cancel);
            break;
        }
    }
    ::printf("\nA140808>");
}



////////////////////////////////////////
void parse_cmd_parms(const std::string& p_command, uint8_t& p_param1, uint8_t& p_param2, uint8_t& p_param3)
{
    p_param1 = 0;
    p_param2 = 0;
    p_param3 = 0;

    const std::string::size_type pos1 = p_command.find(' ');
    if(std::string::npos == pos1)
    {
        return;
    }

    const std::string::size_type pos2 = p_command.find(' ', pos1+1);
    if(std::string::npos == pos2)
    {
        const std::string str = p_command.substr(pos1+1);
        p_param1 = (uint8_t)::strtol(str.c_str(), NULL, 0);
        return;
    }
    else
    {
        const std::string str = p_command.substr(pos1+1, pos2-pos1);
        p_param1 = (uint8_t)::strtol(str.c_str(), NULL, 0);
    }

    const std::string::size_type pos3 = p_command.find(' ', pos2+1);
    if(std::string::npos == pos3)
    {
        const std::string str = p_command.substr(pos2+1);
        p_param2 = (uint8_t)::strtol(str.c_str(), NULL, 0);
        return;
    }
    else
    {
        const std::string str = p_command.substr(pos2+1, pos3-pos2);
        p_param2 = (uint8_t)::strtol(str.c_str(), NULL, 0);
    }

    const std::string str = p_command.substr(pos3+1);
    p_param3 = (uint8_t)::strtol(str.c_str(), NULL, 0);
}


////////////////////////////////////////
//    ::srand(::GetTickCount());
//    const uint8_t val = (uint8_t)(::rand() % 255);
bool do_command(MsgProcessor& p_mp, const std::string& p_command)
{
    ::printf("do_command: %s\n", p_command.c_str());

    if(0 == ::strcmp("read in", p_command.c_str()))
    {
        p_mp.dispatch_read_register(REG_INPUT_1);
        return(true);
    }
    if(0 == ::strcmp("read out", p_command.c_str()))
    {
        p_mp.dispatch_read_register(REG_OUTPUT_1);
        return(true);
    }

    if(0 == ::strcmp("sub in", p_command.c_str()))
    {
        p_mp.dispatch_subscribe_register(REG_INPUT_1);
        return(true);
    }
    if(0 == ::strcmp("sub in cancel", p_command.c_str()))
    {
        p_mp.dispatch_subscribe_register(REG_INPUT_1, 0, true);
        return(true);
    }
    if(0 == ::strcmp("sub out", p_command.c_str()))
    {
        p_mp.dispatch_subscribe_register(REG_OUTPUT_1);
        return(true);
    }
    if(0 == ::strcmp("sub out cancel", p_command.c_str()))
    {
        p_mp.dispatch_subscribe_register(REG_OUTPUT_1, 0, true);
        return(true);
    }

    if((p_command.size() > 3) && ('p' == p_command[0]) && ('i' == p_command[1]) && ('n' == p_command[2]) && ('g' == p_command[3]))
    {
        uint8_t param1 = 0;
        uint8_t param2 = 0;
        uint8_t param3 = 0;
        parse_cmd_parms(p_command, param1, param2, param3);

        ::printf("sending ping - p1:[%d] p2:[%d] p3:[%d]\n\n", param1, param2, param3);
        if(!p_mp.dispatch_ping(param1, param2, param3))
        {
            ::printf("failed to send ping\n\n");
            return(false);  // error
        }
        return(true);
    }

    // from here on out, commands are two chars followed by a space
    if((p_command.size() < 6) || (' ' != p_command[2]))
    {
        return(false);  // invalid command
    }

    uint8_t param1 = 0;
    uint8_t param2 = 0;
    uint8_t param3 = 0;
    parse_cmd_parms(p_command, param1, param2, param3);

    // write register
    if(('w' == p_command[0]) && ('r' == p_command[1]))
    {
        ::printf("writing register - val: [%d] mask: [%d]\n\n", param1, param2);
        if(!p_mp.dispatch_write_register(REG_OUTPUT_1, param1, param2))
        {
            ::printf("failed to send write register\n\n");
            return(false);  // error
        }
        return(true);  // valid command
    }

    // write register bit
    else if(('w' == p_command[0]) && ('b' == p_command[1]))
    {
        if(param1 > 7)
        {
            ::printf("bit number must be 0-7 - invalid value: [%d]\n\n", param1);
            return(false);  // error
        }
        if((0 != param2) && (1 != param2))
        {
            ::printf("bit val must be 0 or 1 - invalid value: [%d]\n\n", param2);
            return(false);  // error
        }

        ::printf("writing register bit - bit: [%d] val: [%d]\n\n", param1, param2);
        if(!p_mp.dispatch_write_register_bit(REG_OUTPUT_1, param1, 0!=param2))
        {
            ::printf("failed to send write register bit\n\n");
            return(false);  // error
        }
        return(true);  // valid command
    }

    // pulse register bit
    else if(('p' == p_command[0]) && ('b' == p_command[1]))
    {
        if(param1 > 7)
        {
            ::printf("bit number must be 0-7 - invalid value: [%d]\n\n", param1);
            return(false);  // error
        }

        ::printf("pulsing register bit - bit: [%d] duration: [%d ms]\n\n", param1, param2);
        if(!p_mp.dispatch_pulse_register_bit(REG_OUTPUT_1, param1, param2))
        {
            ::printf("failed to send pulse register bit\n\n");
            return(false);  // error
        }
        return(true);  // valid command
    }

    return(false);  // unknown command
}


////////////////////////////////////////
    static std::string command;
    static std::vector<std::string> commandStack;
    static int8_t historyIndex = 0;
bool scan_keyboard(keyboard& p_kb, MsgProcessor& p_mp)
{
    if(p_kb.kbhit())
    {
        int ch = p_kb.getch();
        if(10 == ch)  // enter
        {
            // do command
            ::printf("\n");
            if((4 == command.size()))
            {
                if( (0 == ::strcmp("exit", command.c_str())) ||
                    (0 == ::strcmp("quit", command.c_str())) )
                {
                    ::printf("goodbye\n\n");
                    return(false);
                }
                if(0 == ::strcmp("help", command.c_str()))
                {
                    ::printf("\n");
                    ::printf("read in               - read inputs\n");
                    ::printf("read out              - read outputs\n");
                    ::printf("sub in                - subscribe inputs\n");
                    ::printf("sub in cancel         - cancel subscribe inputs\n");
                    ::printf("sub out               - subscribe outputs\n");
                    ::printf("sub out cancel        - cancel subscribe outputs\n");
                    ::printf("ping [p1] [p2] [p3]   - ping the avr [optional values]\n");
                    ::printf("wr <value> <mask>     - write register\n");
                    ::printf("wb <bit> <bool>       - write bit\n");
                    ::printf("pb <bit> <delay ms>   - pulse bit state for delay ms\n");
                    ::printf("exit                  - quit this application\n");
                    ::printf("\n");
                    command.clear();
                    ::printf("\r                                                            \rA140808>%s", command.c_str());
                    ::fflush(stdout);
                    return(true);
                }
            }
            if(!command.empty())
            {
                if(do_command(p_mp, command))
                {
                    bool found = false;
                    for(std::vector<std::string>::const_iterator it=commandStack.begin(); it!=commandStack.end(); ++it)
                    {
                        const std::string& item = *it;
                        if(item == command)
                        {
                            found = true;
                            break;
                        }
                    }
                    if(!found)
                    {
                        commandStack.push_back(command);
                    }
                    if(commandStack.size() > 5)
                    {
                        commandStack.erase(commandStack.begin());
                    }
                    historyIndex = commandStack.size();
                }
                else
                {
                    ::printf("unknown command: %s\n\n", command.c_str());
                }
                command.clear();
            }
        }
        else if(27 == ch) // esc
        {
            if(p_kb.kbhit() && (91 == p_kb.getch()) && p_kb.kbhit())
            {
                // multiple char command
                switch(p_kb.getch())
                {
                    case 65: // up arrow
                    {
                        if(!commandStack.empty())
                        {
                            if(--historyIndex < 0) historyIndex = (commandStack.size() - 1); // wrap to last element
                            command = commandStack[historyIndex];
                        }
                        break;
                    }
                    case 68: // left arrow
                    {
                        if(!command.empty())
                        {
                            command.erase(command.end()-1);
                        }
                        break;
                    }
                    case 66: // down arrow
                    {
                        if(!commandStack.empty())
                        {

                            if(++historyIndex >= commandStack.size()) historyIndex = 0; // wrap to beginning
                            command = commandStack[historyIndex];
                        }
                        break;
                    }
                }
            }
            else
            {
                // <esc> alone
                command.clear();
                historyIndex = commandStack.size();
            }
        }
        else if((ch > 31) && (ch < 127)) // printable chars
        {
            command += ::tolower(ch);
        }
        else if(127 == ch) // delete
        {
            if(!command.empty())
            {
                command.erase(command.end()-1);
            }
        }
        ::printf("\r                                                            \rA140808>%s", command.c_str());
        ::fflush(stdout);
    }

    return(true);
}


////////////////////////////////////////
int main(const int p_argc, const char** p_argv)
{
    if(p_argc < 2)
    {
        ::printf("specify serial device\n");
        return(1);
    }
    const char* serialDevice = p_argv[1];

    // create the message pump
    MsgProcessor mp;
    if(!mp.init(serialDevice, 57600, true/*E71*/))
    {
        ::printf("\nfailed to initialize message processor\n");
        return(1);
    }

    keyboard kb;
//    setvbuf(stdout, NULL, _IONBF, 0);
    ::printf("\nA140808>");
    for(;;)
    {
        // check for new messages
        mp.poll();

        if(!scan_keyboard(kb, mp))
        {
            break;
        }

        ::usleep(100000);  // loop 10 times a second
    }

    return(0);
}

