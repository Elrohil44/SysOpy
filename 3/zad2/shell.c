#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <limits.h>
#include "bench.h"
#include <sys/time.h>
#include <sys/resource.h>

#define MAX_ARGS 500

void printexec(char* const* args);

void printusage(const char * name)
{
  printf("Usage:\t %s Batch_file CPU_LIMIT MEMORY_LIMIT:\n\n", name);
  printf("\tBatch_file - path to file containing a batch script\n");
  printf("\tCPU_LIMIT - limit in seconds\n");
  printf("\tMEMORY_LIMIT - limit in bytes\n");
  exit(0);
}

int parse(char* line, char** args)
{
  int i=0;
  if((args[0]=strtok(line, " \n\r\t"))!=NULL)
    while (i<MAX_ARGS-2 && (args[++i]=strtok(NULL, " \n\r\t"))!=NULL);
  args[i]=NULL;
  if(i==0) return 3;
  else if(i==1 && args[0][0]=='#') return 1;
  else if (i>1 && args[0][0]=='#') return 2;
  else return 0;
}

void printexec(char* const* args)
{
  if(args==NULL) return;
  for(int i=0;args[i]!=NULL;i++) printf("%s ", args[i]);
  printf("\n");
}

int isNumber(const char* arg)
{
  for(int i=0;i<strlen(arg);i++) if(arg[i]<'0' || arg[i]>'9') return 0;
  return 1;
}

int main(int argc, char const *argv[]) {
  if(argc!=4) printusage(argv[0]);
  if(!isNumber(argv[2]) || !isNumber(argv[3])) printusage(argv[0]);
  FILE* file;
  if((file = fopen(argv[1], "r"))==NULL)
  {
    printf("Error opening file %s\n", argv[1]);
    exit(0);
  }
  char* args[MAX_ARGS];
  char* line=NULL;
  size_t n=0;
  pid_t pid,w;
  int nr=1;
  int status;
  struct rlimit cpu;
  cpu.rlim_cur=cpu.rlim_max=atoi(argv[2]);
  struct rlimit vm;
  vm.rlim_cur=vm.rlim_max=atoi(argv[3]);
  struct rusage time_a,time_l;

  while((getline(&line,&n,file))!=-1)
  {
      switch(parse(line,args))
      {
        case 3:
          break;
        case 1:
          unsetenv(&args[0][1]);
          break;
        case 2:
          setenv(&args[0][1],args[1],1);
          break;
        case 0:
          time_l=getTime();
          if((pid=fork())<0)
          {
            printf("Line:%d:Couldn't create process for: ",nr);
            printexec(args);
            exit(EXIT_FAILURE);
          }
          else if(pid==0)
          {
            setrlimit(RLIMIT_CPU, &cpu);
            setrlimit(RLIMIT_AS, &vm);
            if(execvp(args[0],args)<0)
            {
              printf("Line:%d:Error while executing: ",nr);
              printexec(args);
              exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
          }
          do
          {
            w = waitpid(pid, &status, WUNTRACED | WCONTINUED);
            if (w == -1)
            {
              printf("Line:%d:Error while waiting for process: ",nr);
              printexec(args);
              exit(EXIT_FAILURE);
            }


            if (WIFEXITED(status)) {
                if(WEXITSTATUS(status))
                {
                  printexec(args);
                  time_a = getTime();
                  printf("Line:%d:Process was exited with status: %d\n", nr, WEXITSTATUS(status));
                  printtimes(time_l, time_a);
                  exit(EXIT_FAILURE);
                }
              } else if (WIFSIGNALED(status)) {
                  printexec(args);
                  time_a = getTime();
                  printf("Line:%d:Process was killed by signal %d\n", nr,WTERMSIG(status));
                  printtimes(time_l, time_a);
                  exit(EXIT_FAILURE);
              } else if (WIFSTOPPED(status)) {
                  printexec(args);
                  printf("Line:%d:Above process was stopped by signal %d\n", nr,WSTOPSIG(status));
              } else if (WIFCONTINUED(status)) {
                  printexec(args);
                  printf("Line:%d:Above process is being continued\n",nr);
              }
          } while(!WIFEXITED(status) && !WIFSIGNALED(status));
          time_a = getTime();
          printexec(args);
          printtimes(time_l, time_a);
      }
      nr++;
  }
  free(line);
  if(fclose(file)==-1) fprintf(stderr, "Error while closing file: %s\n", argv[1]);
  return 0;
}
