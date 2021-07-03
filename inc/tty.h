/**
 * @file tty.h
 * @author bwisn (bwisn_dev (at) outlook (dot) com)
 * @brief 
 * @version 0.1
 * @date 24-06-2021
 * 
 * 
 */
#ifndef TTY_H_INCLUDED
#define TTY_H_INCLUDED
#include <stdint.h>
#include <time.h>

/**
 * @brief Maximum number of ttys in the system
 * 
 */
#define TTY_MAXTTYS 128

/**
 * @brief Maximum line length in tty config file
 * 
 */
#define TTY_MAX_LINELEN 256

/**
 * @brief Maximum device name length in tty config file
 * 
 */
#define TTY_MAX_DEVNAMELEN 64

/**
 * @brief Maximum command length in tty config file
 * 
 */
#define TTY_MAX_CMDLEN 128  // tty max command length

/**
 * @brief Minimum interval between tty restarts [seconds]
 * 
 */
#define TTY_STARTINTERVAL 1

/*! \cond PRIVATE */
#define TTY_STATE_STOPPED 0
#define TTY_STATE_RUNNING 1
#define TTY_ACTION_ASKFIRST 0
#define TTY_ACTION_ONCE 1
#define TTY_ACTION_RESPAWN 2
/*! \endcond */

uint8_t tty_start(uint8_t tty);
uint8_t tty_stop(uint8_t tty);
uint8_t tty_check(uint8_t pid);
uint8_t tty_respawn(uint8_t pid);
void tty_init();

/**
 * @brief Structure storing informations about specific TTY
 * 
 */
struct tty_struct {
    /**
     * @brief PID of the process
     * 
     */
    pid_t pid;

    /**
     * @brief timestamp when the TTY was started last time
     * 
     */
    time_t time;

    /**
     * @brief Status of the TTY eg. running or not
     * 
     */
    uint8_t state;

    /**
     * @brief Action which should be done after starting eg. askfirst (0)
     * 
     */
    uint8_t action;

    /**
     * @brief 1 if the device is a tty
     * 
     */
    uint8_t is_tty;

    /**
     * @brief tty name or empty, without beginning /dev/ eg. ttyS1
     * 
     */
    char dev[TTY_MAX_DEVNAMELEN];

    /**
     * @brief Command which has to be run after starting
     * 
     */
    char command[TTY_MAX_CMDLEN];
};

extern struct tty_struct tty_data[TTY_MAXTTYS];

#endif
