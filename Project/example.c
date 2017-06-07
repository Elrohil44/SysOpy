#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <stdio.h>

int b;

void* f(void* a)
{
  b = 15;
  return NULL;
}

int main(int argc, char const *argv[]) {
  printf("xD\n");
  printf("%d\n", argc);
  char* c = malloc(3 * 1024 * 1024);
  if (c == NULL) printf("Out of memory, malloc failed\n");
  char a[2];
  fflush(stdout);
  fprintf(stdout, "%s\n", "fprintf");
  pthread_t t;
  scanf(" %d\n", &b);
  //pthread_create(&t, NULL, f, NULL);
  for(int i = 0; i<1e9; i++, b++);
  printf("%d\n", b);
  printf("%d [%d]\n", 12, 1);
  printf("%d [%d]\n", b, 1);
  exit(EXIT_FAILURE);
  return 0;
}
