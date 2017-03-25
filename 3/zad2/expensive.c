#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void usage(const char* program)
{
  printf("Usage: %s type\n",program);
  printf("\ttype - 1 -> infinite loop\n");
  printf("\t\t2 -> large alloc of memory\n");
}

int isNumber(const char* arg)
{
  for(int i=0;i<strlen(arg);i++) if(arg[i]<'0' || arg[i]>'9') return 0;
  return 1;
}

int main(int argc, char const *argv[]) {
  if(argc != 2 || !isNumber(argv[1])) usage(argv[0]);
  char* x;
  switch(atoi(argv[1]))
  {
    case 1:
      while(1);
      break;
    case 2:
      x = malloc(1000000);
      x[100]='u';
      free(x);
    default:
    break;
  }
  return 0;
}
