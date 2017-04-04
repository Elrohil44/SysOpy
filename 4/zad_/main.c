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
  printf("Usage:\t %s N L T=0\n\n", name);
  printf("\tN - number of children\n");
  printf("\tL - number of signals\n");
  printf("\tT=0 - interval\n");
  exit(0);
}

void swap(pid_t* a, pid_t* b)
{
  pid_t tmp = *b;
  *b = *a;
  *a = tmp;
}

int isNumber(const char* arg)
{
  for(int i=0;i<strlen(arg);i++) if(arg[i]<'0' || arg[i]>'9') return 0;
  return 1;
}


sig_atomic_t volatile i = 0;
union sigval bbb;
int loop = 1;
static sig_atomic_t* pids;
int L;
int child = 1;
int N;
static pid_t root;

void g(int sig)
{
  switch (sig) {
    case SIGUSR1:
        if(child == 0)
        {
          if(sigqueue(getppid(), SIGUSR1, bbb) == -1)
            printf("Error\n");
        }
        i++;
        break;

    case SIGUSR2:
        printf("Child: %d SIGUSR1 received\n", i);
        loop = 0;
        break;

    case SIGINT:
        if(child == 0) exit(EXIT_FAILURE);
        printf("Received signal SIGINT\n");
        for(int i=0; i<N; i++)
        {

          if(pids[i]!=-1 && kill(pids[i], 0) == 0  )
    	     {

            if(kill(pids[i], SIGINT) == -1)
            printf("Error while sending signal SIGINT from %d to %d\n",getpid(), pids[i]);
          }
    	     }
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

void send(int type, int childno);

int main(int argc, char const *argv[]) {
  if (argc < 3) printusage(argv[0]);
  if (!isNumber(argv[1]) || !isNumber(argv[2])) printusage(argv[0]);
  srand(time(NULL));
  int T;
  if(argc == 4 && isNumber(argv[3])) T = atoi(argv[3]);
  else T = 0;
  L = atoi(argv[2]);
  N = atoi(argv[1]);
  root = getpid();

  const char* options[]={"KILL","SIGQUEUE","REALTIME"};

  struct sigaction action;
  action.sa_handler = g;
  action.sa_flags = SA_RESTART;
  sigemptyset(&action.sa_mask);
  sigaddset(&action.sa_mask, SIGUSR2);
  sigaddset(&action.sa_mask, SIGRTMIN+1);
  sigaction(SIGUSR1, &action, NULL);
  sigaction(SIGINT, &action, NULL);
  sigaction(SIGUSR2, &action, NULL);
  for (int i= SIGRTMIN;i<=SIGRTMIN+1;i++) sigaction(i, &action, NULL);

  pids = malloc(N * sizeof(pid_t));
  for(int i=0; i<N; i++) pids[i] = -1;
  for(int i=0; i<N; i++)
  {
    if ((pids[i] = fork()) == 0)
    {
      child = 0;
      while(loop) pause();
      free(pids);
      exit(EXIT_SUCCESS);
    }
  }
  int status, option = 0, childind;
  while(N>0)
  {
    i=0;
    childind = rand()%N;
    if(pids[childind] != -1){
    option = (option + 1)%3;
    printf("Parent: Sending %d signals with %s\n", L, options[option]);
    sleep(1);
    send(option + 1, childind);
    waitpid(pids[childind], &status, 0);
    pids[childind] = -1;
    printf("Parent: %d signals received\n", i);
  }
    swap(&pids[--N], &pids[childind]);
    printf("\n");
    sleep(T);
  }
  free(pids);
  return 0;
}


void send(int type, int childno)
{
  switch (type) {
    case 1:
    //sleep(1);
    for(int l=0;l<L;l++)
    {
      kill(pids[childno], SIGUSR1);
      //sleep(1);
    }
    //sleep(1);
    kill(pids[childno], SIGUSR2);
    break;
    case 2:
    for(int l=0;l<L;l++)
    {
      sigqueue(pids[childno], SIGUSR1, bbb);
      //sleep(1);
    }
    sigqueue(pids[childno], SIGUSR2, bbb);
    break;
    case 3:
    //sleep(1);
    for(int l=0;l<L;l++)
    {
      kill(pids[childno], SIGRTMIN);
      //sleep(1);
    }
    //sleep(1);
    kill(pids[childno], SIGRTMIN+1);
    break;
    default:
    exit(EXIT_FAILURE);
  }
}
