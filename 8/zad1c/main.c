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
int fd;
int p =0;
static pthread_mutex_t mutex;
pthread_key_t* keys;

struct record
{
  int id;
  char text[1025 - sizeof(int)];
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

void freeing(void* h)
{
  free(h);
}

void* f(void* pp)
{
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
  pthread_mutex_lock(&mutex);
  //fprintf(stderr, "Locked\n");
  for(int i=0;i<T;i++) pthread_setspecific(keys[i], malloc(sizeof(struct record)));
  //fprintf(stderr, "Allocating\n");
  pthread_mutex_unlock(&mutex);
  const char* pattern = (char*) pp;
  struct record* record;
  int count = 1;
  while(count)
  {
    count = 0;
    pthread_mutex_lock(&mutex);
    for(int i=0;i<T;i++, count++)
    {
      record = pthread_getspecific(keys[i]);
      if(read(fd, &record->id, sizeof(int)) == 0 || read(fd, record->text, sizeof(record->text) - 1) == 0)
      {
        break;
      }

      record->text[1020] = '\0';
      // free(pthread_getspecific(keys[i]));
      pthread_setspecific(keys[i], record);
    }
    pthread_mutex_unlock(&mutex);
    for(int i=0;i<count;i++)
    {
      record = pthread_getspecific(keys[i]);
      if(strstr(record->text, pattern) != NULL)
      {
        pthread_mutex_lock(&mutex);
        printf("%ld found an occurence in %d\n", pthread_self(), record->id);
        pthread_mutex_unlock(&mutex);
      }
      // free(record);
      // pthread_setspecific(keys[i], NULL);
    }
  }
  pthread_mutex_lock(&mutex);
  ++p;
  pthread_mutex_unlock(&mutex);
  return NULL;
}

int main(int argc, char const *argv[]) {
  if (argc !=5 || !isNumber(argv[1]) || !isNumber(argv[3])) printusage(argv[0]);
  N = atoi(argv[1]);
  T = atoi(argv[3]);

  pthread_mutex_init(&mutex, NULL);
  fd = open(argv[2], O_RDONLY);

  IDs = malloc(N * sizeof(pthread_t));
  keys = malloc(T * sizeof(pthread_key_t));
  for(int i=0; i<T; i++) pthread_key_create(&keys[i], &freeing);
  pthread_mutex_lock(&mutex);
  for(int i=0; i<N; i++)
  {
    pthread_create(&IDs[i], NULL, f,(void*) argv[4]);
  }

  for(int i=0; i<N; i++)
  {
    pthread_detach(IDs[i]);
  }

  pthread_mutex_unlock(&mutex);
  while(p<T);
  pthread_mutex_destroy(&mutex);
  for(int i=0; i<T; i++) pthread_key_delete(keys[i]);
  free(keys);
  free(IDs);
  return 0;
}
