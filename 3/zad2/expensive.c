#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ALLOCK_SIZE 5000000
#define NUMBER_OF_ALLOCKS 20

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
  char* x[NUMBER_OF_ALLOCKS];
  switch(atoi(argv[1]))
  {
    case 1:
      while(1);
      break;
    case 2:
      for(int i=0;i<NUMBER_OF_ALLOCKS;i++)
      {
        if((x[i] = malloc(ALLOCK_SIZE))!=NULL)
          printf("%d bytes have been already allocated\n",(i+1)*ALLOCK_SIZE);
        else
        {
          printf("Error while allocating memory\n");
          exit(EXIT_FAILURE);
        }
      }
      for(int i=0;i<NUMBER_OF_ALLOCKS && x[i]!=NULL;i++) free(x[i]);
      break;
    default:
    break;
  }
  return 0;
}
