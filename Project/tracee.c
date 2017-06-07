#include "load.h"

void setlimit(int MBs)
{
  struct rlimit mem_limit;
  mem_limit.rlim_cur = mem_limit.rlim_max = MBs;
  setrlimit(RLIMIT_AS, &mem_limit);
}


pid_t tracee(char** argv, int mem_limit)
{
  pid_t kid;
  int status;
  if((kid = fork()) == 0)
  {
    if (mem_limit) setlimit(mem_limit);
    ptrace(PTRACE_TRACEME, 0, NULL, NULL);
    kill(getpid(), SIGSTOP);
    execvp(*argv, argv);
    perror("Error execve:");
    exit(EXIT_FAILURE);
  }
  waitpid(kid, &status, 0);
  if(WIFEXITED(status) || WIFSIGNALED(status)) return -1;
  if(WIFSTOPPED(status) && WSTOPSIG(status) == SIGSTOP) return kid;
  return -1;
}
