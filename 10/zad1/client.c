#define _BSD_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <signal.h>
#include <unistd.h>
#include "common.h"


const char* name;
const char* path;
int port;
int socket_fd;
int xflag = 0;

void sighandler(int n)
{
  exit(EXIT_FAILURE);
}

void onexit(void)
{
  shutdown(socket_fd, SHUT_RD);
  close(socket_fd);
}

int isNumber(const char* arg)
{
  for(int i=0;i<strlen(arg);i++) if(arg[i]<'0' || arg[i]>'9') return 0;
  return 1;
}

void printusage(const char * name)
{
  printf("Usage:\t %s -n NAME [-4 IP -p PORT | -x SOCKET_PATH]\n\n", name);
  printf("\tPORT - sever's port\n");
  printf("\tIP - sever's IP\n");
  printf("\tNAME - client's name\n");
  printf("\tSOCKET_PATH - path to server's UNIX socket\n");
  exit(0);
}

void parseargs(int argc, char** argv)
{
  if(argc<5) printusage(argv[0]);
  int c;
  int nflag = 0, ip_flag = 0, pflag = 0;
  while ((c = getopt (argc, argv, "n:x:4:p:")) != -1)
    switch (c)
      {
        case 'n':
          nflag = 1;
          name = optarg;
          break;
        case 'x':
          xflag = 1;
          path = optarg;
          break;
        case 'p':
          pflag = 1;
          if(isNumber(optarg)) port = htons(atoi(optarg));
          else printusage(argv[0]);
          break;
        case '4':
          ip_flag = 1;
          name = optarg;
          printf("%s\n", name);
          break;
        case '?':
          if (optopt == 'n' || optopt == 'm')
            {
              fprintf (stderr, "Option -%c requires an argument.\n", optopt);
              printusage(argv[0]);
            }
          else
            {
              fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
              printusage(argv[0]);
            }
          exit(EXIT_FAILURE);
        default:
          abort ();
      }
  printf("pflag = %d, ip_flag = %d, x_flag = %d, nflag = %d\n", pflag, ip_flag, xflag, nflag);
  if((pflag ^ ip_flag) || (ip_flag && xflag) || !nflag) printusage(argv[0]);

  for (int index = optind; index < argc; index++)
  {
    printf ("Non-option argument %s\n", argv[index]);
    printusage(argv[0]);
  }

}



int main(int argc, char **argv) {
  parseargs(argc, argv);
  socket_fd = xflag ? socket(AF_UNIX, SOCK_STREAM, 0) : socket(AF_INET, SOCK_STREAM, 0);
  struct message msg;
  struct client client;
  struct response response;
  strcpy(response.name, name);
  if(xflag)
  {
    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, path);
    if(connect(socket_fd, (struct sockaddr *)&addr, sizeof(addr)))
    {
      perror("Error");
      exit(EXIT_FAILURE);
    }
  }
  else
  {
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    if(!inet_aton(name, &addr.sin_addr))
    {
      perror("Error, wrong IP");
      exit(EXIT_FAILURE);
    }
    addr.sin_port = port;
    if(connect(socket_fd, (struct sockaddr *)&addr, sizeof(addr)))
    {
      perror("Error");
      exit(EXIT_FAILURE);
    }
  }
  strcpy(client.name, name);
  atexit(onexit);
  signal(SIGINT, sighandler);
  write(socket_fd, &client, sizeof(client));
  recv(socket_fd, &msg, sizeof(msg), MSG_WAITALL);
  msg.type = ntohl(msg.type);
  if(msg.type == NAME_TAKEN)
  {
    printf("Name %s is already taken\n", name);
    exit(EXIT_FAILURE);
  }
  else
  {
    printf("%s succesfully registered\n", name);
  }
  int received;
  while(1)
  {
    received = recv(socket_fd, &msg, sizeof(msg), MSG_WAITALL);
    if(received == 0)
    {
      perror("Server closed connection");
      exit(EXIT_SUCCESS);
    }
    if(received == -1) continue;
    response.counter = msg.counter;
    printf("Type: %d, Arg1: %d, Arg2: %d\n", ntohl(msg.type), ntohl(msg.arg1), ntohl(msg.arg2));
    switch (ntohl(msg.type)) {
      case ADD:
        response.result = htonl(ntohl(msg.arg1) + ntohl(msg.arg2));
        break;
      case MULTIPLY:
        response.result = htonl(ntohl(msg.arg1) * ntohl(msg.arg2));
        break;
      case SUBTRACT:
        response.result = htonl(ntohl(msg.arg1) - ntohl(msg.arg2));
        break;
      case DIVIDE:
        response.result = htonl(ntohl(msg.arg1) / ntohl(msg.arg2));
        break;
      default:
        printf("Received some message\n");
        continue;
        break;
    }
    printf("%s received %d order and the result is: %d\n", name,
      ntohl(response.counter), (int)
      ntohl(response.result));
    write(socket_fd, &response, sizeof(response));
  }
  return 0;
}
