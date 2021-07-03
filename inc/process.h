/**
 * @file process.h
 * @author bwisn (bwisn_dev (at) outlook (dot) com)
 * @brief 
 * @version 0.1
 * @date 24-06-2021
 * 
 * 
 */
#ifndef PROCESS_H_INCLUDED
#define PROCESS_H_INCLUDED
#include <unistd.h>
void process_killEverything();
int process_spawnChildren();
pid_t process_execute(char *prog);
pid_t process_executeTty(char *prog, int tty);

#endif
