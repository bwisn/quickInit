/**
 * @file console.c
 * @author bwisn (bwisn_dev (at) outlook (dot) com)
 * @brief 
 * @version 0.1
 * @date 24-06-2021
 * 
 */
#include "console.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

const char* ANSI_CLEARSCREEN = "\033[2J\033[1;1H";

/**
 * @brief Multiple print in one function
 * 
 * @param type 0 :normal print 1: info 2: error 3: debug
 * @param text string
 * @param ... parameters as in printf
 * @return uint8_t 
 */
static uint8_t printMulti(const uint8_t type, const char* text, va_list message) {
    FILE* f = fopen(MSG_CONSOLE, "w");
    if (f) {
        uint8_t hdrsize = sizeof(COLOR_GRN) + sizeof("quickInit: ") + +sizeof(COLOR_WHT) + sizeof(COLOR_NO);
        char* buf = (char*)malloc((strlen(text) + 1 + hdrsize) * sizeof(char));

        if (type == 0) {
            vfprintf(f, text, message);
        } else if (type == 1) {
            sprintf(buf, COLOR_GRN "quick" COLOR_WHT "Init: " COLOR_NO "%s", text);
            vfprintf(f, buf, message);
        } else if (type == 2) {
            sprintf(buf, COLOR_RED "quick" COLOR_WHT "Init: " COLOR_NO "%s", text);
            vfprintf(f, buf, message);
        } else if (type == 3) {
            sprintf(buf, COLOR_CYA "quick" COLOR_WHT "Init: " COLOR_NO "%s", text);
            vfprintf(f, buf, message);
        }
        fclose(f);
        free(buf);

        return EXIT_SUCCESS;
    } else
        return EXIT_FAILURE;
}

/**
 * @brief Print debug to console
 * 
 * @param text string
 * @param ... parameters as in printf
 * @return uint8_t 
 */
uint8_t console_debug(const char* text, ...) {
#ifdef DEBUG
    va_list message;
    va_start(message, text);
    uint8_t status = printMulti(3, text, message);  // 3 is debug
    va_end(message);
    return status;
#else
    return 0;
#endif
}

/**
 * @brief Print error to console
 * 
 * @param text string
 * @param ... parameters as in printf
 * @return uint8_t EXIT_FAILURE or EXIT_SUCCESS
 */
uint8_t console_error(const char* text, ...) {
    va_list message;
    va_start(message, text);
    uint8_t status = printMulti(2, text, message);  // 2 is error
    va_end(message);
    return status;
}

/**
 * @brief Print info to console
 * 
 * @param text string
 * @param ... parameters as in printf
 * @return uint8_t EXIT_FAILURE or EXIT_SUCCESS
 */
uint8_t console_info(const char* text, ...) {
    va_list message;
    va_start(message, text);
    uint8_t status = printMulti(1, text, message);  // 1 is info
    va_end(message);
    return status;
}

/**
 * @brief Print normal text to console
 * 
 * @param text string
 * @param ... parameters as in printf
 * @return uint8_t EXIT_FAILURE or EXIT_SUCCESS
 */
uint8_t console_print(const char* text, ...) {
    va_list message;
    va_start(message, text);
    uint8_t status = printMulti(0, text, message);  // 0 is normal
    va_end(message);
    return status;
}

/**
 * @brief Print to kmsg
 * 
 * @param text string
 * @param ... parameters as in printf
 * @return uint8_t EXIT_SUCCESS or EXIT_FAILURE
 */
uint8_t printk(const char* text, ...) {
    FILE* f = fopen("/dev/kmsg", "w");
    if (f) {
        va_list message;
        va_start(message, text);
        vfprintf(f, text, message);
        va_end(message);
        fclose(f);
        return EXIT_SUCCESS;
    } else
        return EXIT_FAILURE;
}

/**
 * @brief Clear console using ANSI escape codes
 * 
 */
void console_clearScreen() {
    console_print(ANSI_CLEARSCREEN);  // J2 cursor reset
}

/**
 * @brief Clears tty using ANSI escape codes 
 * 
 * @param tty tty id
 * @return uint8_t EXIT_SUCCESS or EXIT_FAILURE
 */
uint8_t console_clearTty(char* tty) {
    FILE* f = fopen(tty, "w");
    if (f) {
        fprintf(f, ANSI_CLEARSCREEN);
        fclose(f);
        return EXIT_SUCCESS;
    } else
        return EXIT_FAILURE;
}

/**
 * @brief Prints init version
 * 
 */
void console_printVersion() {
    console_info("version %s\r\n", QUICKINIT_VERSION);
}
