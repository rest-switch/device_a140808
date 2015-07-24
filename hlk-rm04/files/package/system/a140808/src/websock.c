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

#include <inttypes.h>  // PRIu64
#include <stdio.h>
#include <stdarg.h>    // va_list, va_start
#include <string.h>
#include <syslog.h>

#include <cyassl/openssl/ssl.h>
#include <libwebsockets.h>

#include "global.h"
#include "websock.h"
#include "ring_buf.h"

#define RECONNECT_INTERVAL_SEC  5
static uint32_t s_reconnect_timer = 0;

#define HEARTBEAT_INTERVAL_SEC  300
static uint32_t s_heartbeat_timer = 0;

#define WRITE_BUF_MAX  64
static struct ring_buf_data s_ping_buf = { 0 };
static struct ring_buf_data s_text_buf = { 0 };

enum ws_ready_state
{
    WS_CONNECTING,  // the connection is not yet open
    WS_OPEN,        // the connection is open and ready to communicate
    WS_CLOSING,     // the connection is in the process of closing
    WS_CLOSED       // the connection is closed or couldn't be opened
};
static enum ws_ready_state s_ready_state = WS_CLOSED;

static struct libwebsocket* s_pWs = NULL;
static struct libwebsocket_context* s_pWsContext = NULL;

// forward declare for callback
int ws_onevent(struct libwebsocket_context*, struct libwebsocket*, enum libwebsocket_callback_reasons, void*, void*, size_t);


////////////////////////////////////////
int ws_connect(const char* p_host, const char* p_url_path, const int p_port, const int p_ssl_flags)
{
    // TODO: trace should include all debug?
    log_debug("ws_connect");
    log_trace("ws_connect");

    // param cache for ws_reconnect method
    static const char* s_host = NULL;
    static const char* s_url_path = NULL;
    static int s_port = 0;
    static int s_ssl_flags = 0;

    if(NULL != p_host)
    {
        s_host = p_host;
        s_url_path = p_url_path;
        s_port = p_port;
        s_ssl_flags = p_ssl_flags;
    }

    if(NULL == s_pWsContext)
    {
        log_debug("ws_connect: creating context");

        // this doesn't really belong here, but it is a good init hook location
        // because the write buffer gets freed at the same location as s_pWsContext
        if(rb_capacity(&s_ping_buf) < WRITE_BUF_MAX)
        {
            if(!rb_init(&s_ping_buf, WRITE_BUF_MAX))
            {
                log_crit("ws_connect: failed to allocate buffer - size: [%d]", WRITE_BUF_MAX);
                return(-1);
            }
        }
        if(rb_capacity(&s_text_buf) < WRITE_BUF_MAX)
        {
            if(!rb_init(&s_text_buf, WRITE_BUF_MAX))
            {
                log_crit("ws_connect: failed to allocate buffer - size: [%d]", WRITE_BUF_MAX);
                return(-1);
            }
        }

        static struct libwebsocket_protocols s_protocols[] =
        {
            {"ws_onevent",  ws_onevent,  0, 64, 0, NULL, 0},
            { 0 } // end
        };

        struct lws_context_creation_info info = { 0 };
        info.port = CONTEXT_PORT_NO_LISTEN;
        info.protocols = s_protocols;
        info.gid = -1;
        info.uid = -1;
//        info.ka_time = 300;
//        info.ka_probes = 3;
//        info.ka_interval = 5;

        s_pWsContext = libwebsocket_create_context(&info);
        if(NULL == s_pWsContext)
        {
            log_err("ws_connect: failed to create libwebsocket context");
            return(-1);
        }
    }

    s_ready_state = WS_CONNECTING;
//    s_pWs = libwebsocket_client_connect(s_pWsContext, s_host, s_port, s_ssl_flags, s_url_path, s_host, s_host, 0, 13 /* -1==latest */);
    s_pWs = libwebsocket_client_connect(s_pWsContext, s_host, s_port, s_ssl_flags, s_url_path, s_host, 0, 0, 13 /* -1==latest */);
    if(NULL == s_pWs)
    {
        log_err("ws_connect: libwebsocket connect failed");
        s_ready_state = WS_CLOSED;
        s_reconnect_timer = date_sec_now();  // try to reconnect...
        return(-1);
    }

    log_debug("ws_connect: connect request suceeded");

    return(0);
}


