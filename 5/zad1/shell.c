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

int parseline(char* line, char** programs)
{
  int i=0;
  if((programs[0]=strtok(line, "|\n"))!=NULL)
    while (i<MAX_PROGRAMS-2 && (programs[++i]=strtok(NULL, "|\n"))!=NULL);
  programs[i]=NULL;
  return i;
}

int parseargs(char* program, char** args)
{
  int i=0;
  if((args[0]=strtok(program, " \t\r\n"))!=NULL)
    while (i<MAX_ARGS-2 && (args[++i]=strtok(NULL, " \t\r\n"))!=NULL);
  args[i]=NULL;
  return i;
}

void printexec(char* const* args)
{
  if(args==NULL) return;
  for(int i=0;args[i]!=NULL;i++) printf("%s ", args[i]);
  printf("\n");
}

void swappipes(int** pipe1, int** pipe2)
{
  int* tmp = *pipe1;
  *pipe1 = *pipe2;
  *pipe2 = tmp;
}

int main(int argc, char const *argv[]) {
  char* args[MAX_ARGS];
  char* programs[MAX_PROGRAMS];
  pid_t pids[MAX_PROGRAMS];
  int pipeR[2];
  int pipeW[2];
  char* line=NULL;
  size_t n=0;
  int programsc;
  int status;
  int f = 1;
  printf(">>> ");
  fflush(stdout);
  while((getline(&line,&n,stdin))!=-1)
  {
    programsc = parseline(line, programs);
    for(int i=0; i<programsc; i++)
    {
      if(i>0 && f) swappipes((int**)&pipeW, (int**)&pipeR);
      if(i==0) pipe(pipeW);
      if(parseargs(programs[i], args))
      {

          f = 1;
          if((pids[i] = fork())==0)
          {

            if(i<programsc - 1)
            {
            close(pipeW[0]);
            if(dup2(pipeW[1], STDOUT_FILENO)==-1) printf("WutwUt\n");
            close(pipeW[1]);

            // close(pipeW[0]);
            }
            if(i>0)
            {
              close(pipeR[1]);
              if(dup2(pipeR[0],STDIN_FILENO) == -1) printf("WutWut\n");
              close(pipeR[0]);

            }
            if(execvp(*args, args) == -1) exit(EXIT_FAILURE);
          }
      }
      else
      {
        f = 0;
        continue;
      }
      if(i>0)
      {
        close(pipeR[0]);
        close(pipeR[1]);
      }
      if(i<programsc - 1) pipe(pipeR);
    }
    close(pipeW[1]);
    close(pipeW[0]);
    // close(pipeR[0]);
    // close(pipeR[1]);
    for(int i=0 ; i<programsc; i++)
    {
      waitpid(pids[i], &status, 0);
    }
    printf(">>> ");
    fflush(stdout);
  }
  free(line);
  return 0;
}
