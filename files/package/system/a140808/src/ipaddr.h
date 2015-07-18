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

#ifndef __ipaddr_h__
#define __ipaddr_h__

#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if.h>

#include "global.h"


int get_ipv4_addresses(const char* p_delim, char* p_buf, size_t p_buflen)
{
    memset(p_buf, 0, p_buflen);

    struct ifaddrs* paddrs;
    if(getifaddrs(&paddrs) < 0)
    {
        log_err("get_ipv4_addresses:getifaddrs, err: [%s]", strerror(errno));
        return(-1);
    }

    size_t out_len = 0;
    const size_t delim_len = strlen(p_delim);
    for(struct ifaddrs* pifa = paddrs; pifa != NULL; pifa = pifa->ifa_next)
    {
        if((NULL == pifa->ifa_addr) || (AF_INET != pifa->ifa_addr->sa_family))
        {
            continue; // skip non ipv4 addresses
        }

        if(IFF_LOOPBACK == (pifa->ifa_flags & IFF_LOOPBACK))
        {
            continue; // skip loopback interfaces
        }

        if((IFF_UP != (pifa->ifa_flags & IFF_UP)) || (IFF_RUNNING != (pifa->ifa_flags & IFF_RUNNING)))
        {
            continue; // skip interfaces that are not up and running
        }

        struct sockaddr_in* paddr = (struct sockaddr_in*)pifa->ifa_addr;
        // struct sockaddr_in* pmask = (struct sockaddr_in*)pifa->ifa_netmask;

        const char* ip = inet_ntoa(paddr->sin_addr);
        const size_t ip_len = strlen(ip);
        if((out_len + delim_len + ip_len + 1) > p_buflen)
        {
            break; // cant fit any more addresses on
        }

	if(out_len > 0)
        {
            // add in a delim
            strcat(p_buf, p_delim);
            out_len += delim_len;
        }

        strcat(p_buf, ip);
        out_len += ip_len;
    }

    freeifaddrs(paddrs);

    // 1.1.1.1
    return((out_len < 7) ? -1 : 0);
}

#endif // __ipaddr_h__