////////////////////////////////////////
// p_msg must be < WRITE_BUF_MAX
void ws_send_ping(const char* p_msg)
{
    if(NULL == p_msg) return;  // nothing to do

    const size_t msg_len = strlen(p_msg);
    if(msg_len < 1) return;  // nothing to do

    // truncate if too long
    uint8_t write_len = ((msg_len > 0xff) ? 0xff : msg_len);
    write_len = ((write_len > WRITE_BUF_MAX) ? WRITE_BUF_MAX : write_len);
    rb_set_data(&s_ping_buf, p_msg, write_len);

    // signal that we want an LWS_CALLBACK_CLIENT_WRITEABLE next service
    libwebsocket_callback_on_writable(s_pWsContext, s_pWs);
}

////////////////////////////////////////
// p_msg must be < WRITE_BUF_MAX
void ws_send_text(const char* p_msg)
{
    if(NULL == p_msg) return;  // nothing to do

    const size_t msg_len = strlen(p_msg);
    if(msg_len < 1) return;  // nothing to do

    // truncate if too long
    uint8_t write_len = ((msg_len > 0xff) ? 0xff : msg_len);
    write_len = ((write_len > WRITE_BUF_MAX) ? WRITE_BUF_MAX : write_len);
    rb_set_data(&s_text_buf, p_msg, write_len);

    // signal that we want an LWS_CALLBACK_CLIENT_WRITEABLE next service
    libwebsocket_callback_on_writable(s_pWsContext, s_pWs);
}


////////////////////////////////////////
void ws_close(void)
{
    log_debug("ws_close");

    if(NULL != s_pWsContext)
    {
        s_ready_state = WS_CLOSING;
        libwebsocket_context_destroy(s_pWsContext);
    }
}


////////////////////////////////////////
void ws_poll(void)
{
    // try reconnect?
    if((WS_CLOSED == s_ready_state) && (0 != s_reconnect_timer))
    {
        const uint32_t now = date_sec_now();
        if((now - s_reconnect_timer) >= RECONNECT_INTERVAL_SEC)
        {
            // flush timer (this is a timeout not an interval)
            s_reconnect_timer = 0;
            log_debug("attempting to reconnect...", 0);
            ws_connect(NULL, NULL, 0, 0);  // use cached credentials
        }
    }

    // pulse heartbeat?
    if((WS_OPEN == s_ready_state) && (0 != s_heartbeat_timer))
    {
        // we are connected, pulse the heartbeat
        const uint32_t now = date_sec_now();
        if((now - s_heartbeat_timer) >= HEARTBEAT_INTERVAL_SEC)
        {
            // reset the interval for the next heartbeat
            s_heartbeat_timer = now;
            log_debug("...heartbeat...", 0);
            char buf[32];
            sprintf(buf, "%" PRIx64, date_ms_now());
            ws_send_ping(buf);
        }
    }

    if(WS_CONNECTING == s_ready_state)
    {
        log_trace2(".");
    }

    // poll
    if(NULL == s_pWsContext)
    {
        // we are not connected, only loop once every 8 seconds
        sleep_ms(8000);
    }
    else
    {
        libwebsocket_service(s_pWsContext, 100); // 100ms timeout
        sleep_ms(125); // loop 8 times per sec
    }
}


////////////////////////////////////////
// LWS_WRITE_TEXT
// LWS_WRITE_BINARY
// LWS_WRITE_PING
// LWS_WRITE_PONG
void ws_write_data(struct ring_buf_data* p_pd, enum libwebsocket_write_protocol p_proto, struct libwebsocket_context* p_pWsContext, struct libwebsocket* p_pWs)
{
    static unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + WRITE_BUF_MAX + LWS_SEND_BUFFER_POST_PADDING];

    if(rb_empty(p_pd))
    {
        return;  // nothing to send
    }

    const uint8_t len = rb_get_data(p_pd, &buf[LWS_SEND_BUFFER_PRE_PADDING], WRITE_BUF_MAX);
