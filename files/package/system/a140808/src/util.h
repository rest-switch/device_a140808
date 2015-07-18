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

#ifndef __util_h__
#define __util_h__

#include <stdint.h>    // uint64_t
#include <stdlib.h>    // strtoull
#include <unistd.h>    // usleep
#include <sys/time.h>

#include "global.h"
#include "log.h"

#define min(a,b) ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a < _b ? _a : _b; })
#define max(a,b) ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a > _b ? _a : _b; })

#define date_ms_now()  ({struct timeval tv; gettimeofday(&tv, NULL); ((uint64_t)(tv.tv_sec) * 1000 + (uint64_t)(tv.tv_usec) / 1000);})
#define date_sec_now() ({struct timeval tv; gettimeofday(&tv, NULL); tv.tv_sec;})
#define sleep_ms(ms)   usleep((ms)*1000);

#define str2num(str,base)   ({strtoull((str), NULL, (base));})
#define hex2num(str)        ({strtoull((str), NULL, 16);})

#endif // __util_h__
