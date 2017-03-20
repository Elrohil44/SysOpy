
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "bench.h"
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

const char* RANDOM_PATH = "/dev/urandom";

enum Library {LIB, SYS};
typedef enum Library Library;

enum Function {GENERATE, SHUFFLE, SORT};
typedef enum Function Function;

struct Todo
{
  Function func;
  Library lib;
  int count;
  int length;
  const char* filename;
};

typedef struct Todo Todo;

void printerror(const char* errormsg, const char* filename)
{
  char* error = strerror(errno);
  fprintf((stderr), "%s: %s; %s\n",errormsg,filename,error);
  exit(EXIT_FAILURE);
}

int generate(int x, int y, const char* name)
{
    int source = open(RANDOM_PATH, O_RDONLY);
    if(source<0) printerror("Error encountered while opening file",RANDOM_PATH);
    int target = open(name,O_WRONLY | O_CREAT, S_IRWXU);
    if(target<0) printerror("Error encountered while opening file",name);
    char* record= malloc(y);
    int curr;
    for(int i=0;i<x;i++)
    {
        size_t recordLen = 0;
        while(recordLen < y)
        {
            curr = read(source, record+recordLen, y - recordLen);
            if(curr==-1) printerror("Error encountered while reading from",RANDOM_PATH);
            recordLen += curr;
        }
        curr = write(target,record,y);
        if (curr==-1) printerror("Error encountered while writing to",name);
        if (curr<y) fprintf(stderr, "Error encountered while writing to %s\n", name);
    }
    free(record);
    if(close(target)==-1) printerror("Error encountered while closing",name);
    if(close(source)==-1) printerror("Error encountered while closing",RANDOM_PATH);
    return 0;
}

int shuffle(int x, int y, const char* name, Library type)
{
    if(type!=LIB && type!=SYS) return -2;
    srand(time(NULL));
    char* from = malloc(y);
    char* to = malloc(y);
    int random;
    if (type == SYS)
    {
        int file = open(name,O_RDWR);
        if (file<0) printerror("Error encountered while opening file",name);
        for(int i=x;i>1;i--)
        {
            random = rand()%i;
            if(random!=i-1)
            {
                if(lseek(file,random*y,SEEK_SET)==-1) printerror("Error encountered while setting position",name);
                if(read(file,from,y)<y) printerror("Error encountered while reading",name);
                if(lseek(file,(i-1)*y,SEEK_SET)==-1) printerror("Error encountered while setting position",name);
                if(read(file,to,y)<y) printerror("Error encountered while reading",name);
                if(lseek(file,-y,SEEK_CUR)==-1) printerror("Error encountered while setting position",name);
                if(write(file,from,y)<y) printerror("Error encountered while writing to",name);
                if(lseek(file,random*y,SEEK_SET)==-1) printerror("Error encountered while setting position",name);
                if(write(file,to,y)<y) printerror("Error encountered while writing to",name);
            }
        }
        if(close(file)==-1) printerror("Error encountered while closing",name);
    }
    else
    {
        FILE* file = fopen(name,"r+");
        if(file==NULL) printerror("Error encountered while opening",name);
        for(int i=x;i>1;i--)
        {
            random = rand()%i;
            if(random!=i-1)
            {
                if(fseek(file,random*y,SEEK_SET)==-1) printerror("Error encountered while setting position",name);
                if(fread(from,1,y,file)<y) printerror("Error encountered while reading from",name);
                if(fseek(file,(i-1)*y,SEEK_SET)==-1) printerror("Error encountered while setting position",name);
                if(fread(to,1,y,file)<y) printerror("Error encountered while reading from",name);
                if(fseek(file,-y,SEEK_CUR)==-1) printerror("Error encountered while setting position",name);
                if(fwrite(from,1,y,file)<y) printerror("Error encountered while writing to",name);
                if(fseek(file,random*y,SEEK_SET)==-1) printerror("Error encountered while setting position",name);
                if(fwrite(to,1,y,file)<y) printerror("Error encountered while writing to",name);
	    }
	}
	if(fclose(file)!=0) printerror("Error encountered while closing",name);
    }
    free(from);
    free(to);
    return 0;
}

void swap(char** a,char** b)
{
    char* tmp;
    tmp = *a;
    *a = *b;
    *b = tmp;
}

void swap_recordss(int x, int y,int nr1,char* from,int nr2, char* to, int file, const char* name)
{
    if(nr1==nr2) return;
    if(lseek(file,nr1*y,SEEK_SET)==-1) printerror("Error encountered while setting position in",name);;
    if(write(file,to,y)<y) printerror("Error encountered while writing to",name);
    if(lseek(file,nr2*y,SEEK_SET)==-1) printerror("Error encountered while setting position in",name);
    if(write(file,from,y)<y) printerror("Error encountered while writing to",name);
}

void swap_recordsl(int x, int y,int nr1,char* from,int nr2, char* to, FILE* file, const  char* name)
{
    if(nr1==nr2) return;
    if(fseek(file,nr1*y,SEEK_SET)==-1) printerror("Error encountered while setting position in",name);
    if(fwrite(to,1,y,file)<y) printerror("Error encountered while writing to",name);
    if(fseek(file,nr2*y,SEEK_SET)==-1) printerror("Error encountered while setting position in",name);
    if(fwrite(from,1,y,file)<y) printerror("Error encountered while writing to",name);
}


