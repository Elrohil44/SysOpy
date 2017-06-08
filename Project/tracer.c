#include "load.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

pid_t TRACEE;
const int FLAGS = PTRACE_O_TRACESYSGOOD | PTRACE_O_EXITKILL;
long long opened = 0;

sigset_t set;
void init_sigset(sigset_t* set);
void handler(int signo);
int wait_for_syscall(pid_t child);
int check_syscall(struct user_regs_struct regs);
void settimeout(int secs)
{
  signal(SIGALRM, handler);
  alarm(secs);
}


void trace(pid_t tracee, int timeout)
{
  if (tracee <= 0) return;
  TRACEE = tracee;
  init_sigset(&set);
  int entering = 1;
  int exec = 0;
  int signo;
  struct user_regs_struct regs;
  ptrace(PTRACE_SETOPTIONS, TRACEE, 0, FLAGS);
  while(1)
  {
    if((signo = wait_for_syscall(TRACEE)))
    {
      fprintf(stderr, "Tracee received: %s\n", strsignal(signo));
      fprintf(stderr, "Press return to exit...\n");
      getchar();
      break;
    }
    ptrace(PTRACE_GETREGS, TRACEE, 0, &regs);

    if(entering)
    {
      if(!exec && regs.orig_rax == SYS_execve)
      {
        exec = 1;
        if(timeout) settimeout(timeout);
      }
      else
      {
        if(check_syscall(regs))
        {
          fprintf(stderr, "Forbidden operation for syscall: %lld\n", regs.orig_rax);
	  fprintf(stderr, "Press return to exit... \n");
	  getchar();
          fprintf(stderr, "Sending SIGKILL to Tracee...\n");
          kill(TRACEE, SIGKILL);
          break;
        }
      }
    }
    else if(regs.orig_rax == SYS_open)
    {
      opened = regs.rax;
    }

    entering = 1 - entering;
  }
}


int check_syscall(struct user_regs_struct regs)
{
  switch (regs.orig_rax) 
  {
    case SYS_writev:
    case SYS_write:
      if(regs.rdi != STDOUT_FILENO && regs.rdi !=STDERR_FILENO)
      {
        fprintf(stderr, "You are not allowed to call write\n");
        fprintf(stderr, "with first argument equal to: %lld\n", regs.rdi);
        return -1;
      }
      break;
    case SYS_readv:
    case SYS_read:
      if(regs.rdi != STDIN_FILENO && regs.rdi != opened)
      {
        fprintf(stderr, "You are not allowed to call read\n");
        fprintf(stderr, "with first argument equal to: %lld\n", regs.rdi);
        return -1;
      }
      break;
    case SYS_open:
      if(!opened)
      {
        if(((regs.rsi & (~O_RDONLY)) & (~O_CLOEXEC)))
        {
          fprintf(stderr, "You are not allowed to call open with these FLAGS\n");
          return -1;
        }
      }
      else
      {
        fprintf(stderr, "You are not allowed to open more than 1 file\n");
        return -1;
      }
      break;
    case SYS_close:
      if(!opened)
      {
        fprintf(stderr, "You are not allowed to close not opened file\n");
        return -1;
      }
      else if(regs.rdi != opened)
      {
        fprintf(stderr, "You are not allowed to close this descriptor\n");
        return -1;
      }
      opened = 0;
      break;
    case SYS_fstat:
      if(regs.rdi != STDIN_FILENO && regs.rdi != STDOUT_FILENO)
      {
        if(!opened)
        {
          fprintf(stderr, "You are not allowed to fstat not opened file\n");
          return -1;
        }
        else if(regs.rdi != opened)
        {
          fprintf(stderr, "You are not allowed to fstat this descriptor\n");
          return -1;
        }
      }
      break;
    case SYS_mprotect:
    case SYS_arch_prctl:
    case SYS_munmap:
    case SYS_exit:
    case SYS_exit_group:
    case SYS_mmap:
    case SYS_brk:
    case SYS_access:
    case SYS_set_tid_address:
    case SYS_set_robust_list:
    case SYS_rt_sigaction:
    case SYS_rt_sigprocmask:
    case SYS_getrlimit:
      break;
    default:
      return -1;
  }
  return 0;
}

int wait_for_syscall(pid_t child)
{
  int status;
  while (1) {
    ptrace(PTRACE_SYSCALL, child, 0, 0);
    if(waitpid(child, &status, 0) < 0)
    {
      perror("Error wating for tracee:");
      exit(EXIT_FAILURE);
    };
    if(WIFEXITED(status))
    {
      fprintf(stderr, "Tracee has exited\n");
      exit(EXIT_SUCCESS);
    }
    if(WIFSIGNALED(status))
    {
      fprintf(stderr, "Tracee has been terminated with signal\n");
      exit(EXIT_SUCCESS);
    }
    if(WIFSTOPPED(status) && (WSTOPSIG(status) & 0x80)) return 0;
    if(WIFSTOPPED(status))
    {
      if(sigismember(&set, WSTOPSIG(status)))
        return WSTOPSIG(status);
    }
  }
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

void handler(int signo)
{
  fprintf(stderr, "Timeout exceeded\n");
  if(TRACEE) kill(TRACEE, SIGKILL);
}


void printusage(char* name, int exiting)
{
  printf("Usage:\t %s MEM_LIMIT TIMEOUT PROGRAM {ARG}\n", name);
  printf("\n\n\t MEM_LIMIT - Virtual Memory limit for sandboxed process\n");
  printf("\n\n\t\t 0 - inherited, N - limit N MBs\n");
  printf("\n\n\t TIMEOUT - Timeut for sandboxed process in seconds\n");
  printf("\n\n\t\t 0 - no timeout\n");
  printf("\n\n\t PROGRAM - Path to program to isolate\n");
  printf("\n\n\t ARG - Argument for isolated program\n");
  if(exiting) exit(exiting);
}

void init_sigset(sigset_t* set)
{
  sigemptyset(set);
  sigaddset(set, SIGINT);
  sigaddset(set, SIGTERM);
  sigaddset(set, SIGHUP);
  sigaddset(set, SIGQUIT);
  sigaddset(set, SIGILL);
  sigaddset(set, SIGABRT);
  sigaddset(set, SIGFPE);
  sigaddset(set, SIGSEGV);
  sigaddset(set, SIGPIPE);
  sigaddset(set, SIGUSR1);
  sigaddset(set, SIGUSR2);
}

