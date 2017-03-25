#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>

void printusage(const char * name)
{
  printf("Usage:\t %s Name:\n\n", name);
  printf("\tName - name of an environmental variable\n");
  exit(0);
}

int main(int argc, char const *argv[]) {
  if (argc !=2) printusage(argv[0]);
  const char* x = secure_getenv(argv[1]);
  if(x !=NULL) printf("%s\n", x);
  else printf("There is no \"%s\" variable in the environment\n", argv[1]);
  return 0;
}
