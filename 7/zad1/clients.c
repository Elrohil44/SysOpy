#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
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
int sem_p;
int sems;
int sem_room;
struct waiting_room* room;
int* waiting;
int* PIDS;
int N;
int doing = 0;
int leave = 0;

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
    shmdt(waiting);
    shmdt(room);
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
  struct sembuf op;
  struct sembuf invite;
  struct sembuf wait;
  struct timespec _time;
  common = shmget(ftok(getenv("HOME"), COMMON_S), 0, 0);
  shared = shmget(ftok(getenv("HOME"), COMMON_S2), 0, 0);
  sem_p = semget(ftok(getenv("HOME"), SEM_PRIMARY), 0, 0);
  sems = semget(ftok(getenv("HOME"), SEMS), 0, 0);
  sem_room = semget(ftok(getenv("HOME"), SEM_ROOM), 0, 0);
  room = shmat(common, NULL, 0);
  waiting = shmat(shared, NULL, 0);
  op.sem_flg = 0;
  op.sem_num = 0;
  op.sem_op = -1;
  wait.sem_flg = 0;
  wait.sem_num = 0;
  invite.sem_flg = 0;
  invite.sem_op = 0;
  for(int i=0; i<N; i++)
  {
    if((PIDS[i] = fork()) != 0) continue;
    pid = getpid();
    int j = 0;
    while(j<S)
    {
      doing = 0;
      if(leave) break;
      wait.sem_op = -1;
      semop(sem_room, &wait, 1);
      if(room->sleeping)
      {
        semop(sem_p, &op, 1);
        semop(sem_p, &op, 1);
        wait.sem_op = 1;
        semop(sem_room, &wait, 1); //budzenie
        doing = 1;
        clock_gettime(CLOCK_MONOTONIC, &_time);
        printf("%f\tWakeing up the barber %d\n", _time.tv_sec + (double) _time.tv_nsec / 1e9, pid);
        room->seat = pid;
        id = -1;
        semop(sem_p, &op, 1);//usiadl
        semop(sem_p, &op, 1);//wstal
        room->seat = -1;
        clock_gettime(CLOCK_MONOTONIC, &_time);
        printf("%f\tSuccesful barbing, leaving... %d\n", _time.tv_sec + (double) _time.tv_nsec / 1e9, pid);
        semop(sem_p, &op, 1);
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
          wait.sem_op = 1;
          semop(sem_room, &wait, 1);
          clock_gettime(CLOCK_MONOTONIC, &_time);
          printf("%f\tLimit reached, leaving... %d\n", _time.tv_sec + (double) _time.tv_nsec / 1e9, pid);
          continue;
        }
        invite.sem_num = id;
        clock_gettime(CLOCK_MONOTONIC, &_time);
        printf("%f\tWaiting... %d\n", _time.tv_sec + (double) _time.tv_nsec / 1e9, pid);
        wait.sem_op = 1;
        semop(sem_room, &wait, 1);
        invite.sem_op = -1;
        semop(sems, &invite, 1);
        doing = 1;//czekanie//siadanie
        wait.sem_op = -1;
        semop(sem_room, &wait, 1);
        room->seat = pid;
        waiting[id] = -1;
        id = -1;
        wait.sem_op = 1;
	room->taken--;
	room->first = (room->first + 1)%room->count;
        clock_gettime(CLOCK_MONOTONIC, &_time);
        printf("%f\tLeft the waiting room %d\n", _time.tv_sec + (double) _time.tv_nsec / 1e9, pid);
        semop(sem_room, &wait, 1);
        semop(sem_p, &op, 1);
        semop(sem_p, &op, 1);
        room->seat = -1;
        clock_gettime(CLOCK_MONOTONIC, &_time);
        printf("%f\tSuccesful barbing, leaving... %d\n", _time.tv_sec + (double) _time.tv_nsec / 1e9, pid);
        semop(sem_p, &op, 1);
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