void sort(int x, int y, const char* name, Library type)
{
    if(type!=LIB && type!=SYS) return;
    char* from = malloc(y);
    char* to = malloc(y);
    int sorted=0;
    if(type==SYS)
    {
        int file = open(name,O_RDWR);
        if (file==-1) printerror("Error encountered while opening",name);
        for(int i=0;i<x && !sorted;i++)
        {
            sorted = 1;
            if(lseek(file,(x-1)*y,SEEK_SET)==-1) printerror("Error encountered while setting position in",name);
            if(read(file,from,y)<y) printerror("Error encountered while reading",name);
            for(int j=x-1;j>i;j--)
            {
                if(lseek(file,(j-1)*y,SEEK_SET)==-1) printerror("Error encountered while setting position in",name);
                if(read(file,to,y)<y) printerror("Error encountered while reading",name);
                if(*from<*to)
                {
                    sorted = 0;
                    swap_recordss(x,y,j,from,j-1,to,file,name);
                }
                else
                {
                    swap(&from,&to);
                }
            }
        }
        close(file);
    }
    else
    {
        FILE* file = fopen(name,"r+");
        if(file==NULL) printerror("Error encountered while opening",name);
        for(int i=0;i<x && !sorted;i++)
        {
            sorted = 1;
            if(fseek(file,(x-1)*y,SEEK_SET)==-1) printerror("Error encountered while setting position in",name);
            if(fread(from,1,y,file)<y) printerror("Error encountered while reading from",name);
            for(int j=x-1;j>i;j--)
            {
                if(fseek(file,(j-1)*y,SEEK_SET)==-1) printerror("Error encountered while setting position in",name);
                if(fread(to,1,y,file)<y) printerror("Error encountered while reading from",name);
                if(*from<*to)
                {
                    sorted = 0;
                    swap_recordsl(x,y,j,from,j-1,to,file,name);
                }
                else
                {
                    swap(&from,&to);
                }
            }
       }
       if(fclose(file)!=0) printerror("Error encountered while closing",name);

    }
	free(from);
	free(to);
}

void usage(const char* program)
{
  printf("Usage: %s PATH Library Function count length\n",program);
  printf("\tPATH - path to a file\n");
  printf("\tLibrary <- LIB or SYS\n");
  printf("\tFunction <- GENERATE or SHUFFLE or SORT\n");
  printf("\tcount - count of records\n");
  printf("\tlength - length of a single record in bytes\n\n");
  printf("Alternate usage: %s PATH GENERATE count length\n",program);
}

int isNumber(const char* arg)
{
  for(int i=0;i<strlen(arg);i++) if(arg[i]<'0' || arg[i]>'9') return 0;
  return 1;
}

Todo* argparse(int argc, char* argv[])
{
  Todo* action=NULL;
  if(argc<5)
  {
    usage(argv[0]);
    exit(EXIT_FAILURE);
  }
  else if(argc==5)
  {
    if(strcmp(argv[2],"GENERATE")!=0) usage(argv[0]);
    else if(!isNumber(argv[3]) || !isNumber(argv[4]))
    {
      usage(argv[0]);
      exit(EXIT_FAILURE);
    }
    else
    {
      action = malloc(sizeof(Todo));
      action->filename = argv[1];
      action->func = GENERATE;
      action->count = atoi(argv[3]);
      action->length = atoi(argv[4]);
    }
  }
  else
  {
    action = malloc(sizeof(Todo));
    action->filename=argv[1];
    if(strcmp(argv[2],"LIB")==0) action->lib=LIB;
    else if(strcmp(argv[2],"SYS")==0) action->lib=SYS;
    else
    {
      usage(argv[0]);
      exit(EXIT_FAILURE);
    }
    if(strcmp(argv[3],"GENERATE")==0) action->func=GENERATE;
    else if(strcmp(argv[3],"SHUFFLE")==0) action->func=SHUFFLE;
    else if(strcmp(argv[3],"SORT")==0) action->func=SORT;
    else
    {
      usage(argv[0]);
      exit(EXIT_FAILURE);
    }
    if(isNumber(argv[4])) action->count = atoi(argv[4]);
    else
    {
      usage(argv[0]);
      exit(EXIT_FAILURE);
    }
    if(isNumber(argv[5])) action->length = atoi(argv[5]);
    else
    {
      usage(argv[0]);
      exit(EXIT_FAILURE);
    }
    if(argc>6) printf("Unecessary arguments\n");
  }
  return action;
}


int main(int argc, char* argv[])
{

    Todo* action = argparse(argc, argv);
    struct rusage time_l;
    struct rusage time_a;
    if(action!=NULL)
    {
      for(int i=0;i<argc && i<6;i++)
      {
        printf("%s ",argv[i]);
      }
      printf("\n");
      time_l=getTime();
      switch(action->func)
      {
        case GENERATE:
          generate(action->count,action->length,action->filename);
          break;
        case SHUFFLE:
          shuffle(action->count,action->length,action->filename,action->lib);
          break;
        case SORT:
          sort(action->count,action->length,action->filename,action->lib);
          break;
        default:
          break;
      }
      time_a=getTime();
      printtimes(time_l,time_a);
    }
    return 0;
}
