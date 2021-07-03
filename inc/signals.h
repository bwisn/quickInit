/**
 * @file signals.h
 * @author bwisn (bwisn_dev (at) outlook (dot) com)
 * @brief 
 * @version 0.1
 * @date 24-06-2021
 * 
 * 
 */
#ifndef SIGNALS_H_INCLUDED
#define SIGNALS_H_INCLUDED

#include <stdint.h>

void signals_setup();
void signals_restoreDefault();
void signals_blockAll();
void signals_unblockAll();
#endif
