/**
 * @file tty.c
 * @author bwisn (bwisn_dev (at) outlook (dot) com)
 * @brief 
 * @version 0.1
 * @date 24-06-2021
 * 
 * 
 */
#include "tty.h"

#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

#include "config.h"
#include "console.h"
#include "process.h"

static uint8_t tty_count = 0;

struct tty_struct tty_data[TTY_MAXTTYS];

/**
 * @brief Start a tty
 * Function starts tty using a tty id.
 * 
 * @param tty id of tty
 * @return uint8_t EXIT_SUCCESS or EXIT_FAILURE
 */
uint8_t tty_start(uint8_t tty) {
    if (tty > tty_count)  // if tty is bigger than tty set up
        return EXIT_FAILURE;

    if (tty_data[tty].state == TTY_STATE_RUNNING)
        return EXIT_FAILURE;

    if (tty_data[tty].pid != 0) {
        if (difftime(time(NULL), tty_data[tty].time) < TTY_STARTINTERVAL)  // prevent spamming
            return EXIT_FAILURE;
    }

    tty_data[tty].time = time(NULL);  // set time to current

    if (tty_data[tty].is_tty) {
        console_clearTty(tty_data[tty].dev);
        tty_data[tty].pid = process_executeTty(tty_data[tty].command, tty);
    } else {
        tty_data[tty].pid = process_execute(tty_data[tty].command);
    }

    tty_data[tty].state = TTY_STATE_RUNNING;
    console_debug("started %s with PID=%d\r\n", tty_data[tty].dev, tty_data[tty].pid);
    return EXIT_SUCCESS;
}

/**
 * @brief Stop a tty
 * Function stops tty using a tty id.
 * 
 * @param tty id of tty
 * @return uint8_t EXIT_SUCCESS or EXIT_FAILURE
 */
uint8_t tty_stop(uint8_t tty) {
    if (tty > tty_count)
        return EXIT_FAILURE;

    if (tty_data[tty].state != TTY_STATE_RUNNING)
        return EXIT_FAILURE;

    kill(tty_data[tty].pid, SIGTERM);
    kill(tty_data[tty].pid, SIGKILL);
    tty_data[tty].pid = 0;
    tty_data[tty].state = TTY_STATE_STOPPED;
    console_debug("stopped %s with PID=%d\r\n", tty_data[tty].dev, tty_data[tty].pid);
    return EXIT_SUCCESS;
}

/**
 * @brief Respawn a tty
 * Function respawns tty using a tty id.
 * 
 * @param tty id of tty
 * @return uint8_t EXIT_SUCCESS or EXIT_FAILURE
 */
uint8_t tty_respawn(uint8_t tty) {
    if (tty > tty_count)
        return EXIT_FAILURE;

    if (getpgid(tty_data[tty].pid) <= 0) {  // check if tty is running or it's killed
        if (tty_data[tty].action == TTY_ACTION_RESPAWN) {
            if (tty_data[tty].state == TTY_STATE_RUNNING) {  // if it isn't running but it should - restart it
                console_debug("respawning %s\r\n", tty_data[tty].dev);
                tty_stop(tty);
                tty_start(tty);
            }
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief TTY respawn thread function
 * 
 */
static void *tty_thread() {
    console_debug("tty watchdog started\r\n");
    while (1) {
        for (uint8_t i = 0; i < tty_count; i++) {
            if (tty_respawn(i) == EXIT_FAILURE) {
                console_error("error occured while respawning tty!\r\n");
            }
            sleep(1);
        }
    }
    return NULL;
}

/**
 * @brief Read and parse tty configfile
 * File format:
 * /dev/tty[xyz]:[runlevel]:[respawn askfirst once]:[command]
 * @return uint8_t EXIT_SUCCESS or EXIT_FAILURE
 */
static uint8_t readTtyFile() {
    FILE *fp;
    fp = fopen("/etc/quickinit/tty", "r");
    if (fp == NULL) {
        console_error("tty configuration not found!\r\n");
        return EXIT_FAILURE;
    }

    char *file_buf = (char *)malloc((TTY_MAX_LINELEN + 1) * sizeof(char));

    uint8_t i = 0;
    while (fgets(file_buf, TTY_MAX_LINELEN, fp) != NULL) {
        if (strncmp(file_buf, "#", 1) != 0 && strncmp(file_buf, "//", 2) != 0) {  // ignore lines beggining with ; or // or #
            uint8_t field = 0;                                                    //current field in line
            uint8_t error = 0;                                                    // error in current line
            struct tty_struct tty_buf;
            tty_buf.state = TTY_STATE_STOPPED;
            tty_buf.time = 0;

            strtok(file_buf, "\n");  // strange way to remove newline

            char *ttystr;
            char *strsep_buf = strdup(file_buf);
            while ((ttystr = strsep(&strsep_buf, ":")) != NULL) {
                switch (field) {
                    case 0:                                   // device
                        if (strstr(ttystr, "tty") != NULL) {  //, add /dev before
                            char ttydev[64];
                            snprintf(ttydev, 64, "/dev/%s", ttystr);
                            strcpy(tty_buf.dev, ttydev);
                            tty_buf.is_tty = 1;
                        } else {
                            tty_buf.is_tty = 0;
                        }
                        break;
                    case 1:  // runlevel
                             /* runlevels are not supported */
                        break;
                    case 2:  // action
                        if (strstr(ttystr, "askfirst") != NULL) {
                            tty_buf.action = TTY_ACTION_ASKFIRST;
                        } else if (strstr(ttystr, "once") != NULL) {
                            tty_buf.action = TTY_ACTION_ONCE;
                        } else if (strstr(ttystr, "respawn") != NULL) {
                            tty_buf.action = TTY_ACTION_RESPAWN;
                        } else {
                            error = 1;
                        }
                        break;
                    case 3:  // command
                        strcpy(tty_buf.command, ttystr);
                        break;
                }
                field++;

                if (error > 0)
                    break;
            }

            if (field > 2) {  // there were all fields in the line
                tty_data[i] = tty_buf;
                i++;
            }
        }
    }

    tty_count = i;
    free(file_buf);

    if (fclose(fp) != 0)
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

/**
 * @brief Creates new thread which keeps respawning ttys
 * 
 */
static void watchdog() {
    pthread_t watchdog_thread;

    if (pthread_create(&watchdog_thread, NULL, tty_thread, NULL)) {
        console_error("error creating thread\r\n");
    }
}

/**
 * @brief Initializes tty system
 * 
 * 
 */
void tty_init() {
    readTtyFile();

    for (uint8_t i = 0; i < tty_count; i++) {
        tty_data[i].pid = 0;  // set pid to 0 as it isn't running
        tty_start(i);
    }

    watchdog();
}
