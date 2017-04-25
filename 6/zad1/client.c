#include <stdlib.h>
#include <stdio.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <signal.h>
#include "common.h"

int id;

void menu()
{
  printf("\tECHO\t\t-\t1\n");
  printf("\tUPPERCASE\t-\t2\n");
  printf("\tTIME\t\t-\t3\n");
  printf("\tQUIT SERVER\t-\t4\n");
  printf("\n\n\tChoose task:\t");
}

void g(int signo)
{
  msgctl(id, IPC_RMID, NULL);
  exit(EXIT_FAILURE);
}

int main(int argc, char const *argv[]) {
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGINT);
  struct sigaction newaction;
  newaction.sa_handler = g;
  sigaction(SIGINT, &newaction, NULL);
  int queue;
  int task;
  int f = 1;
  struct message msg;
  struct message response;
  for(int i = MINKEY; i<= MAXKEY && (id = msgget(ftok(getenv("HOME"), i), IPC_CREAT | IPC_EXCL | 0666)) == -1; i++);
  if(id == -1) exit(EXIT_FAILURE);
  if((queue = msgget(ftok(getenv("HOME"), SERVER_KEY), 0)) == -1) exit(EXIT_FAILURE);
  msg.mtype = START;
  msg.key = id;
  msgsnd(queue, &msg, MSGLEN, 0);
  msgrcv(id, &response, MSGLEN, 0, MSG_NOERROR);
  msg.key = response.key;
  while(f)
  {
    menu();
    scanf(" %d", &task);
    fgets(msg.msg, BUFSIZE, stdin);
    switch(task)
    {
        case ECHO:
          msg.mtype = ECHO;
          printf("\tText to send:\t ");
          fgets(msg.msg, BUFSIZE, stdin);
          break;
        case UPCASE:
          msg.mtype = UPCASE;
          printf("\tText to send:\t");
          fgets(msg.msg, BUFSIZE, stdin);
          break;
        case TIME:
          msg.mtype = TIME;
          break;
        case EXIT:
          msg.mtype = EXIT;
          f = 0;
          break;
        default:
          printf("UNSUPPORTED OPERATION\n");
          continue;
    }
    msgsnd(queue, &msg, MSGLEN, 0);
    if(f)
    {
      if(msgrcv(id, &response, MSGLEN, 0, MSG_NOERROR) == -1) break;
      printf("\n\tResponse: %s\n", response.msg);
    }
    else break;
  }
  msgctl(id, IPC_RMID, NULL);
  return 0;
}
