#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int i = 1;
char base = 'A';

void f(int a){
  base += i * ('Z' - 'A');
  i = -i;
}

void g(int a){
  printf("Received signal: %d\n", a);
  exit(EXIT_FAILURE);
}

int main(int argc, char const *argv[]) {
  int l = -1;
  char diff = 'Z'-'A'+1;
  struct sigaction action;
  action.sa_handler = g;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
  sigaction(SIGINT, &action, NULL);
  signal(SIGTSTP, f);
  while(1)
  {
    printf("%c\n", base + i * (l++)%diff);
	sleep(1);
  }
  return 0;
}
