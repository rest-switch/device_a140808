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

#ifndef __global_h__
#define __global_h__

#define WEB_HOST        "pubsub.rest-switch.com"
#define PROGNAME        "a140808"
#define DEFAULT_SERNO   "000000000"
#define PIDFILE         "/var/run/" PROGNAME ".pid"
#define SERIAL_PORT     "/dev/ttyS1"
#define SERIAL_BAUD     57600
#define SERIAL_USE_E71  true

#define DEBUG
//#define DEBUG_TRACE
#define RUN_AS_DAEMON


#include "log.h"
#include "util.h"


#endif // __global_h__
