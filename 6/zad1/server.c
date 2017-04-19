#include <sys/msg.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include "common.h"

char* uppercase(char* s)
{
  int i = 0;
  while(s[i])
  {
    s[i] = toupper(s[i++]);
  }
  return s;
}

int main(int argc, char const *argv[]) {
  int clients[MAX_CLIENTS_COUNT];
  struct message msg;
  struct tm time_a;
  struct msqid_ds details;
  struct msg* last;
  struct msg* first;
  int f = MSG_NOERROR;
  int count = 0;
  for(int i=0; i<MAX_CLIENTS_COUNT; i++) clients[i] = -1;
  int queue = msgget(ftok(getenv("HOME"), SERVER_KEY), IPC_CREAT);
  while(msgrcv(queue, &msg, MSGLEN, 0, f))
  {
    switch (msg.mtype) {
      case START:
        clients[count] = msgget(msg.key, 0);
        msg.key = count;
        msgsnd(clients[count++], &msg, MSGLEN, 0);
        break;
      case ECHO:
        msgsnd(clients[msg.key], &msg, MSGLEN, 0);
        break;
      case UPCASE:
        msg.msg = uppercase(msg.msg);
        msgsnd(clients[msg.key], &msg, MSGLEN, 0);
        break;
      case TIME:
        time_a = localtime(time(NULL));
        sprintf(msg.msg, "%d:%d:%d\t%d.%d.%d", time_a.tm_hour, time_a.tm_min,
                                               time_a.tm_sec, time_a.tm_mday,
                                               time_a.tm_mon, time_a.tm_year + 1900);
         msgsnd(clients[msg.key], &msg, MSGLEN, 0);
         break;
      case EXIT:
        f = MSG_NOERROR | IPC_NOWAIT;
    }
  }
  return 0;
}
