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

#include <stdarg.h>     // va_list, va_start
#include <stdio.h>
#include <sys/types.h>  // getpid
#include <unistd.h>

#include "global.h"
#include "log.h"
#include "util.h"


////////////////////////////////////////
void startsyslog(void)
{
    openlog(PROGNAME, LOG_PID, LOG_DAEMON);  // LOG_USER, LOG_LOCAL0
}

////////////////////////////////////////
void closesyslog(void)
{
    closelog();
}


#ifdef DEBUG_TRACE
////////////////////////////////////////
void debug_trace(const char* format, ...)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    va_list param;
    va_start(param, format);
    fprintf(stderr, "TRACE  (%d) %d.%d: ", getpid(), tv.tv_sec, tv.tv_usec);
    vfprintf(stderr, format, param);
    fprintf(stderr, "\n");
    va_end(param);
}

////////////////////////////////////////
void debug_trace2(const char* format, ...)
{
    static int trace_env = -1;
    if(trace_env == -1)
    {
        trace_env = (getenv("DEBUG_TRACE2") ? 1 : 0);
    }
    if(trace_env < 1)
    {
        return;
    }

    struct timeval tv;
    gettimeofday(&tv, NULL);

    va_list param;
    va_start(param, format);
    fprintf(stderr, "TRACE2  (%d) %d.%d: ", getpid(), tv.tv_sec, tv.tv_usec);
    vfprintf(stderr, format, param);
    fprintf(stderr, "\n");
    va_end(param);
}
#endif // DEBUG_TRACE
