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

#include <string.h> // strcmp
#include <json/json.h>

// ip address begin
#include <sys/types.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if.h>
// ip address end

#include "global.h"
#include "websock.h"
#include "msg_proc.h"



////////////////////////////////////////
void pulseRelay(const uint8_t p_num, const uint8_t p_ms)
{
    log_notice("pulseRelay - num: [%d] cyc: [%dms]", p_num, p_ms);
    if(!mp_dispatch_pulse_register_bit(REG_OUTPUT_1, p_num, p_ms))
    {
        log_err("mp_dispatch_pulse_register_bit -- val: [%d]  cyc: [%d]\n", p_num, p_ms);
    }
}

////////////////////////////////////////
void writeOutputRegister(const uint8_t p_value, const uint8_t p_mask)
{
    log_notice("writeOutputRegister - val: [%d] mask: [%d]", p_value, p_mask);
    if(!mp_dispatch_write_register(REG_OUTPUT_1, p_value, p_mask))
    {
        log_err("mp_dispatch_write_register -- val: [%d]  mask: [%d]\n", p_value, p_mask);
    }
}

////////////////////////////////////////
void dialModem(const char* p_str)
{
    log_notice("dialModem - str: [%s]  (ignoring)", p_str);
}

////////////////////////////////////////
void processHello(const char* p_str)
{
    log_notice("server says 'hello' to device: [%s]", p_str);
}


////////////////////////////////////////
// { "ipv4Addresses": [ "172.17.133.3", "172.17.133.4", "172.17.133.5" ] }
void requestIpv4Addresses(void)
{
    log_notice("requestIpv4Addresses");

    struct ifaddrs* paddrs;
    if(getifaddrs(&paddrs) < 0)
    {
        log_err("get_ipv4_addresses:getifaddrs, err: [%s]", strerror(errno));
        return;
    }

    json_object* jarray = json_object_new_array();
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

        // an ipv4 address we care about
        const char* ip = inet_ntoa(paddr->sin_addr);
        log_debug("adding ipv4 address: %s", ip);
        json_object_array_add(jarray, json_object_new_string(ip));
    }
    freeifaddrs(paddrs);

    // serialize & send the ipv4 addresses back
    json_object* jobject = json_object_new_object();
    json_object_object_add(jobject, "ipv4Addresses", jarray);
    const char* str = json_object_to_json_string(jobject);
    ws_send_text(str);
    json_object_put(jarray);
    json_object_put(jobject);
}



////////////////////////////////////////
int get_string_from_array(struct json_object* p_pobj, const int p_index, const char** p_result)
{
    // must be an array
    const enum json_type obj_type = json_object_get_type(p_pobj);
    if(json_type_array != obj_type)
    {
        log_err("error get_string_from_array - message is not an array, type is: [%d]", obj_type);
        return(-1); // error, not an array
    }

    // check out of range
    const int arr_len = json_object_array_length(p_pobj);
    if(p_index >= arr_len)
    {
        log_err("error get_string_from_array - index out of range, capacity: [%d] index: [%d]", arr_len, p_index);
        return(-1); // error, out of range
    }

    // get the element
    struct json_object* pelem = json_object_array_get_idx(p_pobj, p_index);

    // make sure it is a string
    const enum json_type elem_type = json_object_get_type(pelem);
    if(json_type_string != elem_type)
    {
        log_err("error get_string_from_array - array element at index: [%d] is not a string, type is: [%d]", p_index, elem_type);
        return(-1); // error, not a string
    }

    // fetch as string
    *p_result = json_object_get_string(pelem);
    return(0);
}

////////////////////////////////////////
int get_int_from_array(struct json_object* p_pobj, const int p_index, int* p_result)
{
    // must be an array
    const enum json_type obj_type = json_object_get_type(p_pobj);
    if(json_type_array != obj_type)
    {
        log_err("error get_int_from_array - message is not an array, type is: [%d]", obj_type);
        return(-1); // error, not an array
    }

    // check out of range
    const int arr_len = json_object_array_length(p_pobj);
    if(p_index >= arr_len)
    {
        log_err("error get_int_from_array - index out of range, capacity: [%d] index: [%d]", arr_len, p_index);
        return(-1); // error, out of range
    }

    // get the element
    struct json_object* pelem = json_object_array_get_idx(p_pobj, p_index);

    // make sure it is a string (coerce) or an int
    const enum json_type elem_type = json_object_get_type(pelem);
    if((json_type_string != elem_type) && (json_type_int != elem_type))
    {
        log_err("error get_int_from_array - array element at index: [%d] is not a string nor an int, type is: [%d]", p_index, elem_type);
        return(-1); // error, not a string nor an int
    }

    // fetch as int
    *p_result = json_object_get_int(pelem);
    return(0);
}


