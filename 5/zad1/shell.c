#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <limits.h>

#define MAX_ARGS 500
#define MAX_PROGRAMS 100
// #define MAX_LENGTH 1000

void printexec(char* const* args);

// void printusage(const char * name)
// {
//   printf("Usage:\t %s Batch_file:\n\n", name);
//   printf("\tBatch_file - path to file containing a batch script\n");
//   exit(0);
// }

int parseline(char* line, char** programs)
{
  int i=0;
  if((args[0]=strtok(line, "|\n"))!=NULL)
    while (i<MAX_PROGRAMS-2 && (args[++i]=strtok(NULL, "|\n"))!=NULL);
  programs[i]=NULL;
  return 1;
}

int parseargs(char* program, char** args)
{
  int i=0;
  if((args[0]=strtok(line, " \t\r\n"))!=NULL)
    while (i<MAX_ARGS-2 && (args[++i]=strtok(NULL, " \t\r\n"))!=NULL);
  programs[i]=NULL;
  return 1;
}

// void printexec(char* const* args)
// {
//   if(args==NULL) return;
//   for(int i=0;args[i]!=NULL;i++) printf("%s ", args[i]);
//   printf("\n");
// }

int main(int argc, char const *argv[]) {
  char* args[MAX_ARGS];
  char* programs[MAX_PROGRAMS];
  int pipes[MAX_PROGRAMS][2];
  char* line=NULL;
  size_t n=0;
  pid_t pid,w;
  int programsc, argsc;
  int status;
  while((getline(&line,&n,stdin))!=-1)
  {
    programsc = parseline(line, programs);
    for(int i=0; i<programsc; i++)
    {
      pipe(pipes[i]);
      if(parseargs(programs[i], args))
      {
        
      };

    }


    printf(">>> ");
    fflush(stdout);
  }
  free(line);
  return 0;
}
