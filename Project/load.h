#ifndef LOAD_H
#define LOAD_H

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/user.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>


void printusage(char* name, int exit);
pid_t tracee(char** argv, int mem_limit);
void trace(pid_t tracee, int timeout);
int isNumber(const char* arg);

#endif
