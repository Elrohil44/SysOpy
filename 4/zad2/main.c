#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>
#include <sys/resource.h>

void printusage(const char * name)
{
  printf("Usage:\t %s N M\n\n", name);
  printf("\tN - number of children\n");
  printf("\tM - number of requests to wait\n");
  exit(0);
}

int isNumber(const char* arg)
{
  for(int i=0;i<strlen(arg);i++) if(arg[i]<'0' || arg[i]>'9') return 0;
  return 1;
}


sig_atomic_t requests = 0;
static sig_atomic_t* pids;
sig_atomic_t i = -1;
static int M;
static int volatile N, _N;
static int loop = 1, loop1 = 1;


void swap(pid_t* a, pid_t* b)
{
  pid_t tmp = *b;
  *b = *a;
  *a = tmp;
}

void g(int sig, siginfo_t* a, void* b)
{
  switch (sig) {
    case SIGUSR1:
    printf("Received signal SIGUSR1 from %d\n", a->si_pid);
    fflush(stdout);
    if(++requests < M)
    {
      for(int l=i+1;l < N;l++)
        if (pids[l] == a->si_pid)
        {
          swap(&pids[l], &pids[i + 1]);
          i++;
        }
    }
    else
    {
      loop1 = 0;
      printf("Sending signal SIGUSR2 from %d to %d\n",getpid(), a->si_pid );
      fflush(stdout);
      if(kill(a->si_pid, SIGUSR2) == -1)
        printf("Error while sending signal SIGUSR2 from %d to %d\n",getpid(), a->si_pid );
    }
    break;
    case SIGINT:
    printf("Received signal SIGINT\n");
    for(int i=0; i<_N; i++)
    {

      if(kill(pids[i], 0) == 0 && pids[i]!=-1)
	{

        printf("Sending signal SIGINT from %d to %d\n",getpid(), pids[i]);
        fflush(stdout);
        if(kill(pids[i], SIGINT) == -1)
        printf("Error while sending signal SIGINT from %d to %d\n",getpid(), pids[i]);
	}
    }
    exit(EXIT_FAILURE);
    break;
    default:
    if(sig <= SIGRTMAX && sig >= SIGRTMIN)
    {
      printf("Received exit code %d from %d\n", sig, a->si_pid);
      fflush(stdout);
      for(int i=0; i<_N; i++) if(pids[i]==a->si_pid) pids[i] = -1;
      N--;
    }
  }

}

int main(int argc, char const *argv[]) {
  if (argc != 3) printusage(argv[0]);
  if (!isNumber(argv[1]) || !isNumber(argv[2])) printusage(argv[0]);
  M = atoi(argv[2]);
  N = _N = atoi(argv[1]);
  struct sigaction action;
  action.sa_sigaction = g;
  sigemptyset(&action.sa_mask);
  //sigaddset(&action.sa_mask, SIGUSR1);
  //for (int i= SIGRTMIN;i<=SIGRTMAX;i++) sigaddset(&action.sa_mask, i);
  action.sa_flags = SA_SIGINFO;
  sigaction(SIGUSR1, &action, NULL);
  sigaction(SIGINT, &action, NULL);
  for (int i= SIGRTMIN;i<=SIGRTMAX;i++) sigaction(i, &action, NULL);
  pids = malloc(N * sizeof(pid_t));
  for(int i=0; i<N; i++) pids[i] = -1;
  for(int i=0; i<N; i++)
  {
    if ((pids[i] = fork()) == 0)
    {
      execlp("./child", "./child", NULL);
      exit(0);
    }
  }
  while(loop)
  {
      while (loop1) pause();
      for(int l=i; l>-1; l--)
      {
        kill(pids[l], SIGUSR2);
      }
      loop = 0;
  }
  while (N>0) pause();
  free(pids);
  return 0;
}
