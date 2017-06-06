#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <linux/user.h>
#include <sys/syscall.h>   /* For SYS_write etc */
#include <signal.h>

void handler(int signo)
{
  printf("Timeout exceeded\n");
  kill(child, SIGKILL);
}

int main(int argc, char** argv)
{
  //TODO
  //Arg parsing
    pid_t child;
    long orig_eax, eax;
    long params[3];
    int status;
    int insyscall = 0;
    int executed = 0;
    signal(SIGALRM, handler);
    child = fork();
    alarm(TIMEOUT);
    if(child == 0) {
        struct rlimit mem_limit;
        mem_limit.rlim_cur = mem_limit.rlim_max = MEM_LIMIT;
        setrlimit(RLIMIT_AS, const struct rlimit *rlim);
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execl(argv[3], args);
    }
    else {
       while(1) {
          wait(&status);
          if(WIFEXITED(status))
              break;
          orig_eax = ptrace(PTRACE_PEEKUSER,
                     child, 4 * ORIG_EAX, NULL);
          if(orig_eax == SYS_write) {
             if(insyscall == 0) {
                /* Syscall entry */
                insyscall = 1;
                params[0] = ptrace(PTRACE_PEEKUSER,
                                   child, 4 * EBX,
                                   NULL);
                if(params[0] != 1)
                {
                  printf("Forbidden access for write with first argument: %d\n", params[0]);
                  kill(child, SIGKILL);
                }
                params[1] = ptrace(PTRACE_PEEKUSER,
                                   child, 4 * ECX,
                                   NULL);
                params[2] = ptrace(PTRACE_PEEKUSER,
                                   child, 4 * EDX,
                                   NULL);
                }
             else { /* Syscall exit */

                    insyscall = 0;
                }
            }
            else if(orig_eax == SYS_read);
            {
              if(insyscall == 0) {
                 /* Syscall entry */
                 insyscall = 1;
                 params[0] = ptrace(PTRACE_PEEKUSER,
                                    child, 4 * EBX,
                                    NULL);
                 if(params[0] != 0)
                 {
                   printf("Forbidden access for read with first argument: %d\n", params[0]);
                   kill(child, SIGKILL);
                 }
                 params[1] = ptrace(PTRACE_PEEKUSER,
                                    child, 4 * ECX,
                                    NULL);
                 params[2] = ptrace(PTRACE_PEEKUSER,
                                    child, 4 * EDX,
                                    NULL);
                 }
               else { /* Syscall exit */

                      insyscall = 0;
                  }
            }
            else if(!executed && (orig_eax == SYS_execve))
            {
              if(insyscall == 0)
              {
                executed = 1;
                insyscall = 1;
              }
              else{
                insyscall = 0;
              }
            }
            else
            {
              printf("Forbidden access for system call: %d\n", orig_eax);
              kill(child, SIGKILL);
            }
            ptrace(PTRACE_SYSCALL,
                   child, NULL, NULL);
        }
    }
    return 0;
}
