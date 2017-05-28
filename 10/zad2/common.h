#ifndef COMMON_H
#define COMMON_H

#include <sys/types.h>
#include <sys/socket.h>

#define MAX_CLIENTS 128
#define NAME_SIZE 64

#define ADD 3247
#define SUBTRACT 1298
#define DIVIDE 3610
#define MULTIPLY 9248

#define REGISTER -2
#define PING 7345

#define CLOSE 1734
#define REG_SUCC 4167
#define NAME_TAKEN 8943

struct client
{
  char name[NAME_SIZE];
  struct sockaddr addr;
  socklen_t len;
  int descriptor;
};

struct message
{
  int type;
  int counter;
  int arg1;
  int arg2;
};

struct response
{
  char name[NAME_SIZE];
  int counter;
  int result;
};

#endif
