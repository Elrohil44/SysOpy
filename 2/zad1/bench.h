#ifndef BENCH_H
#define BENCH_H

#include <sys/time.h>
#include <sys/resource.h>

void printtimes(struct rusage time_l, struct rusage time_a);
struct rusage getTime();


#endif
