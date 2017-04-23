#ifndef COMMON_H
#define COMMON_H

#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define ECHO 1
#define UPCASE 2
#define TIME 3
#define EXIT 4
#define START 5

#define MAX_CLIENTS_COUNT 254

#define MSGLEN 1024
#define BUFSIZE MSGLEN - sizeof(int) - sizeof(mqd_t)

const char SERVER_NAME[] = "/SERVFORTHEWIN";

struct message
{
  int mtype;
  mqd_t key;
  char msg[BUFSIZE];
};



#endif
