/**
 * @file process.c
 * @author bwisn (bwisn_dev (at) outlook (dot) com)
 * @brief 
 * @version 0.1
 * @date 24-06-2021
 * 
 * 
 */
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <sys/reboot.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "console.h"
#include "signals.h"
#include "tty.h"

static char *ENV[] = {
    "TERM=vt100",
    "HOME=/",
    "PATH=/usr/bin:/bin:/usr/sbin:/sbin",
    "SHELL=/bin/sh",
    "USER=root",
    NULL};

/**
 * @brief Kills everything (use before shutdown/reboot)
 * 
 */
void process_killEverything() {
    reboot(RB_ENABLE_CAD);

    console_info("Sending SIGTERM to all processes\r\n");
    kill(-1, SIGTERM);
    sync();
    sleep(1);

    console_info("Sending SIGKILL to all processes\r\n");
    kill(-1, SIGKILL);
    sync();
    sleep(1);
}

/**
 * @brief Spawns a children which will keep the init alive
 * 
 * @return uint8_t EXIT_SUCCESS or EXIT_FAILURE
 */
uint8_t process_spawnChildren() {
    int i;
    pid_t pid;

    signals_blockAll();
    pid = fork();

    if (pid < 0) {
        return EXIT_FAILURE;
    } else if (pid > 0) {
        for (;;) {
            wait(&i);
        }
    }

    signals_unblockAll();
    setsid();
    setpgid(0, 0);
    return EXIT_SUCCESS;
}

/**
 * @brief Redirect streams to device
 * 
 * @param tty path to device such as "/dev/ttyS0"
 * @param which e,i,o error input output
 * @return uint8_t EXIT_SUCCESS or EXIT_FAILURE
 */
static uint8_t redirectStdio(const char *tty, char *which) {
    int fd = open(tty, O_RDWR);
    if (fd) {
        for (; *which != 0; which++) {
            switch (*which) {
                case 'e':
                    dup2(fd, STDERR_FILENO);
                    break;
                case 'i':
                    dup2(fd, STDIN_FILENO);
                    break;
                case 'o':
                    dup2(fd, STDOUT_FILENO);
                    break;
                default:
                    break;
            }
        }
        close(fd);
        return EXIT_SUCCESS;
    } else {
        console_info("failed while redirecting stdio\r\n");
        return EXIT_SUCCESS;
    }
}
/**
 * @brief Detach or attach to controlling terminal
 * 
 * @param devbuf path to device
 * @param detach 0: attach, 1: detach
 * @return uint8_t EXIT_FAILURE or EXIT_SUCCESS
 */
static uint8_t detachTty(const char *devbuf, const uint8_t detach) {
    int fd = open(devbuf, O_RDWR);
    if (fd) {
        if (detach) {  // detach a tty
            if (ioctl(fd, TIOCNOTTY, NULL) < 0)
                console_error("failed detaching from controlling terminal: %s\r\n", strerror(errno));
        } else {                           // attach a tty
            if (ioctl(fd, TIOCSCTTY) < 0)  // force attach to controlling terminal
                console_error("failed attaching to controlling terminal: %s\r\n", strerror(errno));
        }
        close(fd);
        return EXIT_SUCCESS;
    } else {
        close(fd);
        return EXIT_FAILURE;
    }
}

/**
 * @brief Execute program or tty
 * 
 * @param prog char* program with parameters
 * @param istty 1 if it's tty, 0 if it's not  
 * @param tty number of tty
 * @return pid_t pid
 */
static pid_t executeProg(char *prog, const uint8_t istty, const uint8_t tty) {
    int MAXARGS = 255;
    int i = 0;
    int status;
    char *argv[MAXARGS];

    char *buf = (char *)malloc((strlen(prog) + 1) * sizeof(char));
    strcpy(buf, prog);
    argv[i = 0] = strtok(buf, " \n");
    while (i < MAXARGS && argv[i] != NULL)
        argv[++i] = strtok(NULL, " \n");

    argv[++i] = NULL;  // NUL on end
    free(buf);

    if (istty == 1) {
        detachTty(tty_data[tty].dev, 1);  // detach from controlling terminal
    }

    pid_t pid = fork();

    if (pid < 0) {
        console_error("fork failed\r\n");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {  // is a child

        if (istty == 1) {
            vhangup();                                // ensure that terminal is clean
            redirectStdio(tty_data[tty].dev, "eio");  // redirect stdio stderr stdout to tty
        } else {
            redirectStdio("/dev/null", "eio");
        }

        signals_unblockAll();
        signals_restoreDefault();

        setsid();
        setpgid(0, getpid());

        if (istty == 1) {
            detachTty(tty_data[tty].dev, 0);  // attach to controlling terminal
            if (tty_data[tty].action == TTY_ACTION_ASKFIRST) {
                char *pname_buf = "tty_askfirst";
                prctl(PR_SET_NAME, pname_buf);

                printf("Please press Enter to activate this console. \r\n");
                char c;
                while (read(STDIN_FILENO, &c, 1) == 1 && c != '\n')
                    ;
                console_clearTty(tty_data[tty].dev);
            }
        }
        if (execve(argv[0], argv, ENV) < 0) {
            console_error("exec failed\r\n");
            exit(1);
        } else {
            while (wait(&status) != pid)
                ;
        }
    }
    return pid;
}

/**
 * @brief Execute a program
 * 
 * @param prog path with parameters
 * @return pid_t pid of a program
 */
pid_t process_execute(char *prog) {
    return executeProg(prog, 0, 0);  // isn't tty, 0
}

/**
 * @brief Execute a program on tty
 * 
 * @param prog path with parameters
 * @param tty tty number 
 * @return pid_t pid of a program
 */
pid_t process_executeTty(char *prog, int tty) {
    return executeProg(prog, 1, tty);  // istty, tty number
}
