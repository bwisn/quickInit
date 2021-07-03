/**
 * @file console.h
 * @author bwisn (bwisn_dev (at) outlook (dot) com)
 * @brief 
 * @version 0.1
 * @date 24-06-2021
 * 
 * 
 */
#ifndef CONSOLE_H_INCLUDED
#define CONSOLE_H_INCLUDED

#include <stdint.h>

/**
 * @brief ANSI no color
 * 
 */
#define COLOR_NO "\x1B[0m"

/**
 * @brief ANSI red color
 * 
 */
#define COLOR_RED "\x1B[31m"

/**
 * @brief ANSI green color
 * 
 */
#define COLOR_GRN "\x1B[32m"

/**
 * @brief ANSI yellow color
 * 
 */
#define COLOR_YEL "\x1B[33m"

/**
 * @brief ANSI blue color
 * 
 */
#define COLOR_BLU "\x1B[34m"

/**
 * @brief ANSI magenta color
 * 
 */
#define COLOR_MAG "\x1B[35m"

/**
 * @brief ANSI cyan color
 * 
 */
#define COLOR_CYA "\x1B[36m"

/**
 * @brief ANSI white color
 * 
 */
#define COLOR_WHT "\x1B[37m"

uint8_t console_print(const char* text, ...);
uint8_t console_error(const char* text, ...);
uint8_t console_info(const char* text, ...);
uint8_t console_debug(const char* text, ...);
uint8_t printk(const char* text, ...);
void console_clearScreen();
uint8_t console_clearTty(char* tty);
void console_printVersion();

#endif
