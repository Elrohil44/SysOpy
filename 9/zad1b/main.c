#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <limits.h>
#include <unistd.h>
#include "monitor.h"
#include <time.h>
#include <pthread.h>

struct state state;
int detailed=0;
int silent=0;
pthread_t* WritersIDs;
pthread_t ReadersIDs[MAX_READERS];
int arguments[MAX_READERS];
pthread_mutex_t printmutex;



void sighandler(int n)
{
  exit(EXIT_FAILURE);
}

void onexit(void)
{
  free(WritersIDs);
  free(state.queue);
  pthread_cond_destroy(&state.firstchange);
  pthread_cond_destroy(&state.readchange);
  pthread_mutex_destroy(&state.mutex);
  pthread_mutex_destroy(&printmutex);
}

int isNumber(const char* arg)
{
  for(int i=0;i<strlen(arg);i++) if(arg[i]<'0' || arg[i]>'9') return 0;
  return 1;
}


int areNumbers(const char* arg)
{
  for(int i=0;i<strlen(arg);i++)
  {
    if(arg[i]<'0' || arg[i]>'9')
      if(arg[i] !=' ' && arg[i] !='\t') return 0;
  }
  return 1;
}

void monitor(pthread_cond_t* var, int is_reader)
{
  pthread_mutex_lock(&state.mutex);
  while(!pthread_equal(state.queue[state.first], pthread_self()))
  pthread_cond_wait(var, &state.mutex);
  if(is_reader)
  {
    state.is_read++;
    state.first = (state.first+1)%state.size;
    pthread_cond_broadcast(&state.firstchange);
  }
  pthread_mutex_unlock(&state.mutex);
}

void monitorreading(pthread_cond_t* var)
{
  pthread_mutex_lock(&state.mutex);
  while(state.is_read)
  pthread_cond_wait(var, &state.mutex);
  pthread_mutex_unlock(&state.mutex);
}

void* writer()
{
  int count;
  int ind, value;
  while(1)
  {
    pthread_mutex_lock(&state.mutex);
    state.last = (state.last+1)%state.size;
    state.queue[state.last] = pthread_self();
    pthread_mutex_unlock(&state.mutex);
    monitor(&state.firstchange, 0);
    monitorreading(&state.readchange);
    count = rand()%LENGTH;
    for(int i=0; i<count; i++)
    {
      ind = rand()%LENGTH;
      value = ((double)rand() / RAND_MAX - 0.5) * INT_MAX;
      state.data[ind] = value;
      if(detailed)
      {
        pthread_mutex_lock(&printmutex);
        printf("[%ld] put %d under %d\n", pthread_self(), value, ind);
        fflush(stdout);
        pthread_mutex_unlock(&printmutex);
      }
    }
    if(!silent)
    {
      pthread_mutex_lock(&printmutex);
      printf("[%ld] array modified\n", pthread_self());
      fflush(stdout);
      pthread_mutex_unlock(&printmutex);
    }
    pthread_mutex_lock(&state.mutex);
    state.first = (state.first+1)%state.size;
    pthread_cond_broadcast(&state.firstchange);
    pthread_mutex_unlock(&state.mutex);
  }
}

void* reader(void* mod)
{
  int* n = mod;
  int count;
  while (1) {
    pthread_mutex_lock(&state.mutex);
    state.last = (state.last+1)%state.size;
    state.queue[state.last] = pthread_self();
    pthread_mutex_unlock(&state.mutex);
    monitor(&state.firstchange, 1);
    count = 0;
    for(int i=0;i<LENGTH; i++)
    {
      if(state.data[i]%*n == 0)
      {
        count++;
        if(detailed)
        {
          pthread_mutex_lock(&printmutex);
          printf("[%ld] found %d under %d\n", pthread_self(), state.data[i], i);
          fflush(stdout);
          pthread_mutex_unlock(&printmutex);
        }
      }
    }
    if(!silent)
    {
      pthread_mutex_lock(&printmutex);
      printf("[%ld] found %d numbers dividible by %d\n", pthread_self(), count, *n);
      fflush(stdout);
      pthread_mutex_unlock(&printmutex);
    }
    pthread_mutex_lock(&state.mutex);
    state.is_read--;
    pthread_cond_broadcast(&state.readchange);
    pthread_mutex_unlock(&state.mutex);
  }
}


void printusage(const char * name)
{
  printf("Usage:\t %s [-i] [-s] -n N -m MODS\n\n", name);
  printf("\tN - number of writers\n");
  printf("\tMODS - arguments for readers, like \"12 45 11\", including quotes\n");
  printf("\ti - detailed mode\n");
  printf("\ts - silent mode\n");
  exit(0);
}

int main(int argc, char** argv) {
  if(argc<5) printusage(argv[0]);
  int c;
  int nflag = 0, mflag = 0;
  int writers;
  int readers = 0;
  char* arg;
  char* forreaders;
  srand(time(NULL));
  while ((c = getopt (argc, argv, "isn:m:")) != -1)
    switch (c)
      {
        case 's':
          silent = 1;
          break;
        case 'i':
          detailed = 1;
          break;
        case 'n':
          if(!isNumber(optarg)) printusage(argv[0]);
          writers = atoi(optarg);
          nflag = 1;
          break;
        case 'm':
          if(!areNumbers(optarg)) printusage(argv[0]);
          mflag = 1;
          forreaders = optarg;
          break;
        case '?':
          if (optopt == 'n' || optopt == 'm')
            {
              fprintf (stderr, "Option -%c requires an argument.\n", optopt);
              printusage(argv[0]);
            }
          // else if (isprint (optopt))
          //   {
          //     fprintf (stderr, "Unknown option `-%c'.\n", optopt);
          //     printusage(argv[0]);
          //   }
          else
            {
              fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
              printusage(argv[0]);
            }
          return 1;
        default:
          abort ();
      }

  if(!nflag || !mflag) printusage(argv[0]);

  for (int index = optind; index < argc; index++)
  {
    printf ("Non-option argument %s\n", argv[index]);
    printusage(argv[0]);
  }

  for(int i=0; i<LENGTH;i++) state.data[i]=0;
  state.is_read = 0;
  state.first = 0;
  state.last = -1;
  pthread_mutex_init(&state.mutex, NULL);
  pthread_mutex_init(&printmutex, NULL);
  WritersIDs = malloc(writers * sizeof(pthread_t));
  pthread_cond_init(&state.firstchange, NULL);
  pthread_cond_init(&state.readchange, NULL);
  atexit(onexit);
  signal(SIGINT, sighandler);
  pthread_mutex_lock(&state.mutex);
  for(int i=0; i<writers; i++)
    pthread_create(&WritersIDs[i], NULL, &writer, NULL);
  if((arg=strtok(forreaders, " \t\r\n\0"))!=NULL && readers<MAX_READERS - 1)
  {
    arguments[readers] = atoi(arg);
    pthread_create(&ReadersIDs[readers], NULL, &reader, &arguments[readers]);
    readers++;
    while (readers<MAX_READERS - 1 && (arg=strtok(NULL, " \t\r\n\0"))!=NULL)
    {
      arguments[readers] = atoi(arg);
      pthread_create(&ReadersIDs[readers], NULL, &reader, &arguments[readers]);
      readers++;
    }
  }
  state.size = readers + writers;
  state.queue = malloc(state.size * sizeof(pthread_t));
  pthread_mutex_unlock(&state.mutex);

  for(int i=0; i<writers; i++) pthread_join(WritersIDs[i], NULL);
  for(int i=0; i<readers; i++) pthread_join(ReadersIDs[i], NULL);

  return 0;
}
