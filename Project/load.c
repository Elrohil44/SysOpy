#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <sys/user.h>
#include <sys/reg.h>
#include <sys/syscall.h>   /* For SYS_write etc */
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

pid_t child;

void handler(int signo)
{
  printf("Timeout exceeded\n");
  kill(child, SIGKILL);
}

int isNumber(const char* arg)
{
  int i=0;
  while(arg[i])
  {
    if(arg[i]<'0' || arg[i]>'9') return 0;
    i++;
  }
  return 1;
}

int wait_for_syscall(pid_t child)
{
  int status;
  while (1) {
    ptrace(PTRACE_SYSCALL, child, 0, 0);
    waitpid(child, &status, 0);
    if(WIFEXITED(status)) return -1;
    if(WIFSIGNALED(status)) return -1;
    if(WIFSTOPPED(status) && (WSTOPSIG(status) & 0x80)) return 0;
  }
}

int main(int argc, char** argv)
{
  if(argc < 4 || !(isNumber(argv[1]) && isNumber(argv[2]))) exit(EXIT_FAILURE);
  int MEM_LIMIT = atoi(argv[1]) * 1024 * 1024;
  int TIMEOUT = atoi(argv[2]);
  struct user_regs_struct regs;
  int status;
  int insyscall = 0;
  int executed = 0;
  int brked = 0;
  signal(SIGALRM, handler);
  child = fork();
  if(child == 0)
  {
    struct rlimit mem_limit;
    mem_limit.rlim_cur = mem_limit.rlim_max = MEM_LIMIT;
    setrlimit(RLIMIT_AS, &mem_limit);
    ptrace(PTRACE_TRACEME, 0, NULL, NULL);
    raise(SIGSTOP);
    execvp(argv[3], &argv[3]);
    exit(EXIT_FAILURE);
  }
  else
  {
    waitpid(child, &status, 0);
    alarm(TIMEOUT);
    ptrace(PTRACE_SETOPTIONS, child, 0, PTRACE_O_TRACESYSGOOD);
    while(1)
    {
      if(wait_for_syscall(child)) break;
      ptrace(PTRACE_GETREGS, child, 0, &regs);
      if(!insyscall)
      {
        printf("SYSCALL %lld\n", regs.orig_rax);
        insyscall = 1;
      }
      else
      {
        insyscall = 0;
      }
    }
  return 0;
  }
}
