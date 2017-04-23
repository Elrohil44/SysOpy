#include <stdlib.h>
#include <stdio.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include "common.h"
#include <string.h>

void menu()
{
  printf("\tECHO\t\t-\t1\n");
  printf("\tUPPERCASE\t-\t2\n");
  printf("\tTIME\t\t-\t3\n");
  printf("\tQUIT SERVER\t-\t4\n");
  printf("\n\n\tChoose task:\t");
}

void rand_str(char *dest, size_t length) {
    int seed;
    int fd = open("/dev/urandom", O_RDONLY);
    read(fd, &seed, sizeof(int));
    close(fd);
    srand(seed);
    char charset[] = "0123456789"
                     "abcdefghijklmnopqrstuvwxyz"
                     "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    while (length-- > 0) {
        size_t index = (double) rand() / RAND_MAX * (sizeof charset - 1);
        *dest++ = charset[index];
    }
    *dest = '\0';
}


int main(int argc, char const *argv[]) {
  mqd_t id, queue;
  int task;
  int f = 1;
  struct message msg;
  struct message response;
  char name[10] = "/";
  rand_str(&name[1], 9);
  struct mq_attr attr;
  attr.mq_maxmsg = 10;
  attr.mq_msgsize = MSGLEN;
  while((id = mq_open(name, O_RDONLY | O_CREAT | O_EXCL, 0666, &attr)) == -1)
    rand_str(&name[1], 9);
  if((queue = mq_open(SERVER_NAME, O_WRONLY)) == -1) exit(EXIT_FAILURE);
  msg.mtype = START;
  strcpy(msg.msg, name);
  mq_send(queue, (char*)&msg, MSGLEN, 0);
  mq_receive(id, (char*)&response, MSGLEN, NULL);
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
    mq_send(queue, (char*)&msg, MSGLEN, 0);
    if(f)
    {
      if(mq_receive(id, (char*)&response, MSGLEN, NULL) == -1) break;
      printf("\n\tResponse: %s\n", response.msg);
    }
    else break;
  }
  mq_close(id);
  mq_unlink(name);
  return 0;
}
