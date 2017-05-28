#ifndef COMMON_H
#define COMMON_H

#define MAX_CLIENTS 128
#define NAME_SIZE 64

#define ADD 3247
#define SUBTRACT 1298
#define DIVIDE 3610
#define MULTIPLY 9248

#define PING 7345

#define REG_SUCC 4167
#define NAME_TAKEN 8943

struct client
{
  char name[NAME_SIZE];
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