////////////////////////////////////////
int dispatch_msg(const char* p_msg)
{
    log_debug("parsing message: %s", p_msg);
    struct json_object* pobj = json_tokener_parse(p_msg);
    if((NULL == pobj) || (is_error(pobj)))
    {
        log_err("error: message does not appear to be a valid json message: %s", p_msg);
        return(-1);
    }

    // param 0 is the function name
    const char* fcn;
    int rc = get_string_from_array(pobj, 0, &fcn);
    if(0 != rc)
    {
        log_err("error: failed to get function name from message");
        return(-1);
    }

    if(0 == strcmp("pulseRelay", fcn))
    {
        // ["pulseRelay",1,250]
        int num;
        rc = get_int_from_array(pobj, 1, &num);
        if(0 != rc)
        {
            log_err("error: pulseRelay: failed to extract param0 relay num");
            return(-1);
        }
        if((num < 0) || (num > 255))
        {
            log_err("error: pulseRelay: relay num out of range (0-255): [%d]", num);
            return(-1);
        }

        int ms;
        rc = get_int_from_array(pobj, 2, &ms);
        if(0 != rc)
        {
            log_debug("pulseRelay: no cycle ms specified, defaulting to 250ms");
            ms = 250;
        }
        if((ms < 0) || (ms > 255))
        {
            log_err("error: pulseRelay: pulse duration ms out of range (0-255): [%dms]", ms);
            return(-1);
        }

        pulseRelay((uint8_t)num, (uint8_t)ms);
    }

    else if(0 == strcmp("writeOutputRegister", fcn))
    {
        // ["writeOutputRegister",1,255]
        int val;
        rc = get_int_from_array(pobj, 1, &val);
        if(0 != rc)
        {
            log_err("error: writeOutputRegister: failed to extract param0 'value'");
            return(-1);
        }
        if((val < 0) || (val > 255))
        {
            log_err("error: writeOutputRegister: register value out of range (0-255): [%d]", val);
            return(-1);
        }

        int mask;
        rc = get_int_from_array(pobj, 2, &mask);
        if(0 != rc)
        {
            log_err("error: writeOutputRegister: failed to extract param1 'mask'");
            return(-1);
        }
        if((mask < 0) || (mask > 255))
        {
            log_err("error: writeOutputRegister: register mask out of range (0-255): [%d]", mask);
            return(-1);
        }

        writeOutputRegister((uint8_t)val, (uint8_t)mask);
    }

    else if(0 == strcmp("dialModem", fcn))
    {
        // ["dialModem","ATD3,4,4;"]
        const char* val;
        rc = get_string_from_array(pobj, 1, &val);
        if(0 != rc)
        {
            log_err("error: dialModem: failed to extract param0 dial string");
            return(-1);
        }

        dialModem(val);
    }

    else if(0 == strcmp("hello", fcn))
    {
        // ["hello","<device id>"]
        const char* str;
        rc = get_string_from_array(pobj, 1, &str);
        if(0 != rc)
        {
            log_err("error: hello: failed to extract param0 device id");
            return(-1);
        }

        processHello(str);
    }

    else if(0 == strcmp("requestIpv4Addresses", fcn))
    {
        requestIpv4Addresses();
    }

    else
    {
        log_err("error: unknown function: [%s]", fcn);
    }

    return(0);
}



//
// websock.h callback impl
//

////////////////////////////////////////
void ws_onopen(void)
{
    log_info("ws_onopen -- connection established --");
}

////////////////////////////////////////
void ws_onmessage(const char* p_msg)
{
    log_debug("ws_onmessage");
    log_trace("message received: %s", p_msg);
    dispatch_msg(p_msg);
}

////////////////////////////////////////
void ws_onerror(void)
{
    log_debug("ws_onerror");
}

////////////////////////////////////////
void ws_onclose(const bool p_force)
{
    log_info("ws_onclose");
}

////////////////////////////////////////
void ws_onpong(const char* p_msg)
{
    log_debug("ws_onpong");
    log_trace("message received: %s", p_msg);

    const uint64_t now = date_ms_now();
    const uint64_t then = hex2num(p_msg);
    const int eplapsed = (now - then);
    log_debug("ping flight time: [%dms]", eplapsed);
}



//
// msg_proc.h callback impl
//

////////////////////////////////////////
void mp_on_pong(const uint8_t p_param1, const uint8_t p_param2, const uint8_t p_param3)
{
    log_debug("mp_on_pong");
}

////////////////////////////////////////
void mp_on_read_register(const uint8_t p_registerAddress)
{
    log_debug("mp_on_read_register");
}

////////////////////////////////////////
void mp_on_write_register(const uint8_t p_registerAddress, const uint8_t p_value, const uint8_t p_mask)
{
    log_debug("mp_on_write_register");
}

////////////////////////////////////////
void mp_on_write_register_bit(const uint8_t p_registerAddress, const uint8_t p_bit, const bool p_state)
{
    log_debug("mp_on_write_register_bit");
}

////////////////////////////////////////
void mp_on_pulse_register_bit(const uint8_t p_registerAddress, const uint8_t p_bit, const uint8_t p_durationMs)
{
    log_debug("mp_on_pulse_register_bit");
}

////////////////////////////////////////
void mp_on_subscribe_register(const uint8_t p_registerAddress, const uint8_t p_value, const bool p_cancel)
{
    log_debug("mp_on_subscribe_register");
}
