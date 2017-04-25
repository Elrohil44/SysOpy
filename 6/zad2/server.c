#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include "common.h"



mqd_t clients[MAX_CLIENTS_COUNT];
mqd_t queue;

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
  for (int i=0; i<MAX_CLIENTS_COUNT; i++) if(clients[i]!=-1) mq_close(clients[i]);
  mq_close(queue);
  mq_unlink(SERVER_NAME);
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
  int count = 0;
  for(int i=0; i<MAX_CLIENTS_COUNT; i++) clients[i] = -1;
  struct mq_attr attr;
  attr.mq_maxmsg = 10;
  attr.mq_msgsize = MSGLEN;
  attr.mq_flags = 0;
  char name[10];
  queue = mq_open(SERVER_NAME, O_RDONLY | O_CREAT, 666, &attr);
  while(mq_receive(queue, (char*)&msg, MSGLEN, NULL) != -1)
  {
    switch (msg.mtype) {
      case START:
        sscanf(msg.msg, "%s", name);
        clients[count] = mq_open(name, O_WRONLY);
        msg.key = count;
        count++;
        mq_send(clients[msg.key], (char*)&msg, MSGLEN, 0);
        break;
      case ECHO:
        mq_send(clients[msg.key], (char*)&msg, MSGLEN, 0);
        break;
      case UPCASE:
        uppercase(msg.msg);
        mq_send(clients[msg.key], (char*)&msg, MSGLEN, 0);
        break;
      case TIME:
        time(&time_sec);
        time_a = localtime(&time_sec);
        sprintf(msg.msg, "%s", asctime(time_a));
        mq_send(clients[msg.key], (char*)&msg, MSGLEN, 0);
        break;
      case EXIT:
        mq_close(clients[msg.key]);
        clients[msg.key]=-1;
        attr.mq_flags = O_NONBLOCK;
        mq_setattr(queue, &attr, NULL);
    }
  }
  for (int i=0; i<MAX_CLIENTS_COUNT; i++) if(clients[i]!=-1) mq_close(clients[i]);
  mq_close(queue);
  mq_unlink(SERVER_NAME);
  return 0;
}
