#include "bench.h"
#include <stdio.h>

void printtimes(struct rusage time_l, struct rusage time_a)
{
  printf("user:\t%.6lf\n",(time_a.ru_utime.tv_sec + (double)time_a.ru_utime.tv_usec/1e6)-
                          (time_l.ru_utime.tv_sec + (double)time_l.ru_utime.tv_usec/1e6));
  printf("sys:\t%.6lf\n",(time_a.ru_stime.tv_sec + (double)time_a.ru_stime.tv_usec/1e6)-
                         (time_l.ru_stime.tv_sec + (double)time_l.ru_stime.tv_usec/1e6));
}

struct rusage getTime()
{
  struct rusage time;
  getrusage(RUSAGE_SELF,&time);
  return time;
}
