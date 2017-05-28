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
#include <netdb.h>
#include "common.h"

struct client clients[MAX_CLIENTS];
int clients_count = 0;
pthread_cond_t count_changed;
pthread_mutex_t count_mutex;
int UNIX_socket, IP_socket;
int epoll_fd;
const char* unix_path;
int counter = 0;

int name_taken(const char* name);
void monitor();
void connector_handler(int fd);
void main_menu();
void sighandler(int n)
{
  exit(EXIT_FAILURE);
}

void onexit(void)
{
  pthread_mutex_lock(&count_mutex);
  shutdown(UNIX_socket, SHUT_RDWR);
  shutdown(IP_socket, SHUT_RDWR);
  for(--clients_count; clients_count>=0; clients_count--)
  {
    shutdown(clients[clients_count].descriptor, SHUT_RDWR);
    close(clients[clients_count].descriptor);
  }
  close(UNIX_socket);
  close(IP_socket);
  close(epoll_fd);
  pthread_cond_destroy(&count_changed);
  pthread_mutex_unlock(&count_mutex);
  pthread_mutex_destroy(&count_mutex);
  unlink(unix_path);
}


void* pinger()
{
  struct message msg;
  msg.type = htonl(PING);

  while(1)
  {
    for(int i=0; i<clients_count; i++)
    {
      pthread_mutex_lock(&count_mutex);

      if(send(clients[i].descriptor, &msg, sizeof(msg), MSG_NOSIGNAL) == -1)
      {
        clients[i] = clients[--clients_count];
      }
      pthread_cond_broadcast(&count_changed);
      pthread_mutex_unlock(&count_mutex);
    }
    sleep(5);
  }
}

void* connector_IP_acceptor()
{
  if(listen(IP_socket, MAX_CLIENTS))
    perror("ERROR");
  while(1)
  {
    monitor();

    connector_handler(IP_socket);

    pthread_cond_broadcast(&count_changed);
    pthread_mutex_unlock(&count_mutex);
  }
}

void* connector_UNIX_acceptor()
{
  if(listen(UNIX_socket, MAX_CLIENTS))
    perror("Error");
  while(1)
  {
    monitor();

    connector_handler(UNIX_socket);

    pthread_cond_broadcast(&count_changed);
    pthread_mutex_unlock(&count_mutex);
  }
}

void* interface()
{
  char task;
  char buff[64];
  struct message msg;
  int f = 1;
  int cond = 1;
  int clientid;
  srand(time(NULL));
  while(cond)
  {
    main_menu();
    scanf(" %c", &task);
    fgets(buff, 64, stdin);
    f = 1;
    switch(task)
    {
      case '1':
        msg.type = htonl(ADD);
        break;
      case '2':
        msg.type = htonl(SUBTRACT);
        break;
      case '4':
        msg.type = htonl(MULTIPLY);
        break;
      case '3':
        msg.type = htonl(DIVIDE);
        break;
      case 'q':
        cond = 0;
      default:
        f = 0;
        break;
    }
    if(f)
    {
      if(clients_count == 0)
      {
        printf("\tNo clients available\n\n");
        continue;
      }
      printf("\tArg1:\t");
      scanf(" %d", &msg.arg1);
      fgets(buff, 64, stdin);
      printf("\tArg2:\t");
      scanf(" %d", &msg.arg2);
      fgets(buff, 64, stdin);
      msg.counter = htonl(counter++);
      msg.arg1 = htonl(msg.arg1);
      msg.arg2 = htonl(msg.arg2);
      if(write(clients[(clientid = rand()%clients_count)].descriptor, &msg, sizeof(msg)) == -1)
      {
        printf("Problem while sending message to %s", clients[clientid].name);
        counter--;
      }
    }
  }
  return NULL;
}

void* msg_receiver()
{
  struct epoll_event event;
  struct response response;
  while(1)
  {
    if(epoll_wait(epoll_fd, &event, 1, -1) != -1)
    {
      if(recv(event.data.fd, &response, sizeof(response), MSG_WAITALL) > 0)
      {
        response.counter = ntohl(response.counter);
        response.result = ntohl(response.result);
        printf("\n\n%d. result returned by %s: %d\n\n", response.counter, response.name, response.result);
        main_menu();
      }
    }
  }
}

int isNumber(const char* arg)
{
  for(int i=0;i<strlen(arg);i++) if(arg[i]<'0' || arg[i]>'9') return 0;
  return 1;
}

