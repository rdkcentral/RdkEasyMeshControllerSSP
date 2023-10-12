/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright (c) 2021-2022 AirTies Wireless Networks
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Copyright (c) 2015 RDK Management
 * Licensed under the Apache License, Version 2.0
 *
 * Copyright (c) 2014 Cisco Systems, Inc.
 * Licensed under the Apache License, Version 2.0
*/

/*********************************************************************************

    description:

        This is the template file of ssp_main.c for XxxxSsp.
        Please replace "XXXX" with your own ssp name with the same up/lower cases.

  ------------------------------------------------------------------------------

    revision:

        09/08/2011    initial revision.

**********************************************************************************/

#ifdef __GNUC__
#ifndef _BUILD_ANDROID
#include <execinfo.h>
#endif
#endif

#include <stdlib.h>

#ifdef INCLUDE_BREAKPAD
#include <breakpad_wrapper.h>
#endif

#include <ccsp_dm_api.h>

#include "ssp_global.h"

#define DEBUG_INI_NAME  "/etc/debug.ini"

extern int map_ctrl_main(bool ebtables, bool wfa_cert);
extern void map_controller_stop(void);

extern ANSC_HANDLE bus_handle;
extern char *pComponentName;
char g_Subsystem[32] = {0};

static int cmd_dispatch(int command)
{
    switch (command) {
        case 'e':
#ifdef _ANSC_LINUX
            CcspTraceInfo(("Connect to bus daemon...\n"));

            {
                char CName[256];

                if (g_Subsystem[0] != 0) {
                    _ansc_sprintf(CName, "%s%s", g_Subsystem,
                        CCSP_COMPONENT_ID_EMCTL);
                } else {
                    _ansc_sprintf(CName, "%s", CCSP_COMPONENT_ID_EMCTL);
                }

                ssp_Mbi_MessageBusEngage(CName, CCSP_MSG_BUS_CFG,
                     CCSP_COMPONENT_PATH_EMCTL);
            }
#endif
            ssp_create();
            ssp_engage();
            break;

        case 'm':
            AnscPrintComponentMemoryTable(pComponentName);
            break;

        case 't':
            AnscTraceMemoryTable();
            break;

        case 'c':
            ssp_cancel();
            break;

        default:
            break;
    }

    return 0;
}

#if defined(_ANSC_LINUX)
static void daemonize(void)
{
#ifndef _DEBUG
    int fd;
#endif /* _DEBUG */

    switch (fork()) {
        case 0:
            break;

        case -1:
            // Error
            CcspTraceInfo(("Error daemonizing (fork)! %d - %s\n", errno,
                strerror(errno)));
            exit(0);
            break;

        default:
            _exit(0);
    }

    if (setsid() < 0) {
        CcspTraceInfo(("Error demonizing (setsid)! %d - %s\n", errno,
            strerror(errno)));
        exit(0);
    }

//  chdir("/");

#ifndef _DEBUG
    fd = open("/dev/null", O_RDONLY);
    if (fd != 0) {
        dup2(fd, 0);
        close(fd);
    }
    fd = open("/dev/null", O_WRONLY);
    if (fd != 1) {
        dup2(fd, 1);
        close(fd);
    }
    fd = open("/dev/null", O_WRONLY);
    if (fd != 2) {
        dup2(fd, 2);
        close(fd);
    }
#endif /* _DEBUG */
}

#ifndef INCLUDE_BREAKPAD
static void _print_stack_backtrace(void)
{
#ifdef __GNUC__
#ifndef _BUILD_ANDROID
    void *tracePtrs[100];
    char **funcNames = NULL;
    int i, count = 0;

    count = backtrace(tracePtrs, 100);
    backtrace_symbols_fd(tracePtrs, count, 2);

    funcNames = backtrace_symbols(tracePtrs, count);

    if (funcNames) {
        // Print the stack trace
        for (i = 0; i < count; i++) {
            printf("%s\n", funcNames[i]);
        }

        // Free the string pointers
        free(funcNames);
    }
#endif
#endif
}

static void signal_stop_handler(int signum)
{
    if (signum != SIGINT) {
        _print_stack_backtrace();
    }
    CcspTraceInfo(("Signal %d received, exiting!\n", signum));
    map_controller_stop();
}

static void signal_ignore_handler(int signum)
{
    UNREFERENCED_PARAMETER(signum);
}
#endif /* INCLUDE_BREAKPAD */
#endif /* _ANSC_LINUX */

