#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#define BUF_SIZE 50

struct complex
{
  double x;
  double y;
};
struct complex add(struct complex tmp, struct complex c);
struct complex sqr(struct complex tmp);
struct complex mand(struct complex tmp, struct complex c);
double mod(struct complex c);


int isNumber(const char* arg)
{
  for(int i=0;i<strlen(arg);i++) if(arg[i]<'0' || arg[i]>'9') return 0;
  return 1;
}


int main(int argc, char const *argv[]) {
  if(argc != 4 || !isNumber(argv[2]) || !isNumber(argv[3])) exit(EXIT_FAILURE);
  srand(time(NULL) ^ getpid() << 16);
  int pipe = open(argv[1], O_WRONLY);
  int N = atoi(argv[2]);
  int K = atoi(argv[3]);
  char buf[BUF_SIZE] = "";
  int iters = 0;
  double x;
  double y;
  struct complex c;
  struct complex u;
  for(int i=0; i<N; i++)
  {
    u.x = 0;
    u.y = 0;
    x = 3.0 * ((rand()%RAND_MAX) / (double)RAND_MAX) - 2;
    y = 2.0 * ((rand()%RAND_MAX) / (double)RAND_MAX) - 1;
    c.x = x;
    c.y = y;
    iters = 0;
    while(mod(u) <= 4 && iters < K)
    {
      iters++;
      u = mand(u, c);
    }
    sprintf(buf, " %lf %lf %d \n", x, y, iters);
    write(pipe, buf, BUF_SIZE);
  }
  close(pipe);
  return 0;
}


struct complex add(struct complex tmp, struct complex c)
{
  tmp.x += c.x;
  tmp.y += c.y;
  return tmp;
}


struct complex sqr(struct complex tmp)
{
  double x = tmp.x;
  tmp.x = x*x - tmp.y*tmp.y;
  tmp.y = 2*x*tmp.y;
  return tmp;
}

struct complex mand(struct complex tmp, struct complex c)
{
  tmp = add(c, sqr(tmp));
  return tmp;
}

double mod(struct complex c)
{
  return c.x*c.x + c.y*c.y;
}
