#ifndef COMMON_H
#define COMMON_H

#define ECHO 1
#define UPCASE 2
#define TIME 3
#define EXIT 4
#define START 5

#define SERVER_KEY 1
#define MINKEY 2
#define MAXKEY 255

#define MAX_CLIENTS_COUNT 254

#define MSGLEN 1024
#define BUFSIZE MSGLEN - sizeof(int)

struct message
{
  long mtype;
  int key;
  char msg[BUFSIZE];
};



#endif
