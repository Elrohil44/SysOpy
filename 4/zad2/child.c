#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>
#include <sys/resource.h>
#include <errno.h>


clock_t start_time;
clock_t last_time;
static int loop = 1;
union sigval bbb;
pid_t parent;

void printtime(double time)
{
  printf("PID: %d\t time: %.6lf\n", getpid(),time);
}

void child_handler(int sig){}

int main(int argc, char const *argv[]) {
  srand(time(NULL));
  parent = getppid();
  struct sigaction action;
  action.sa_handler = child_handler;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
  sigaction(SIGUSR2, &action, NULL);

  sleep(rand()%11);
  bbb.sival_int = rand();
  sigset_t set;
  siginfo_t info;
  struct timespec timeout;
  sigemptyset(&set);
  sigaddset(&set, SIGINT);
  sigaddset(&set, SIGUSR2);
  timeout.tv_sec = 0;
  timeout.tv_nsec = 10e7;
  struct timespec start, finish;
  double elapsed;







  clock_gettime(CLOCK_MONOTONIC, &start);
  //printf("Sending signal SIGUSR1 from %d to %d\n", getpid(), parent);
  fflush(stdout);
  if(sigqueue(parent, SIGUSR1, bbb))
  {
    printf("Couldn't send signal SIGUSR1 from %d to %d\n", getpid(), parent);
  }
  int a;
  while (loop) {
    a = sigtimedwait(&set, &info, &timeout);
    if(a == SIGINT) {exit(EXIT_FAILURE);}
    else if(a == SIGUSR2)
    {
	printf("Child %d: Received signal SIGUSR2 from %d\n",getpid(), info.si_pid);
  fflush(stdout);
      //printf("Sending signal SIGRT from %d to %d\n",getpid(), parent);
      fflush(stdout);
      if(sigqueue(parent, SIGRTMIN + rand()%32, bbb))
        printf("Error while sending signal SIGRT from %d to %d\n",getpid(), parent);
        else loop = 0;
        clock_gettime(CLOCK_MONOTONIC, &finish);
        elapsed = (finish.tv_sec - start.tv_sec);
        elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
        printtime(elapsed);
    }
    else if(a == -1 && errno == EAGAIN)
    {
       //printf("Sending signal SIGUSR1 from %d to %d\n", getpid(), parent);
       fflush(stdout);
      if(sigqueue(parent, SIGUSR1, bbb))
      {
        printf("Couldn't send signal SIGUSR1 from %d to %d\n", getpid(), parent);
      }
    }
  }

  return 0;
}
