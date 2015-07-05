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

#include <string.h>   // strcmp
#include <stdio.h>    // printf
#include <stdlib.h>   // strtol
#include <time.h>     // time

typedef _Bool bool;
#define true  1
#define false 0

#include "b32coder.h"



////////////////////////////////////////////////////////////
//
//  print_timestamp [now | ajustv1nr | 0x14ca119e18f] [format]
//
int main(int argc, char** argv)
{
    // if no params, default to format gen b32 now
    bool gen_now = true;
    bool format = false;
    bool encode = true;
    char buf[64];

    // param 1
    //   raw (default)
    //   format
    //   ajustv1nr
    //   0x14ca119e18f
    //   help (default)
    if(argc > 1)
    {
        if(0 == strcmp("now", argv[1]))
        {
            gen_now = true;
            encode = true;
        }
        else if(9 == strlen(argv[1]))
        {
            // assume we are decoding a 9 byte b32 value
            gen_now = false;
            encode = false;  // (decode)
            strcpy(buf, argv[1]);
        }
        else if((strlen(argv[1]) > 12) && ('0' == argv[1][0]) && ('x' == argv[1][1]))
        {
            // assume we are encoding a 5+ byte hex javascript time
            gen_now = false;
            encode = true;
            long datetime = strtol(argv[1], NULL, 16);
            int len = encode_datetime(datetime, buf, sizeof(buf));
            if(9 != len)
            {
                printf("failed to generate time stamp (rc=%d)\n", len);
                return(1);
            }
        }
        else if(0 == strcmp("format", argv[1]))
        {
            format = true;
        }
        else //if(0 == strcmp("help", argv[1]))
        {
            printf("print_timestamp [now | ajustv1nr | 0x14ca119e18f] [raw]\n");
            return(0);
        }
    }

    if((argc > 2) && (0 == strcmp("format", argv[2])))
    {
        format = true;
    }

    if(gen_now)
    {
        int len = encode_datetime_now(buf, sizeof(buf));
        if(9 != len)
        {
            printf("failed to generate time stamp (rc=%d)\n", len);
            return(2);
        }
    }

    if(format)
    {
        // b32 enc  -->  ajustv1nr
        // js time  -->  0x14ca119e18f  (Thu 04/09/2015 10:12:50 PM EDT)
        long res = decode_datetime(buf, 0);
        if(0 == res)
        {
            printf("failed to decode time stamp (rc=%ld)\n", res);
            return(3);
        }

        printf("b32 enc  -->  %s\n", buf);

        // ms to sec
        long ressec = (res / 1000);

        char buf2[32];
        struct tm ts = *localtime(&ressec);
        strftime(buf2, sizeof(buf2), "%a %m/%d/%Y %I:%M:%S %p %Z", &ts);
        printf("js time  -->  0x%lx  (%s)\n", res, buf2);
    }
    else if(encode)
    {
        // encode raw
        printf("%s\n", buf);
    }
    else
    {
        // decode raw
        long res = decode_datetime(buf, 0);
        if(0 == res)
        {
            printf("failed to decode time stamp (rc=%ld)\n", res);
            return(3);
        }

        printf("decoded(raw): 0x%lx\n", res);
    }

    return(0);
}
