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

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h> // _SC_OPEN_MAX
#include <signal.h>

#include "global.h"
#include "ipaddr.h"
#include "msg_proc.h"
#include "websock.h"


////////////////////////////////////////
static bool s_run = false;
void sig_term(int p_signum)
{
    log_notice("received SIGTERM, exiting...");
    s_run = false;
    signal(p_signum, SIG_DFL);
}

////////////////////////////////////////
void sig_hup(int p_signum)
{
    log_notice("received SIGHUP");
}


////////////////////////////////////////
int get_serial(char* p_buf, const size_t p_len)
{
    if(p_len < 10)
    {
        printf("insufficient buffer\n");
        return(-1);
    }

    FILE* pfd = fopen("/dev/mtd2", "r");
    if(NULL == pfd)
    {
        printf("failed to open file\n");
        return(-1);
    }

    if(fseek(pfd, 0x0107, SEEK_SET) < 0)
    {
        printf("fseek failed, err: %d\n", errno);
        return(-1);
    }

    if(9 != fread(p_buf, 1, 9, pfd))
    {
        printf("fread failed\n");
        return(-1);
    }
    p_buf[9] = '\0';

    fclose(pfd);

    return(0);
}


////////////////////////////////////////
int get_cloudcfg(char* p_buf, const size_t p_len)
{
    // sanity
    if(p_len < 16)
    {
        printf("insufficient buffer\n");
        return(-1);
    }

    FILE* pfd = fopen("/etc/config/cloudcfg", "r");
    if(NULL == pfd)
    {
        printf("failed to open file\n");
        return(-1);
    }

    char* line = NULL;
    size_t len = 0;
    const ssize_t rlen = getline(&line, &len, pfd);
    fclose(pfd);
    if(rlen < 0)
    {
        printf("fread failed\n");
        return(-1);
    }

    if(0 == rlen)
    {
        if(line) free(line);
        p_buf[0] = '\0';
        return(0);
    }

    if(rlen > p_len)
    {
        if(line) free(line);
        printf("insufficient buffer\n");
        return(-1);
    }

    for(size_t i=0, imax=(rlen-1); i<imax; ++i)
    {
        p_buf[i] = line[i];
    }
    p_buf[rlen-1] = '\0';

    if(line) free(line);

    return(0);
}



////////////////////////////////////////
int main(int argc, const char** argv)
{
#ifdef RUN_AS_DAEMON
    const pid_t pid = fork();
    if(pid < 0)
    {
        fprintf(stderr, "failed to fork, err: [%s]\n", strerror(errno));
        return(EXIT_FAILURE);
    }
    if(pid > 0)
    {
        // the fork was successful and we are the parent, so exit
        fprintf(stdout, "child process pid: [%d]\n", pid);
        return(EXIT_SUCCESS);
    }

    // pid is 0 so we are the child

    // signals
    signal(SIGCHLD, SIG_DFL); // a child process dies
    signal(SIGTSTP, SIG_IGN); // various tty signals
    signal(SIGTTOU, SIG_IGN); //
    signal(SIGTTIN, SIG_IGN); //
    signal(SIGHUP,  SIG_IGN); // ignore hangup signal

    signal(SIGHUP, sig_hup);

    s_run = true;
    signal(SIGINT, sig_term);
    signal(SIGTERM, sig_term);

    // close all open file descriptors
    const int fd_max = sysconf(_SC_OPEN_MAX);
    for(int fd=0; fd<fd_max; ++fd)
    {
#ifdef DEBUG_TRACE
        if(STDERR_FILENO == fd)
        {
            continue;
        }
#endif // DEBUG_TRACE
        close(fd);
    }

    // open /dev/null three times so file descriptors
    // 0, 1, and 2 can't get mapped to anything else
    const int nullfd = open("/dev/null", O_WRONLY);
    if(nullfd < 0)
    {
        log_trace("failed to open /dev/null for write, err: [%s]", strerror(errno));
        return(EXIT_FAILURE);
    }
    if( (dup2(nullfd, STDOUT_FILENO) < 0) ||
#ifndef DEBUG_TRACE
        (dup2(nullfd, STDERR_FILENO) < 0) ||
#endif // DEBUG_TRACE
        (dup2(nullfd, STDIN_FILENO)  < 0) )
    {
        // we closed all the fd's above, so nobody will see this (for debugging)
        log_trace("failed to open duplicate stdout, stderr, and stdin descriptors, err: [%s]\n", strerror(errno));
        return(EXIT_FAILURE);
    }
    if((STDOUT_FILENO != nullfd) && (STDERR_FILENO != nullfd) && (STDIN_FILENO != nullfd))
    {
        close(nullfd);
    }
#endif // RUN_AS_DAEMON

    // create leases and pid files as 0644
    umask(022);

    // open syslog
    startsyslog();
    log_notice("daemon started");

#ifdef DEBUG
    lws_set_log_level(0x01ff, NULL);
#endif // DEBUG

#ifdef RUN_AS_DAEMON
    // create our own process group
    const pid_t sid = setsid();
    if(sid < 0)
    {
        // TODO: should we care if this fails?
        log_err("could not create new process group, err: [%s]", strerror(errno));
        return(EXIT_FAILURE);
    }
#endif // RUN_AS_DAEMON

    // change the current working directory to root
    if(0 != chdir("/"))
    {
        log_err("could not change working directory to root: /, err: [%s]", strerror(errno));
        return(EXIT_FAILURE);
    }

    // store the pid
    // unlink first to ensure the file does not already exist
    unlink(PIDFILE);
    FILE* pfd = fopen(PIDFILE, "w");
    if(NULL == pfd)
    {
        log_err("could not open pid file: [" PIDFILE "] for exclusive write, errno: [%s]", strerror(errno));
        return(EXIT_FAILURE);
    }
    fprintf(pfd, "%d\n", getpid());
    fclose(pfd);

    // init the serial message processor
    if(!mp_init(SERIAL_PORT, SERIAL_BAUD, SERIAL_USE_E71))
    {
        log_err("failed to open port: [%s]  baud: [%d]  parity: [%s]\n", SERIAL_PORT, SERIAL_BAUD, (SERIAL_USE_E71 ? "E71" : "N81"));
        return(EXIT_FAILURE);
    }

    // kick it at least once to set the creds
    char serno[16] = { 0 };
    if(get_serial(serno, sizeof(serno)) < 0)
    {
        log_err("failed to retrieve serial number, errno: [%s]", strerror(errno));
        strcpy(serno, DEFAULT_SERNO);
    }

    char url[64] = { 0 };
    const size_t pos = sprintf(url, "/ws/%s/", serno);
    while(0 != get_ipv4_addresses("%20%7c%20", (url+pos), (sizeof(url)-pos-1)))
    {
        // check for an ip address every 2 sec
        sleep_ms(2000);
    }

    char webhost[64] = { 0 };
    const int rc = get_cloudcfg(webhost, sizeof(webhost));
    if(0 != rc)
    {
        log_debug("failed to read /etc/config/cloudcfg, using default WEB_HOST value");
        strcpy(webhost, WEB_HOST);
    }

    log_notice("connecting to host: [%s]", webhost);
    ws_connect(webhost, url, 443, 1);

    // worker loop
    while(s_run)
    {
        ws_poll();
    }

    // cleanup
    log_notice("daemon closed");
    ws_close();
    mp_close();
    closesyslog();
    unlink(PIDFILE);

    return(EXIT_SUCCESS);
}
