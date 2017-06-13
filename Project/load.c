#include "load.h"

int main(int argc, char** argv)
{
  if(argc < 4 || !(isNumber(argv[1]) && isNumber(argv[2])))
    printusage(argv[0], EXIT_FAILURE);
  int MEM_LIMIT = atoi(argv[1]) * 1024 * 1024;
  int TIMEOUT = atoi(argv[2]);
  pid_t ptraced;

  if((ptraced = tracee(&argv[3], MEM_LIMIT)) < 0)
  {
    exit(EXIT_FAILURE);
  }

  trace(ptraced, TIMEOUT);

  return 0;
}
