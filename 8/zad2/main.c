#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>

#define MAX_T 256

pthread_t* IDs;
int T, N;
int sig;
int option;
int fd;
int p =0;
int cancelled = 0;
static pthread_mutex_t mutex;
pthread_key_t* keys;
sigset_t set;

struct record
{
  int id;
  char text[1025 - sizeof(int)];
};

void printusage(const char * name)
{
  printf("Usage:\t %s N L T W O S\n\n", name);
  printf("\tN - number of threads\n");
  printf("\tL - filename\n");
  printf("\tT - records per read\n");
  printf("\tW - phrase to find\n");
  printf("\tO - option to test\n");
  printf("\tS - signal number\n");
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

void sig_handler(int a)
{
  printf("[%d][%ld] received signal %s\n", getpid(), pthread_self(), strsignal(a));
}

void* f(void* pp)
{
  if (pthread_equal(IDs[0], pthread_self()) && option == 4)
  {
    pthread_sigmask(SIG_BLOCK, &set, NULL);
    p = 1;
  }
  if(option == 6 && pthread_equal(IDs[0], pthread_self()))
  {
    printf("[%d][%ld] Dividing by 0\n", getpid(), pthread_self());
    int t = 1 / 0;
    printf("[%d][%ld] Divided by 0, t = %d\n", getpid(), pthread_self(), t);
  }
  int q = 0;
  printf("[%d][%ld] Thread\n", getpid(), pthread_self());
  while(1)
  {
    printf("[%ld]%d\n", pthread_self(), q++);
    sleep(1);
  }
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
  //fprintf(stderr, "Waiting for lock to allocate\n");
  pthread_mutex_lock(&mutex);
  //fprintf(stderr, "Locked\n");
  for(int i=0;i<T;i++) pthread_setspecific(keys[i], malloc(sizeof(struct record)));
  //fprintf(stderr, "Allocating\n");
  pthread_mutex_unlock(&mutex);
  const char* pattern = (char*) pp;
  // struct record records[MAX_T];
  struct record* record;
  int count = 1;
  int f = 0;
  while(count && !f)
  {
    count = 0;
    //fprintf(stderr, "Waiting for lock\n");
    pthread_mutex_lock(&mutex);
    //fprintf(stderr, "Locked\n");
    //fprintf(stderr, "Reading\n");
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
    //fprintf(stderr, "Checking\n");
    for(int i=0;i<count && !f;i++)
    {
      record = pthread_getspecific(keys[i]);
      if(strstr(record->text, pattern) != NULL)
      {
        pthread_mutex_lock(&mutex);
        //fprintf(stderr, "Locked\n");
        printf("%ld found an occurence in %d\n", pthread_self(), record->id);
        f = 1;
      }
      // free(record);
      // pthread_setspecific(keys[i], NULL);
    }
  }
  if(f)
  {
    for (int i=0; i<N; i++)
    {
        if(!pthread_equal(IDs[i], pthread_self()))
        {
          pthread_cancel(IDs[i]);
        }
    }
    pthread_mutex_unlock(&mutex);
  }
  pthread_exit(NULL);
  return NULL;
}

int main(int argc, char const *argv[]) {
  if (argc !=7 || !isNumber(argv[1]) || !isNumber(argv[3])
  || !isNumber(argv[5]) || !isNumber(argv[6])) printusage(argv[0]);
  N = atoi(argv[1]);
  T = atoi(argv[3]);
  option = atoi(argv[5]);
  sig = atoi(argv[6]);
  sigemptyset(&set);
  sigaddset(&set, sig);
  struct sigaction action;
  action.sa_handler = sig_handler;

  if (option == 2) pthread_sigmask(SIG_BLOCK, &set, NULL);
  else if(option == 3 || option == 5) sigaction(sig, &action, NULL);
  printf("[%d][%ld] Main\n", getpid(), pthread_self());
  pthread_mutex_init(&mutex, NULL);
  fd = open(argv[2], O_RDONLY);

  IDs = malloc(N * sizeof(pthread_t));
  keys = malloc(T * sizeof(pthread_key_t));
  for(int i=0; i<T; i++) pthread_key_create(&keys[i], freeing);
  pthread_mutex_lock(&mutex);
  //fprintf(stderr, "Locked\n");
  for(int i=0; i<N; i++)
  {
    pthread_create(&IDs[i], NULL, f,(void*) argv[4]);
  }
  pthread_mutex_unlock(&mutex);

  if(option == 1 || option == 2 || option == 3) raise(sig);
  else if(option == 5 || option == 4)
  {
    if(option == 4) while(!p);
    pthread_kill(IDs[0], sig);
  }
  //fprintf(stderr, "Unlocked\n");
  pthread_join(IDs[0], NULL);
  for(int i=0; i<T; i++) pthread_key_delete(keys[i]);
  pthread_mutex_destroy(&mutex);
  free(keys);
  free(IDs);

  return 0;
}
