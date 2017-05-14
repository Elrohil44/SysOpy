#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>

#define MAX_T 256

pthread_t* IDs;
int T, N;
int fdd;
int p =0;
static pthread_mutex_t mutex;

struct record
{
  int id;
  char text[1024 - sizeof(int)];
};

void printusage(const char * name)
{
  printf("Usage:\t %s N L T W\n\n", name);
  printf("\tN - number of threads\n");
  printf("\tL - filename\n");
  printf("\tT - records per read\n");
  printf("\tW - phrase to find\n");
  exit(0);
}


int isNumber(const char* arg)
{
  for(int i=0;i<strlen(arg);i++) if(arg[i]<'0' || arg[i]>'9') return 0;
  return 1;
}

void* f(void* pp)
{
  int fd = dup(fdd);
  const char* pattern = (char*) pp;
  struct record records[MAX_T];
  int count = 1;
  int f = 0;
  while(count && !f)
  {
    count = 0;
    pthread_mutex_lock(&mutex);
    for(int i=0;i<T;i++, count++)
    {
      if(read(fd, &records[i], sizeof(struct record)) == 0) break;
    }
    pthread_mutex_unlock(&mutex);
    for(int i=0;i<count && !f;i++)
    {
      if(strstr(records[i].text, pattern) != NULL)
      {
        printf("%ld found an occurence in %d\n", pthread_self(), records[i].id);
        f = 1;
      }
    }
  }
  if(!count) pthread_mutex_destroy(&mutex);
  return NULL;
}

int main(int argc, char const *argv[]) {
  if (argc !=5 || !isNumber(argv[1]) || !isNumber(argv[3])) printusage(argv[0]);
  N = atoi(argv[1]);
  T = atoi(argv[3]);

  pthread_mutex_init(&mutex, NULL);
  fdd = open(argv[2], O_RDONLY);

  IDs = malloc(N * sizeof(pthread_t));
  for(int i=0; i<N; i++)
  {
    pthread_create(&IDs[i], NULL, f,(void*) argv[4]);
  }

  for(int i=0; i<N; i++)
  {
    pthread_detach(IDs[i]);
  }

  free(IDs);

  return 0;
}
