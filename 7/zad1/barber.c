#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
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
int sem_p;
int sems;
int sem_room;
struct waiting_room* room;
int* waiting;

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
  if(room->seat!=-1 && sigqueue(room->seat, 0, b) != -1)
    sigqueue(room->seat, SIGINT, b);
  for(int i=0; i<N; i++)
  {
    if(waiting[i]!=-1 && sigqueue(waiting[i], 0, b) != -1)
      sigqueue(waiting[i], SIGINT, b);
  }
  shmdt(waiting);
  shmdt(room);
  shmctl(shared, IPC_RMID, 0);
  shmctl(common, IPC_RMID, 0);
  semctl(sem_p, 0, IPC_RMID);
  semctl(sem_room, 0, IPC_RMID);
  for(int i=0; i<N; i++) semctl(sems, i, IPC_RMID);
  exit(EXIT_SUCCESS);
}

int main(int argc, char const *argv[]) {
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGINT);
  struct sigaction newaction;
  newaction.sa_handler = g;
  sigaction(SIGINT, &newaction, NULL);

  if(argc < 2 || !isNumber(argv[1])) usage(argv[0]);
  N = atoi(argv[1]);
  struct sembuf op[2];
  struct sembuf invite;
  struct sembuf wait[2];
  struct timespec _time;
  int val = 1;
  common = shmget(ftok(getenv("HOME"), COMMON_S), sizeof(struct waiting_room), IPC_CREAT | 0666);
  shared = shmget(ftok(getenv("HOME"), COMMON_S2), N * sizeof(int), IPC_CREAT | 0666);
  sem_p = semget(ftok(getenv("HOME"), SEM_PRIMARY), 1, IPC_CREAT | 0666);
  sems = semget(ftok(getenv("HOME"), SEMS), N, IPC_CREAT | 0666);
  sem_room = semget(ftok(getenv("HOME"), SEM_ROOM), 1, IPC_CREAT | 0666);
  semctl(sem_p, 0, SETVAL, 0);
  for(int i=0; i<N; i++) semctl(sems, i, SETVAL, 0);
  semctl(sem_room, 0, SETVAL, val);
  room = shmat(common, NULL, 0);
  waiting = shmat(shared, NULL, 0);
  room->sleeping = 1;
  room->count = N;
  room->taken = 0;
  room->first = 0;
  room->seat = -1;
  for(int i=0; i<N; i++) waiting[i] = -1;
  wait[0].sem_flg = wait[1].sem_flg = invite.sem_flg = op[0].sem_flg = op[1].sem_flg = 0;
  wait[0].sem_num = wait[1].sem_num = op[0].sem_num = op[1].sem_num = 0;
  wait[1].sem_op = -1;
  invite.sem_op = wait[0].sem_op = 1;
  op[0].sem_op = 1;
  op[1].sem_op = 0;
  clock_gettime(CLOCK_MONOTONIC, &_time);
  printf("%f\tBarber is sleeping\n", _time.tv_sec + (double) _time.tv_nsec / 1e9);
  while (1)
  {
    semop(sem_p, op, 1);
    semop(sem_p, &op[1], 1);

    room->sleeping = 0;
    semop(sem_p, op, 1);
    semop(sem_p, &op[1], 1);

    semop(sem_p, op, 1);
    semop(sem_p, &op[1], 1);

    clock_gettime(CLOCK_MONOTONIC, &_time);
    printf("%f\tBarber is starting handling %d\n", _time.tv_sec + (double) _time.tv_nsec / 1e9, room->seat);
  //  sleep(1);
    clock_gettime(CLOCK_MONOTONIC, &_time);
    printf("%f\tBarber is ending handling %d\n", _time.tv_sec + (double) _time.tv_nsec / 1e9, room->seat);
    semop(sem_p, op, 1);
    semop(sem_p, &op[1], 1);

    semop(sem_room, &wait[1], 1);
    semop(sem_p, op, 1);
    semop(sem_p, &op[1], 1);

    // f = 1;
    //semop(sem_room, &wait[1], 1);
    while(room->taken > 0) //golenie tych z poczekalni
    {
      // if(!f) semop(sem_room, &wait[1], 1);
      // else f = 0;
      // while(waiting[room->first] == -1)
      // {
      //   if(room->taken == 0)
      //   {
      //     f = 1;
      //     break;
      //   }
      //   room->first = (room->first+1)%N;
      //   room->taken--;
      // }
      // if(f)
      // {
      //   break;
      // }
      invite.sem_num = room->first;
      semop(sem_room, &wait[0], 1);
      semop(sems, &invite, 1);
      semop(sem_room, &wait[1], 1);
      room->taken--;
      room->first = (room->first + 1)%N;
      semop(sem_room, &wait[0], 1);
      semop(sem_p, op, 1);
      semop(sem_p, &op[1], 1);
      clock_gettime(CLOCK_MONOTONIC, &_time);
      printf("%f\tBarber is starting handling %d\n", _time.tv_sec + (double) _time.tv_nsec / 1e9, room->seat);
    //  sleep(1);
      clock_gettime(CLOCK_MONOTONIC, &_time);
      printf("%f\tBarber is ending handling %d\n", _time.tv_sec + (double) _time.tv_nsec / 1e9, room->seat);
      semop(sem_p, op, 1);
      semop(sem_p, &op[1], 1);

      semop(sem_room, &wait[1], 1);
      semop(sem_p, op, 1);
      semop(sem_p, &op[1], 1);

    }
    room->sleeping = 1;
    semop(sem_room, &wait[0], 1);
    clock_gettime(CLOCK_MONOTONIC, &_time);
    printf("%f\tBarber is sleeping\n", _time.tv_sec + (double) _time.tv_nsec / 1e9);
  }
  raise(SIGINT);
  return 0;
}
