#ifndef MONITOR_H
#define MONITOR_H

#include <semaphore.h>
#include <sys/types.h>


#define MAX_READERS 100
#define LENGTH 10000

const char MEMORY[] = "/monitoringstruct";

struct state
{
  sem_t sem;
  sem_t semread;
  int is_read;
  int data[LENGTH];
};

#endif
