#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include "barber.h"
#include <unistd.h>

int N;
int common;
int shared;
sem_t* sem_p;
sem_t** sems;
sem_t* sem_room;
sem_t* chair;
struct waiting_room* room;
int* waiting;
char name[100] = "";

void usage(const char* name) {
  printf("Usage:\t %s N:\n\n", name);
  printf("\tN - number of seats in waiting room\n");
  exit(0);
}

int isNumber(const char* arg)
{
  for(int i=0;i<strlen(arg);i++) if(arg[i]<'0' || arg[i]>'9') return 0;
  return 1;
}

void g(int signo)
{
  union sigval b;
  for(int i=0; i<N; i++)
  {
    if(waiting[i]!=-1 && sigqueue(waiting[i], 0, b) != -1)
      sigqueue(waiting[i], SIGINT, b);
  }
  munmap(waiting, N*sizeof(int));
  munmap(room, sizeof(struct waiting_room));
  shm_unlink(COMMON_S2);
  shm_unlink(COMMON_S);
  sem_close(sem_p);
  sem_close(sem_room);
  sem_close(chair);
  sem_unlink(CHAIR);
  sem_unlink(SEM_PRIMARY);
  sem_unlink(SEM_ROOM);
  for(int i=0; i<N; i++) sem_close(sems[i]);
  for(int i=0; i<N; i++)
  {
    sprintf(name, "%s%d", SEMS, i);
    sem_unlink(name);
  }
  free(sems);
  exit(EXIT_SUCCESS);
}

int main(int argc, char const *argv[]) {
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGINT);
  struct sigaction newaction;
  newaction.sa_handler = g;
  sigaction(SIGINT, &newaction, NULL);

  struct timespec _time;
  if(argc < 2 || !isNumber(argv[1])) usage(argv[0]);
  N = atoi(argv[1]);
  sems = malloc(N * sizeof(sem_t*));
  // int f = 0;
  common = shm_open(COMMON_S, O_RDWR | O_CREAT, 0666);
  shared = shm_open(COMMON_S2, O_RDWR | O_CREAT, 0666);
  ftruncate(common, sizeof(struct waiting_room));
  ftruncate(shared, N * sizeof(int));
  sem_p = sem_open(SEM_PRIMARY, O_RDWR | O_CREAT, 0666, 0);
  for(int i=0; i<N; i++)
  {
    sprintf(name, "%s%d", SEMS, i);
    sems[i] = sem_open(name, O_RDWR | O_CREAT, 0666, 0);
  }
  sem_room = sem_open(SEM_ROOM, O_RDWR | O_CREAT, 0666, 1);
  chair = sem_open(CHAIR, O_RDWR | O_CREAT, 0666, 0);
  room = mmap(NULL, sizeof(struct waiting_room), PROT_WRITE | PROT_READ, MAP_SHARED, common, 0);
  waiting = mmap(NULL, N * sizeof(int), PROT_WRITE | PROT_READ, MAP_SHARED, shared, 0);
  room->sleeping = 1;
  room->count = N;
  room->taken = 0;
  room->first = 0;
  room->seat = -1;
  for(int i=0; i<N; i++) waiting[i] = -1;
  clock_gettime(CLOCK_MONOTONIC, &_time);
  printf("%f\tBarber is sleeping\n", _time.tv_sec + (double) _time.tv_nsec / 1e9);
  while (1)
  {
    sem_wait(sem_p);
    room->sleeping = 0;
    sem_post(chair);
    sem_wait(sem_p);
    clock_gettime(CLOCK_MONOTONIC, &_time);
    printf("%f\tBarber is starting handling %d\n", _time.tv_sec + (double) _time.tv_nsec / 1e9, room->seat);
    clock_gettime(CLOCK_MONOTONIC, &_time);
    printf("%f\tBarber is ending handling %d\n", _time.tv_sec + (double) _time.tv_nsec / 1e9, room->seat);
    sem_post(chair);
    sem_wait(sem_p);
    sem_wait(sem_room);
    sem_post(chair);
    sem_wait(sem_p);
    while(room->taken > 0) //golenie tych z poczekalni
    {
      sem_post(sem_room);
      sem_post(sems[room->first]);

      sem_wait(sem_p);
      clock_gettime(CLOCK_MONOTONIC, &_time);
      printf("%f\tBarber is starting handling %d\n", _time.tv_sec + (double) _time.tv_nsec / 1e9, room->seat);
      clock_gettime(CLOCK_MONOTONIC, &_time);
      printf("%f\tBarber is ending handling %d\n", _time.tv_sec + (double) _time.tv_nsec / 1e9, room->seat);
      sem_post(chair);
      sem_wait(sem_room);
      sem_wait(sem_p);
      sem_post(chair);
      sem_wait(sem_p);
    }
    room->sleeping = 1;
    sem_post(sem_room);
    clock_gettime(CLOCK_MONOTONIC, &_time);
    printf("%f\tBarber is sleeping\n", _time.tv_sec + (double) _time.tv_nsec / 1e9);
  }
  raise(SIGINT);
  return 0;
  }