#ifdef DEBUG
    buf[LWS_SEND_BUFFER_PRE_PADDING + len] = '\0'; // add term null, assume we have space for it due to LWS_SEND_BUFFER_POST_PADDING
    log_debug(">>>>>>>>>sending %d bytes: %s", len, &buf[LWS_SEND_BUFFER_PRE_PADDING]);
#endif // DEBUG
    const int sent_bytes = libwebsocket_write(p_pWs, &buf[LWS_SEND_BUFFER_PRE_PADDING], len, p_proto);
    if(sent_bytes < 0)
    {
        // not sure how how we went negative, but it will mess-up the math below
        log_err("libwebsocket_write returned a negative result code: %d", sent_bytes);
        return;
    }
    if(sent_bytes < len)
    {
        log_notice("truncated write: only sent %d of %d bytes", sent_bytes, len);

        // TODO: reload the ring buffer with unsent data
        const uint8_t remain_bytes = (len - sent_bytes);
        rb_set_data(p_pd, &buf[LWS_SEND_BUFFER_PRE_PADDING + sent_bytes], remain_bytes);

        // get notified as soon as we can write again
        libwebsocket_callback_on_writable(p_pWsContext, p_pWs);
    }
}

////////////////////////////////////////
int ws_onevent(struct libwebsocket_context* p_pWsContext, struct libwebsocket* p_pWs,
                enum libwebsocket_callback_reasons p_event, void* p_pUser, void* p_pData, size_t p_dataLen)
{
    switch(p_event)
    {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
        {
            log_notice("ws_onevent: LWS_CALLBACK_CLIENT_ESTABLISHED");
            s_ready_state = WS_OPEN;

            // start the heartbeat
            s_heartbeat_timer = date_sec_now();

            ws_onopen();
            break;
        }

        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
        {
            s_ready_state = WS_CLOSED;

            // connect error, queue a reconnect
            s_reconnect_timer = date_sec_now();

            ws_onerror();
            break;
        }

        case LWS_CALLBACK_CLOSED:
        {
            // if the ready state on a close is "closing"
            // then it was initiated by a ws_close request
            const bool force = (WS_CLOSING == s_ready_state);
            s_ready_state = WS_CLOSED;

            // if we were not forced closed, queue a reconnect
            if(!force)
            {
                s_reconnect_timer = date_sec_now();
            }

            ws_onclose(force);
            break;
        }

        case LWS_CALLBACK_CLIENT_RECEIVE:
        {
            char* szData = (char*)p_pData;
            szData[p_dataLen] = '\0';

            ws_onmessage(szData);
            break;
        }

        case LWS_CALLBACK_CLIENT_RECEIVE_PONG:
        {
            char* szData = (char*)p_pData;
            szData[p_dataLen] = '\0';

            ws_onpong(szData);
            break;
        }

        case LWS_CALLBACK_CLIENT_WRITEABLE:
        {
            ws_write_data(&s_ping_buf, LWS_WRITE_PING, p_pWsContext, p_pWs);
            ws_write_data(&s_text_buf, LWS_WRITE_TEXT, p_pWsContext, p_pWs);
            break;
        }

        case LWS_CALLBACK_PROTOCOL_DESTROY:
        {
            log_notice("ws_onevent: LWS_CALLBACK_PROTOCOL_DESTROY (will stay down forever)");
            s_reconnect_timer = 0;
            s_heartbeat_timer = 0;
            s_ready_state = WS_CLOSED;
            s_pWs = NULL;
            s_pWsContext = NULL;
            rb_free(&s_ping_buf);
            rb_free(&s_text_buf);
            break;
        }

        case LWS_CALLBACK_WSI_DESTROY:
        {
            log_notice("ws_onevent: LWS_CALLBACK_WSI_DESTROY (will cause reconnect)");
            // TODO: I believe this means we can set the state closed
            //       causing the reconnect logic to activate
            s_ready_state = WS_CLOSED;
            s_reconnect_timer = date_sec_now();
            break;
        }

        default:
        {
            // debug
            log_trace2("  *****  ws_event callback: [%d]  *****  ", p_event);
            break;
        }
    }

    return(0);
}
