#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/socket.h>

void* g(void* u)
{
  for(int i=0; i<5; i++)
   printf("%d ", i);
  printf("\n");
  return NULL;
}

int main(int argc, char **argv) {
  int fd = open("testfile", O_CREAT, 660);
  // int fd = open("testfile", O_RDONLY);
  // while(1);
  char* mem_test = NULL;
  int i;
  for(i=1024; (mem_test = malloc(i)) != NULL && i<1024*1024*150; i+=1024)
  {
    free(mem_test);
  }
  if((mem_test = malloc(i-1024)))
    printf("%lf MB allocated\n", (double)(i - 1024)/1024/1024);
  printf("Creating socket...\n");
  getchar();
  int IP_socket = socket(AF_INET, SOCK_STREAM, 0);
  printf("Socket created\n");
  printf("Creating thread...\n");
  getchar();
  pthread_t thread;
  pthread_create(&thread, NULL, g, NULL);
  pthread_join(thread, NULL);
  printf("Thread finished\n");
  printf("Forking...\n");
  getchar();
  int x = fork();
  if(x==0)
  {
    printf("In child process\n");
    exit(EXIT_SUCCESS);
  }
  printf("In parent process\nForked\n");
  if(mem_test != NULL) free(mem_test);
  getchar();
  return 0;
}
