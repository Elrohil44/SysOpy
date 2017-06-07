#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int b;


int main(int argc, char const *argv[]) {
  printf("xD\n");
  printf("%d\n", argc);
  char* c = malloc(3 * 1024 * 1024);
  if (c == NULL) printf("Out of memory, malloc failed\n");
  char a[2];
  fflush(stdout);
  fprintf(stdout, "%s\n", "fprintf");
  scanf(" %d\n", &b);
  for(int i = 0; i<1e9; i++, b++);
  printf("%d\n", b);
  printf("%d [%d]\n", 12, 1);
  printf("%d [%d]\n", b, 1);
  exit(EXIT_FAILURE);
  return 0;
}
