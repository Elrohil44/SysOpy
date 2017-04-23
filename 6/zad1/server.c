#include <sys/msg.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <ctype.h>
#include "common.h"

void uppercase(char* s)
{
  int i = 0;
  while(i < BUFSIZE && s[i])
  {
    s[i] = toupper(s[i]);
    i++;
  }
}

int main(int argc, char const *argv[]) {
  int clients[MAX_CLIENTS_COUNT];
  struct message msg;
  struct tm time_a;
  time_t time_sec;
  int f = MSG_NOERROR;
  int count = 0;
  for(int i=0; i<MAX_CLIENTS_COUNT; i++) clients[i] = -1;
  int queue = msgget(ftok(getenv("HOME"), SERVER_KEY), IPC_CREAT | 0666);
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
        time_a = *(localtime(&time_sec));
        sprintf(msg.msg, "%d:%d:%d\t%d.%d.%d", time_a.tm_hour, time_a.tm_min,
                                               time_a.tm_sec, time_a.tm_mday,
                                               time_a.tm_mon + 1, time_a.tm_year + 1900);
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
