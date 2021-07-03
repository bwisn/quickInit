/**
 * @file signals.c
 * @author bwisn (bwisn_dev (at) outlook (dot) com)
 * @brief 
 * @version 0.1
 * @date 24-06-2021
 * 
 * 
 */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/reboot.h>
#include <sys/wait.h>

#include "config.h"
#include "console.h"
#include "process.h"
#include "svc.h"

/**
 * @brief Blocks all signals
 * 
 */
void signals_blockAll() {
    sigset_t set;
    sigfillset(&set);
    sigprocmask(SIG_BLOCK, &set, NULL);
}

/**
 * @brief Unblocks all signals
 * 
 */
void signals_unblockAll() {
    sigset_t set;
    sigfillset(&set);
    sigprocmask(SIG_BLOCK, &set, NULL);
}

/**
 * @brief Restores default signals
 * 
 */
void signals_restoreDefault() {
    struct sigaction sigact = {.sa_handler = SIG_DFL};
    for (int i = 0; i < SIGRTMAX; i++)  // for each signal, restore it to default
        sigaction(i, &sigact, NULL);
}

/**
 * @brief Internal init handler for reboot, shutdown etc. signals
 * 
 * @param signal 
 */
static void sigHandler(int signal) {
    switch (signal) {
        case SIGTERM:
            console_info("reboot received\r\n");
            svc_stopEnabledServices();
            process_killEverything();
            reboot(RB_AUTOBOOT);
            break;
        case SIGUSR2:
            console_info("poweroff received\r\n");
            svc_stopEnabledServices();
            process_killEverything();
            reboot(RB_POWER_OFF);
            break;
        case SIGUSR1:
            console_info("halt received\r\n");
            svc_stopEnabledServices();
            process_killEverything();
            reboot(RB_HALT_SYSTEM);
            break;
        case SIGCHLD:
            while (waitpid(0, NULL, WNOHANG) >= 0)
                ;
            break;
        default:
            break;
    }
}

/**
 * @brief Setup signals needed for init
 * 
 */
void signals_setup() {
    struct sigaction sa = {
        .sa_handler = sigHandler,
        .sa_flags = SA_RESTART,
    };

    sigset_t dftsigset;  // default sigset

    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &sa.sa_mask, &dftsigset);

    char signals[] = {SIGTERM, SIGINT, SIGPWR, SIGHUP, SIGUSR1, SIGUSR2, SIGKILL, SIGCHLD};
    for (uint8_t i = 0; i < sizeof(signals) / sizeof(signals[0]); i++) {
        sigaddset(&sa.sa_mask, signals[i]);
        sigaction(signals[i], &sa, NULL);
    }

    if (DISABLE_CAD == 1)  // if DISABLE_CAD set to 1 then disable Ctrl-Alt-Del reboot
        reboot(RB_DISABLE_CAD);

    setsid();
}
