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
sem_t printsem;



void sighandler(int n)
{
  exit(EXIT_FAILURE);
}

void onexit(void)
{
  free(WritersIDs);
  sem_destroy(&state.sem);
  sem_destroy(&state.semread);
  sem_destroy(&printsem);
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

void* writer()
{
  int count;
  int ind, value;
  while(1)
  {
    sem_wait(&state.sem);
    // sem_wait(&printsem);
    // fprintf(stderr, "{%ld} Waiting, there are %d readers\n", pthread_self(), state.is_read);
    // fflush(stdout);
    // sem_post(&printsem);
    sem_wait(&state.semread);
    if(state.is_read)
    {
      sem_post(&state.semread);
      sem_post(&state.sem);
      continue;
    }
    sem_post(&state.semread);
    count = rand()%LENGTH;
    for(int i=0; i<count; i++)
    {
      ind = rand()%LENGTH;
      value = ((double)rand() / RAND_MAX - 0.5) * INT_MAX;
      state.data[ind] = value;
      if(detailed)
      {
        sem_wait(&printsem);
        printf("[%ld] put %d under %d\n", pthread_self(), value, ind);
        fflush(stdout);
        sem_post(&printsem);
      }
    }
    if(!silent)
    {
      sem_wait(&printsem);
      printf("[%ld] array modified\n", pthread_self());
      fflush(stdout);
      sem_post(&printsem);
    }
    sem_post(&state.sem);
  }
}

void* reader(void* mod)
{
  int* n = mod;
  int count;
  while (1) {
    sem_wait(&state.sem);
    sem_wait(&state.semread);
    state.is_read++;
    sem_post(&state.semread);
    sem_post(&state.sem);
    count = 0;
    for(int i=0;i<LENGTH; i++)
    {
      if(state.data[i]%*n == 0)
      {
        count++;
        if(detailed)
        {
          sem_wait(&printsem);
          printf("[%ld] found %d under %d\n", pthread_self(), state.data[i], i);
          fflush(stdout);
          sem_post(&printsem);
        }
      }
    }
    if(!silent)
    {
      sem_wait(&printsem);
      printf("[%ld] found %d numbers dividible by %d\n", pthread_self(), count, *n);
      fflush(stdout);
      sem_post(&printsem);
    }
    sem_wait(&state.semread);
    state.is_read--;
    sem_post(&state.semread);
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
  sem_init(&state.sem, 0, 1);
  sem_init(&state.semread, 0, 1);
  sem_init(&printsem, 0, 1);
  WritersIDs = malloc(writers * sizeof(pthread_t));
  atexit(onexit);
  signal(SIGINT, sighandler);
  sem_wait(&state.sem);
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
  sem_post(&state.sem);

  for(int i=0; i<writers; i++) pthread_join(WritersIDs[i], NULL);
  for(int i=0; i<readers; i++) pthread_join(ReadersIDs[i], NULL);

  return 0;
}