int main(int argc, char *argv[])
{
    BOOL bRunAsDaemon = TRUE;
    int  idx = 0;
    char *subSys = NULL;
#ifndef INCLUDE_BREAKPAD
    struct sigaction sig_stop_action;
    struct sigaction sig_no_reaction;
    int  sa_ret = 0;
#endif
    bool ebtables = false;
    bool wfa_cert = false;
    DmErr_t err;

#if defined(FEATURE_SUPPORT_RDKLOG)
    RDK_LOGGER_INIT();
#endif

    for (idx = 1; idx < argc; idx++) {
        if ((strcmp(argv[idx], "-subsys") == 0)) {
            AnscCopyString(g_Subsystem, argv[idx + 1]);
        } else if (strcmp(argv[idx], "-c") == 0) {
            bRunAsDaemon = FALSE;
        } else if (strcmp(argv[idx], "-e") == 0) {
            ebtables = true;
        } else if (strcmp(argv[idx], "-w") == 0) {
            wfa_cert = true;
        }
    }

    pComponentName = CCSP_COMPONENT_NAME_EMCTL;

#if defined(_DEBUG) && defined(_COSA_SIM_)
    AnscSetTraceLevel(CCSP_TRACE_LEVEL_INFO);
#endif

#if defined(_ANSC_WINDOWSNT)
    int cmdChar = 0;

    AnscStartupSocketWrapper(NULL);

    cmd_dispatch('e');

    while (cmdChar != 'q') {
        cmdChar = getchar();

        cmd_dispatch(cmdChar);
    }
#elif defined(_ANSC_LINUX)
    if (bRunAsDaemon) {
        daemonize();
    }

#ifdef INCLUDE_BREAKPAD
    breakpad_ExceptionHandler();
#else /* INCLUDE_BREAKPAD */
    /* Signal handlers */
    sig_stop_action.sa_handler = signal_stop_handler;
    sigemptyset(&sig_stop_action.sa_mask);
    sig_stop_action.sa_flags = 0;

    sa_ret |= sigaction(SIGTERM, &sig_stop_action, NULL);
    sa_ret |= sigaction(SIGINT, &sig_stop_action, NULL);
    sigaction(SIGSEGV, &sig_stop_action, NULL);
    sigaction(SIGBUS, &sig_stop_action, NULL);
    sigaction(SIGKILL, &sig_stop_action, NULL);
    sigaction(SIGFPE, &sig_stop_action, NULL);
    sigaction(SIGILL, &sig_stop_action, NULL);
    sigaction(SIGQUIT, &sig_stop_action, NULL);
    sigaction(SIGHUP, &sig_stop_action, NULL);

    sig_no_reaction.sa_handler = signal_ignore_handler;
    sigemptyset(&sig_no_reaction.sa_mask);
    sig_no_reaction.sa_flags = 0;
    sa_ret |= sigaction(SIGPIPE, &sig_no_reaction, NULL);
    sa_ret |= sigaction(SIGALRM, &sig_no_reaction, NULL);
    sa_ret |= sigaction(SIGUSR1, &sig_no_reaction, NULL);
    sa_ret |= sigaction(SIGUSR2, &sig_no_reaction, NULL);

    if (sa_ret) {
        fprintf(stderr, "sigaction failed\n");
        exit(1);
    }
#endif /* INCLUDE_BREAKPAD */
#endif

    cmd_dispatch('e');
#ifdef _COSA_SIM_
    subSys = "";        /* PC simu use empty string as subsystem */
#else
    subSys = NULL;      /* use default sub-system */
#endif
    err = Cdm_Init(bus_handle, subSys, NULL, NULL, pComponentName);
    if (err != CCSP_SUCCESS) {
        fprintf(stderr, "Cdm_Init: %s\n", Cdm_StrError(err));
        exit(1);
    }

    system("touch /tmp/emctl_initialized");

    if (bRunAsDaemon) {
        map_ctrl_main(ebtables, wfa_cert);
    } else {
        map_ctrl_main(ebtables, wfa_cert);
    }

    err = Cdm_Term();
    if (err != CCSP_SUCCESS) {
        fprintf(stderr, "Cdm_Term: %s\n", Cdm_StrError(err));
        exit(1);
    }

    ssp_cancel();

    return 0;
}