void printusage(const char * name)
{
  printf("Usage:\t %s PORT SOCKET_PATH\n\n", name);
  printf("\tPORT - will listen that port\n");
  printf("\tSOCKET_PATH - path to UNIX socket\n");
  exit(0);
}

int main(int argc, char const *argv[]) {
  if(argc<3 || !isNumber(argv[1])) printusage(argv[0]);
  int port = atoi(argv[1]);
  unix_path = argv[2];

  UNIX_socket = socket(AF_UNIX, SOCK_STREAM, 0);
  IP_socket = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in IP_addr;
  IP_addr.sin_family = AF_INET;
  IP_addr.sin_port = htons(port);
  IP_addr.sin_addr.s_addr = INADDR_ANY;

  struct sockaddr_un UNIX_addr;
  UNIX_addr.sun_family = AF_UNIX;
  strcpy(UNIX_addr.sun_path, argv[2]);

  if(bind(IP_socket, (struct sockaddr *)&IP_addr, sizeof(IP_addr)))
  {
    perror("Error binding");
    exit(EXIT_FAILURE);
  }
  if(bind(UNIX_socket, (struct sockaddr *)&UNIX_addr, sizeof(UNIX_addr)))
  {
    perror("Error binding");
    exit(EXIT_FAILURE);
  }
  char hostname[250];
  gethostname(hostname, 250);
  struct hostent*  info= gethostbyname(hostname);
  printf("Server started:\n");
  printf("IP: %s, PORT: %d\n", inet_ntoa(*(struct in_addr*)(info->h_addr_list)), port);
  realpath(argv[2], hostname);
  printf("UNIX: %s\n", hostname);
  epoll_fd = epoll_create1(0);

  pthread_mutex_init(&count_mutex, NULL);
  pthread_cond_init(&count_changed, NULL);
  atexit(onexit);
  signal(SIGINT, sighandler);
  pthread_t acceptor_IP, acceptor_UNIX, pingerino, inter, receivor;
  pthread_create(&acceptor_IP, NULL, &connector_IP_acceptor, NULL);
  pthread_create(&acceptor_UNIX, NULL, &connector_UNIX_acceptor, NULL);
  pthread_create(&pingerino, NULL, &pinger, NULL);
  pthread_create(&receivor, NULL, &msg_receiver, NULL);
  pthread_create(&inter, NULL, &interface, NULL);
  pthread_detach(acceptor_IP);
  pthread_detach(acceptor_UNIX);
  pthread_detach(pingerino);
  pthread_detach(receivor);
  pthread_join(inter, NULL);
  return 0;
}


void main_menu()
{
  printf("\tADD\t\t-\t1\n");
  printf("\tSUBTRACT\t-\t2\n");
  printf("\tDIVIDE\t\t-\t3\n");
  printf("\tMULTIPLY\t-\t4\n");
  printf("\tQUIT\t\t-\tq\n");
  printf("\n\n\tChoose task:\t");
  fflush(stdout);
}

int name_taken(const char* name)
{
  for(int i=0; i<clients_count; i++)
  {
    if(!strcmp(name, clients[i].name)) return 1;
  }
  return 0;
}

void monitor()
{
  pthread_mutex_lock(&count_mutex);
  while(clients_count >= MAX_CLIENTS)
    pthread_cond_wait(&count_changed, &count_mutex);
  pthread_mutex_unlock(&count_mutex);
}

void connector_handler(int fd)
{
  struct message msg;
  epoll_data_t fd_new;
  struct epoll_event event;
  event.events = EPOLLIN | EPOLLET;

  int descriptor = accept(fd, NULL, NULL);
  pthread_mutex_lock(&count_mutex);
  recv(descriptor, &clients[clients_count], sizeof(clients[clients_count]), MSG_WAITALL);
  if(name_taken(clients[clients_count].name))
  {
    msg.type = htonl(NAME_TAKEN);
    write(descriptor, &msg, sizeof(msg));
    shutdown(descriptor, SHUT_RDWR);
    close(descriptor);
  }
  else
  {
    msg.type = htonl(REG_SUCC);
    clients[clients_count++].descriptor = descriptor;
    fd_new.fd = descriptor;
    event.data = fd_new;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, descriptor, &event);
    write(descriptor, &msg, sizeof(msg));
  }
}
