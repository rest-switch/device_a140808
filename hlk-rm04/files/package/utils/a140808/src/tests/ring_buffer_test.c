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
#include <string.h>

#include "../ring_buffer.h"


int main(const int p_argc, const char** p_argv)
{
	printf("\n--- begin test ---\n\n");

	struct ring_buf_data rbd = { 0 };
	rb_init(&rbd, 16);  // init to 16 bytes


	/////////////////////////////////
	// print the capacity & size
	printf("----------------\n");
	printf("initial rb_capacity: %d  rb_size: %d\n", rb_capacity(&rbd), rb_size(&rbd));

	// load the buffer to capacity
	const char* test_data = "0123456789abcdef";
	rb_set_data(&rbd, test_data, strlen(test_data));
	printf("rb_capacity: %d  rb_size: %d\n", rb_capacity(&rbd), rb_size(&rbd));

	// test the contents
	printf("reading buffer conents, expecting: [%s]\n", test_data);
	printf("                              got: [");
	for(uint8_t i=0, imax=rb_size(&rbd); i<imax; ++i)
	{
		printf("%c", rb_at(&rbd, i));
	}
	printf("]\n");
	printf("----------------\n\n");


	/////////////////////////////////
	// inject 3 more
	printf("----------------\n");
	printf("injecting 3 chars: #+@\n");
	rb_push_back(&rbd, '#');
	rb_push_back(&rbd, '+');
	rb_push_back(&rbd, '@');
	printf("rb_capacity: %d  rb_size: %d\n", rb_capacity(&rbd), rb_size(&rbd));
	printf("now: [");
	for(uint8_t i=0, imax=rb_size(&rbd); i<imax; ++i)
	{
		printf("%c", rb_at(&rbd, i));
	}
	printf("]\n");
	printf("----------------\n\n");


	/////////////////////////////////
	// remove the 3 chars from end
	printf("----------------\n");
	printf("remove three chars from end\n");
	{ const char c = rb_pop_back(&rbd); printf("removed: [%c]\n", c); }
	printf("rb_capacity: %d  rb_size: %d\n", rb_capacity(&rbd), rb_size(&rbd));
	printf("now: [");
	for(uint8_t i=0, imax=rb_size(&rbd); i<imax; ++i)
	{
		printf("%c", rb_at(&rbd, i));
	}
	printf("]\n");
	{ const char c = rb_pop_back(&rbd); printf("removed: [%c]\n", c); }
	printf("rb_capacity: %d  rb_size: %d\n", rb_capacity(&rbd), rb_size(&rbd));
	printf("now: [");
	for(uint8_t i=0, imax=rb_size(&rbd); i<imax; ++i)
	{
		printf("%c", rb_at(&rbd, i));
	}
	printf("]\n");
	{ const char c = rb_pop_back(&rbd); printf("removed: [%c]\n", c); }
	printf("rb_capacity: %d  rb_size: %d\n", rb_capacity(&rbd), rb_size(&rbd));
	printf("now: [");
	for(uint8_t i=0, imax=rb_size(&rbd); i<imax; ++i)
	{
		printf("%c", rb_at(&rbd, i));
	}
	printf("]\n");
	printf("----------------\n\n");


	/////////////////////////////////
	// inject WXYZ on the end
	printf("----------------\n");
	printf("inject WXYZ on the end\n");
	rb_push_back(&rbd, 'W');
	printf("rb_capacity: %d  rb_size: %d\n", rb_capacity(&rbd), rb_size(&rbd));
	printf("now: [");
	for(uint8_t i=0, imax=rb_size(&rbd); i<imax; ++i)
	{
		printf("%c", rb_at(&rbd, i));
	}
	printf("]\n");
	rb_push_back(&rbd, 'X');
	printf("rb_capacity: %d  rb_size: %d\n", rb_capacity(&rbd), rb_size(&rbd));
	printf("now: [");
	for(uint8_t i=0, imax=rb_size(&rbd); i<imax; ++i)
	{
		printf("%c", rb_at(&rbd, i));
	}
	printf("]\n");
	rb_push_back(&rbd, 'Y');
	printf("rb_capacity: %d  rb_size: %d\n", rb_capacity(&rbd), rb_size(&rbd));
	printf("now: [");
	for(uint8_t i=0, imax=rb_size(&rbd); i<imax; ++i)
	{
		printf("%c", rb_at(&rbd, i));
	}
	printf("]\n");
	rb_push_back(&rbd, 'Z');
	printf("rb_capacity: %d  rb_size: %d\n", rb_capacity(&rbd), rb_size(&rbd));
	printf("now: [");
	for(uint8_t i=0, imax=rb_size(&rbd); i<imax; ++i)
	{
		printf("%c", rb_at(&rbd, i));
	}
	printf("]\n");
	printf("----------------\n\n");


	/////////////////////////////////
	// trim two from the front
	printf("----------------\n");
	printf("trim two from the front\n");
	{ const char c = rb_pop_front(&rbd); printf("removed: [%c]\n", c); }
	printf("rb_capacity: %d  rb_size: %d\n", rb_capacity(&rbd), rb_size(&rbd));
	printf("now: [");
	for(uint8_t i=0, imax=rb_size(&rbd); i<imax; ++i)
	{
		printf("%c", rb_at(&rbd, i));
	}
	printf("]\n");
	{ const char c = rb_pop_front(&rbd); printf("removed: [%c]\n", c); }
	printf("rb_capacity: %d  rb_size: %d\n", rb_capacity(&rbd), rb_size(&rbd));
	printf("now: [");
	for(uint8_t i=0, imax=rb_size(&rbd); i<imax; ++i)
	{
		printf("%c", rb_at(&rbd, i));
	}
	printf("]\n");
	printf("----------------\n\n");


	/////////////////////////////////
	// grab and empty the whole internal buffer
	printf("----------------\n");
	char buf[16];
	printf("before rb_capacity: %d  rb_size: %d\n", rb_capacity(&rbd), rb_size(&rbd));
	rb_get_data(&rbd, buf, sizeof(buf));
	printf("after rb_capacity: %d  rb_size: %d\n", rb_capacity(&rbd), rb_size(&rbd));
	printf("extracted buffer: [%s]\n", buf);
	printf("----------------\n\n");


	/////////////////////////////////
	// cleanup
	printf("----------------\n");
	printf("cleaning up...\n");
	rb_free(&rbd);
	printf("after cleanup rb_capacity: %d  rb_size: %d\n", rb_capacity(&rbd), rb_size(&rbd));
	printf("----------------\n\n");

	printf("---  end test  ---\n\n");
	return(0);
}
