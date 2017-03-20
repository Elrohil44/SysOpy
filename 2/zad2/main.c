#define _XOPEN_SOURCE 500
#define _BSD_SOURCE
#include <ftw.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>


size_t maxsize;

void printdata(const struct stat *sb, const char* path)
{



  printf( (sb->st_mode & S_IRUSR) ? "r" : "-");
  printf( (sb->st_mode & S_IWUSR) ? "w" : "-");
  printf( (sb->st_mode & S_IXUSR) ? "x" : "-");
  printf( (sb->st_mode & S_IRGRP) ? "r" : "-");
  printf( (sb->st_mode & S_IWGRP) ? "w" : "-");
  printf( (sb->st_mode & S_IXGRP) ? "x" : "-");
  printf( (sb->st_mode & S_IROTH) ? "r" : "-");
  printf( (sb->st_mode & S_IWOTH) ? "w" : "-");
  printf( (sb->st_mode & S_IXOTH) ? "x" : "-");
  printf("\t%jd",(long)(sb->st_size));
  printf("\t%.24s",asctime(localtime(&sb->st_mtime)));
  printf("\t%s\n",path);
}

int fn(const char* fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
  if(S_ISREG(sb->st_mode)!=0 && sb->st_size<=maxsize)
  {
    printdata(sb,fpath);
  }
  return 0;
}

void using_nftw(const char* path)
{
  if(nftw(path,fn,25,FTW_PHYS)==-1)
  {
    fprintf(stderr, "Error occured in nftw\n");
    exit(EXIT_FAILURE);
  }
}

void using_manual(const char* pathh)
{
  DIR* dir = opendir(pathh);
  if(dir==NULL)
  {
    fprintf(stderr, "Error opening: %s\n", pathh);
    return;
  }
  char filepath[PATH_MAX];
  char path[PATH_MAX];
  strcpy(path,pathh);
  strcat(path,"/");
  struct dirent* next;
  while((next=readdir(dir))!=NULL)
  {
    strcpy(filepath,path);
    if(strcmp(next->d_name,"..")==0 || strcmp(next->d_name,".")==0) continue;
    else if (next->d_type==DT_DIR) 
    {
      using_manual(strcat(filepath,next->d_name));
    }
    else
    {
      struct stat data;
      stat(strcat(filepath,next->d_name),&data);
      if(S_ISREG(data.st_mode)!=0 && data.st_size<=maxsize) printdata(&data,filepath);
    }
  }
  if(closedir(dir)==-1) fprintf(stderr, "Error while closing: %s\n",path);
}

void usage(const char* name)
{
  printf("Usage: %s FILEPATH SIZE MODE\n",name);
  printf("\tFILEPATH - path to a directory\n");
  printf("\tSIZE - max size of regular file in bytes\n");
  printf("\tMODE -\t 1 -> with opendir() etc.\n");
  printf("\t\t 2 -> with nftw()\n");
  exit(0);
}

int isNumber(const char* arg)
{
  for(int i=0;i<strlen(arg);i++) if(arg[i]<'0' || arg[i]>'9') return 0;
  return 1;
}

int main(int argc, const char* argv[])
{
  if(argc<4) usage(argv[0]);
  else
  {
    char filepath[PATH_MAX];
    realpath(argv[1],filepath);
    if(!isNumber(argv[2])) usage(argv[0]);
    maxsize = atoi(argv[2]);
    if(strcmp(argv[3],"1")==0) using_manual(filepath);
    else if (strcmp(argv[3],"2")==0) using_nftw(filepath);
    else usage(argv[0]);
    if (argc>4) printf("Unnecessary arguments\n");
  }
  return 0;
}
