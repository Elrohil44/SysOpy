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


void printtime(double time)
{
  printf("PID: %d\t time: %.6lf\n", getpid(),time);
}

void child_handler(int sig){}

int main(int argc, char const *argv[]) {
  srand(time(NULL));

  struct sigaction action;
  action.sa_handler = child_handler;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
  sigaction(SIGUSR2, &action, NULL);

  sleep(1000 * rand()%11);
  bbb.sival_int = rand();
  sigset_t set;
  siginfo_t info;
  struct timespec timeout;
  sigemptyset(&set);
  sigaddset(&set, SIGINT);
  sigaddset(&set, SIGUSR2);
  timeout.tv_sec = 1;
  timeout.tv_nsec = 0;
  struct timespec start, finish;
  double elapsed;







  clock_gettime(CLOCK_MONOTONIC, &start);
  if(sigqueue(getppid(), SIGUSR1, bbb))
  {
    printf("Couldn't send signal SIGUSR1 from %d to %d\n", getpid(), getppid());
  }
  int a;
  while (loop) {
    a = sigtimedwait(&set, &info, &timeout);
    if(a == SIGINT) {loop = 0;}
    else if(a == SIGUSR2)
    {
      if(sigqueue(getppid(), SIGRTMIN + rand()%32, bbb))
        printf("Error while sending signal SIGRT from %d to %d\n",getpid(), getppid());
        clock_gettime(CLOCK_MONOTONIC, &finish);
        elapsed = (finish.tv_sec - start.tv_sec);
        elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
        printtime(elapsed);
        loop = 0;
    }
    else if(a == -1 && errno == EAGAIN)
    {
      if(sigqueue(getppid(), SIGUSR1, bbb))
      {
        printf("Couldn't send signal SIGUSR1 from %d to %d\n", getpid(), getppid());
      }
    }
  }

  return 0;
}
