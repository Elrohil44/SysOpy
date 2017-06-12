#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>

int b;

void f(int signo)
{
  printf("SIGSEGV\n");
}

void* g(void* g)
{
  printf("xD\n");
  return NULL;
}

int main(int argc, char const *argv[]) {
  signal(SIGSEGV, f);
  printf("xD\n");
  printf("%d\n", argc);
  char o[3*1024*1024];
  char* c = malloc(3 * 1024 * 1024);
  if (c == NULL) printf("Out of memory, malloc failed\n");
  char a[2];
  fflush(stdout);
  fprintf(stderr, "%s\n", "fprintf");
  fgets(o, sizeof(o), stdin);
  sscanf(o," %d", &b);
  pthread_t t;
  for(int i = 0; i<1e9; i++, b++);
  printf("%d\n", b);
  printf("%d [%d]\n", 12, 1);
  char iop[3*1024*1024];
  int fd = open("xD.txt", O_RDONLY);
  printf("%d\n", fd);
  printf("%d [%d]\n", b, 1);
  return 0;
}
