#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>
#include <sys/resource.h>
#include <sys/wait.h>

void printusage(const char * name)
{
  printf("Usage:\t %s L T\n\n", name);
  printf("\tL - number of signals\n");
  printf("\tT - type\n");
  exit(0);
}

int isNumber(const char* arg)
{
  for(int i=0;i<strlen(arg);i++) if(arg[i]<'0' || arg[i]>'9') return 0;
  return 1;
}


pid_t child;
sig_atomic_t i = 0;
union sigval bbb;
int loop = 1;
sigset_t set;

void g(int sig)
{
  switch (sig) {
    case SIGUSR1:
        if(child == 0)
        {
          sigqueue(getppid(), SIGUSR1, bbb);
        }
        i++;
        break;

    case SIGUSR2:
        printf("Child: %d SIGUSR1 received\n", i);
        loop = 0;
        break;

    case SIGINT:
        if(kill(child, 0) == 0 && child > 0)
        if(kill(child, SIGINT) == -1)
        printf("Error while sending signal SIGINT from %d to %d\n",getpid(), child);
        exit(EXIT_FAILURE);
    default:
    break;
    }

    if(sig == SIGRTMIN){
      if(child == 0) sigqueue(getppid(), SIGRTMIN, bbb);
      ++i;
    }

    if(sig == SIGRTMIN + 1){
    printf("Child: %d SIGRTMIN received\n", i);
    loop = 0;
    }

}

int main(int argc, char const *argv[]) {
  if (argc != 3) printusage(argv[0]);
  if (!isNumber(argv[1]) || !isNumber(argv[2])) printusage(argv[0]);
  int T = atoi(argv[2]);
  int L = atoi(argv[1]);

  struct sigaction action;
  action.sa_handler = g;
  sigemptyset(&action.sa_mask);
  sigaddset(&action.sa_mask, SIGUSR2);
  sigaddset(&action.sa_mask, SIGRTMIN+1);
  sigaction(SIGUSR1, &action, NULL);
  sigaction(SIGINT, &action, NULL);
  sigaction(SIGUSR2, &action, NULL);
  for (int i= SIGRTMIN;i<=SIGRTMIN+1;i++) sigaction(i, &action, NULL);
  if ((child = fork()) == 0)
  {
      while(loop) pause();
      exit(EXIT_SUCCESS);
  }
  else
  {
    printf("Parent: Sending %d signals\n", L);
    switch (T) {
      case 1:
      for(int l=0;l<L;l++)
      {
        kill(child, SIGUSR1);
        sleep(1);
      }
      kill(child, SIGUSR2);
      break;
      case 2:
      for(int l=0;l<L;l++)
      {
        sigqueue(child, SIGUSR1, bbb);
        sleep(1);
      }
      sigqueue(child, SIGUSR2, bbb);
      break;
      case 3:
      for(int l=0;l<L;l++)
      {
        kill(child, SIGRTMIN);
        sleep(1);
      }
      kill(child, SIGRTMIN+1);
    }
    int status;
    wait(&status);
    printf("Parent: %d signals received\n", i);
  }
  return 0;
}
