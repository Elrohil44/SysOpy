#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "barber.h"

int id = -1;
int common;
int shared;
sem_t* sem_p;
sem_t* sems;
sem_t* sem_room, *chair;
struct waiting_room* room;
int* waiting;
int* PIDS;
int N;
int doing = 0;
int leave = 0;
char name[100] = "";

void usage(const char* name) {
  printf("Usage:\t %s N S:\n\n", name);
  printf("\tN - number of clients\n");
  printf("\tS - number of barbing\n");
  exit(0);
}

int isNumber(const char* arg)
{
  for(int i=0;i<strlen(arg);i++) if(arg[i]<'0' || arg[i]>'9') return 0;
  return 1;
}

void g(int signo)
{
  if(!doing)
  {
    union sigval b;
    if(id != -1) waiting[id] = -1;
    for(int i=0; i<N; i++)
    {
      if(PIDS[i]>0 && sigqueue(PIDS[i], 0, b)!=-1)
        sigqueue(PIDS[i], SIGINT, b);
    }
    munmap(waiting, N*sizeof(int));
    munmap(room, sizeof(struct waiting_room));
    sem_close(sem_p);
    sem_close(sem_room);
    sem_close(chair);
    free(PIDS);
    exit(EXIT_SUCCESS);
  }
  leave = 1;
}

int main(int argc, char const *argv[]) {
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGINT);
  struct sigaction newaction;
  newaction.sa_handler = g;
  sigaction(SIGINT, &newaction, NULL);

  int pid;
  if(argc < 3 || !isNumber(argv[1]) || !isNumber(argv[2])) usage(argv[0]);
  N = atoi(argv[1]);
  int S = atoi(argv[2]);

  PIDS = malloc(N * sizeof(int));
  for(int i=0; i<N; i++) PIDS[i] = -1;

  struct timespec _time;
  common = shm_open(COMMON_S, O_RDWR, 0666);
  shared = shm_open(COMMON_S2, O_RDWR, 0666);
  ftruncate(common, sizeof(struct waiting_room));
  ftruncate(shared, N * sizeof(int));
  sem_p = sem_open(SEM_PRIMARY, O_RDWR);
  sem_room = sem_open(SEM_ROOM, O_RDWR);
  chair = sem_open(CHAIR, O_RDWR);
  room = mmap(NULL, sizeof(struct waiting_room), PROT_WRITE | PROT_READ, MAP_SHARED, common, 0);
  waiting = mmap(NULL, N * sizeof(int), PROT_WRITE | PROT_READ, MAP_SHARED, shared, 0);
  for(int i=0; i<N; i++)
  {
    if((PIDS[i] = fork()) != 0) continue;
    pid = getpid();
    int j = 0;
    while(j<S)
    {
      doing = 0;
      if(leave) break;
      sem_wait(sem_room);
      if(room->sleeping)
      {
        sem_post(sem_p);//budzenie
        sem_wait(chair);
        sem_post(sem_room);
        doing = 1;
        clock_gettime(CLOCK_MONOTONIC, &_time);
        printf("%f\tWakeing up the barber %d\n", _time.tv_sec + (double) _time.tv_nsec / 1e9, pid);
        room->seat = pid;
        id = -1;
        sem_post(sem_p);
        sem_wait(chair);
        room->seat = -1;
        clock_gettime(CLOCK_MONOTONIC, &_time);
        printf("%f\tSuccesful barbing, leaving... %d\n", _time.tv_sec + (double) _time.tv_nsec / 1e9, pid);
        sem_post(sem_p);
        sem_wait(chair);
        sem_post(sem_p);
      }
      else
      {
        if(room->taken < room->count)
        {
          id = (room->first+room->taken)%room->count;
          room->taken++;
          waiting[id] = pid;
        }
        else
        {
          sem_post(sem_room);
          clock_gettime(CLOCK_MONOTONIC, &_time);
          printf("%f\tLimit reached, leaving... %d\n", _time.tv_sec + (double) _time.tv_nsec / 1e9, pid);
          continue;
        }
        sprintf(name, "%s%d", SEMS, id);
        sems = sem_open(name, O_RDWR);
        clock_gettime(CLOCK_MONOTONIC, &_time);
        printf("%f\tWaiting... %d\n", _time.tv_sec + (double) _time.tv_nsec / 1e9, pid);
        sem_post(sem_room);
        sem_wait(sems);
        doing = 1;//czekanie//siadanie
        sem_close(sems);
        sem_wait(sem_room);
        room->seat = pid;
        waiting[id] = -1;
        id = -1;
        sem_post(sem_room);

        sem_post(sem_p);
        sem_wait(chair);
        room->seat = -1;
        clock_gettime(CLOCK_MONOTONIC, &_time);
        printf("%f\tSuccesful barbing, leaving... %d\n", _time.tv_sec + (double) _time.tv_nsec / 1e9, pid);
        sem_post(sem_p);
        sem_wait(chair);
        sem_post(sem_p);
      }
      j++;
    }
    if(leave) raise(SIGINT);
    exit(EXIT_SUCCESS);
  }
  int status;
  for(int i=0; i<N; i++) waitpid(PIDS[i], &status, 0);
  raise(SIGINT);
  return 0;
  }
