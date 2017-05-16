#ifndef MONITOR_H
#define MONITOR_H

#include <semaphore.h>
#include <sys/types.h>


#define MAX_READERS 100
#define LENGTH 10000

const char MEMORY[] = "/monitoringstruct";

struct state
{
  pthread_mutex_t mutex;
  int first;
  pthread_cond_t firstchange;
  pthread_t* queue;
  int is_read;
  pthread_cond_t readchange;
  int last;
  int size;
  int data[LENGTH];
};

#endif
