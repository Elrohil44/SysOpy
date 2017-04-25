#include <sys/msg.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <ctype.h>
#include "signal.h"
#include "common.h"


int clients[MAX_CLIENTS_COUNT];
int queue;

void uppercase(char* s)
{
  int i = 0;
  while(i < BUFSIZE && s[i])
  {
    s[i] = toupper(s[i]);
    i++;
  }
}

void g(int signo)
{
  msgctl(queue, IPC_RMID, NULL);
  int i = 0;
  while(clients[i]!=-1)
  {
    msgctl(clients[i++], IPC_RMID, NULL);
  }
  exit(EXIT_FAILURE);
}

int main(int argc, char const *argv[]) {
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGINT);
  struct sigaction newaction;
  newaction.sa_handler = g;
  sigaction(SIGINT, &newaction, NULL);
  struct message msg;
  struct tm* time_a;
  time_t time_sec;
  int f = MSG_NOERROR;
  int count = 0;
  for(int i=0; i<MAX_CLIENTS_COUNT; i++) clients[i] = -1;
  queue = msgget(ftok(getenv("HOME"), SERVER_KEY), IPC_CREAT | 0666);
  while(msgrcv(queue, &msg, MSGLEN, 0, f) != -1)
  {
    switch (msg.mtype) {
      case START:
        clients[count] = msg.key;
        msg.key = count;
        count++;
        msgsnd(clients[msg.key], &msg, MSGLEN, 0);
        break;
      case ECHO:
        msgsnd(clients[msg.key], &msg, MSGLEN, 0);
        break;
      case UPCASE:
        uppercase(msg.msg);
        msgsnd(clients[msg.key], &msg, MSGLEN, 0);
        break;
      case TIME:
        time(&time_sec);
        time_a = localtime(&time_sec);
        sprintf(msg.msg, "%s", asctime(time_a));
        msgsnd(clients[msg.key], &msg, MSGLEN, 0);
        break;
      case EXIT:
        f = MSG_NOERROR | IPC_NOWAIT;
    }
  }
  msgctl(queue, IPC_RMID, NULL);
  int i = 0;
  while(clients[i]!=-1)
  {
    msgctl(clients[i++], IPC_RMID, NULL);
  }
  return 0;
}
