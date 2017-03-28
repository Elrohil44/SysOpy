#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

int i = 1;

void f(int a){
  i = -i;
}

int main(int argc, char const *argv[]) {
  int l = -1;
  int diff = 'Z'-'A'+1;
  signal(SIGTSTP, f);
  while(1)
  {
    printf("%c\n", 'A'+(l+=i)%diff);
  }
  return 0;
}
