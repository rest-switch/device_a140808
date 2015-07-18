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

#ifndef __websock_h__
#define __websock_h__

#include <stdbool.h>


//
// test implementation:
//
// int main(int argc, const char** argv)
// {
//     // this call will only fail in pretty extraordinary conditions like an invalid host name or bad port number
//     int rc = ws_connect((argc < 2) ? "172.18.90.116" : argv[1], "/JOHN8TEST/172.18.90.116", 8080, 0);
//     if(0 != rc)
//     {
//         return(-1);
//     }
//
//     for(;;)
//     {
//         ws_poll();
//     }
//
//     // stay closed & cleanup
//     ws_close();
//
//     return(0);
// }


// must be implemented
void ws_onopen(void);
void ws_onmessage(const char* p_msg);
void ws_onerror(void);
void ws_onclose(const bool p_force);
void ws_onpong(const char* p_msg);

int ws_connect(const char* p_host, const char* p_url_path, const int p_port, const int p_ssl_flags);
void ws_close(void);
void ws_send_ping(const char* p_msg);
void ws_send_text(const char* p_msg);
void ws_poll(void);

#endif // __websock_h__
