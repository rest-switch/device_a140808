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

int encode(const void* p_inbuf, const int p_inbuflen, void* p_outbuf, const int p_outbuflen);
int decode(const void* p_inbuf, const int p_inbuflen, void* p_outbuf, const int p_outbuflen);
int encode_datetime(const time_t p_datetime, void* p_outbuf, const int p_outbuflen);
int encode_datetime_now(void* p_outbuf, const int p_outbuflen);
long decode_datetime(const char* p_encoded, const int p_encodedlen);
