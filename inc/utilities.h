/**
 * @file utilities.h
 * @author bwisn (bwisn_dev (at) outlook (dot) com)
 * @brief 
 * @version 0.1
 * @date 24-06-2021
 * 
 * 
 */
#ifndef UTILITIES_H_INCLUDED
#define UTILITIES_H_INCLUDED

#include <stdint.h>
#include <sys/stat.h>

uint8_t util_isMounted(char *dirpath);
uint8_t util_isInFstab(char *dirpath);
uint8_t util_dirExists(const char *dirpath);
void util_dirCreate(const char *dirpath, mode_t mode);
#endif
