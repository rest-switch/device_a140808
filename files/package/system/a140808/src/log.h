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

#ifndef __log_h__
#define __log_h__

#include <errno.h>
#include <syslog.h>

#ifdef DEBUG_TRACE
#define DEBUG
#endif // DEBUG_TRACE

// log_emerg   - system is unusable
// log_alert   - action must be taken immediately
// log_crit    - critical condition
// log_err     - error condition
// log_warn    - warning condition
// log_notice  - normal but significant condition
// log_info    - informational message
// log_debug   - debug message
// log_trace   - tracing

void startsyslog(void);
void closesyslog(void);


// always log these
#define log_emerg(...)  syslog(LOG_EMERG, __VA_ARGS__);
#define log_alert(...)  syslog(LOG_ALERT, __VA_ARGS__);
#define log_crit(...)   syslog(LOG_CRIT, __VA_ARGS__);
#define log_err(...)    syslog(LOG_ERR, __VA_ARGS__);
#define log_warn(...)   syslog(LOG_WARNING, __VA_ARGS__);
#define log_notice(...) syslog(LOG_NOTICE, __VA_ARGS__);

#ifdef DEBUG
#define log_info(...)   syslog(LOG_INFO, __VA_ARGS__);
#define log_debug(...)  syslog(LOG_DEBUG, __VA_ARGS__);
#else
#define log_info(...)
#define log_debug(...)
#endif

#ifdef DEBUG_TRACE
void debug_trace(const char* format, ...);
void debug_trace2(const char* format, ...);
#define log_trace(...) debug_trace(__VA_ARGS__);
#define log_trace2(...) debug_trace2(__VA_ARGS__);
#else
#define log_trace(...)
#define log_trace2(...)
#endif // DEBUG_TRACE

#endif // __log_h__
