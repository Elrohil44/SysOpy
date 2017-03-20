#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>


void usage(const char* name)
{
  printf("Usage: %s FILEPATH\n",name);
  printf("\tFILEPATH - path to a file\n");
  exit(0);
}

void set_read_lock(int fd)
{
  int nr;
  int wait;
  printf("Byte number:\t ");
  scanf("%d",&nr);
  printf("Wait mode [0 -> OFF]:\t");
  scanf("%d",&wait);
  struct flock lock;
  lock.l_type = F_RDLCK;
  lock.l_whence = SEEK_SET;
  lock.l_start = nr;
  lock.l_len = 1;
  if(wait && fcntl(fd,F_SETLKW,&lock)==-1)
  {
    fprintf(stderr, "Error while setting read lock\n");
  } else if (!wait && fcntl(fd,F_SETLK,&lock)==-1)
  {
    fprintf(stderr, "Error while setting read lock\n");
  }
}
void set_write_lock(int fd)
{
  int nr;
  int wait;
  printf("Byte number:\t ");
  scanf("%d",&nr);
  printf("Wait mode [0 -> OFF]:\t ");
  scanf("%d",&wait);
  struct flock lock;
  lock.l_type = F_WRLCK;
  lock.l_whence = SEEK_SET;
  lock.l_start = nr;
  lock.l_len = 1;
  if(wait && fcntl(fd,F_SETLKW,&lock)==-1)
  {
    fprintf(stderr, "Error while setting write lock\n");
  } else if (!wait && fcntl(fd,F_SETLK,&lock)==-1)
  {
    fprintf(stderr, "Error while setting write lock\n");
  }
}
void list_locks(int fd)
{
  printf("Byte\tLock type\tPID\n");
  long eof;
  if((eof=lseek(fd,0,SEEK_END))==-1)
  {
    fprintf(stderr, "Error occured while checking eof\n");
    return;
  }
  long indicator = 0;

  struct flock lock;
  while(indicator != eof)
  {
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = indicator;
    lock.l_len = 1;
    fcntl(fd,F_GETLK,&lock);
    if(lock.l_type == F_WRLCK) printf("%ld\tWRITE",indicator);
    else if(lock.l_type == F_RDLCK) printf("%ld\tREAD",indicator);
    if(lock.l_type!=F_UNLCK) printf("\t\t%d\n",lock.l_pid);
    indicator++;
  }
}
void unlock(int fd)
{
  int nr;
  printf("Byte number:\t ");
  scanf("%d",&nr);
  struct flock lock;
  lock.l_type = F_UNLCK;
  lock.l_whence = SEEK_SET;
  lock.l_start = nr;
  lock.l_len = 1;
  if(fcntl(fd,F_SETLK,&lock)==-1)
  {
    fprintf(stderr, "Error while releasing lock\n");
  }
}
void read_byte(int fd)
{
  int nr;
  char c;
  printf("Byte number:\t ");
  scanf("%d",&nr);
  if(lseek(fd,nr,SEEK_SET)==-1)
  {
    fprintf(stderr, "Error while setting position\n");
  }
  else
  {
    int curr = read(fd,&c,1);
    if(curr<1)
    {
      fprintf(stderr, "Error while reading from file\n");
    }
    else printf("%c\n", c);
  }
}
void change_byte(int fd)
{
  int nr;
  char c;
  printf("Byte number:\t ");
  scanf("%d",&nr);
  printf("New byte:\t ");
  scanf(" %c",&c);
  if(lseek(fd,nr,SEEK_SET)==-1)
  {
    fprintf(stderr, "Error while setting position\n");
  }
  else
  {
    int curr = write(fd,&c,1);
    if(curr<1)
    {
      fprintf(stderr, "Error while writing to file\n");
    }
  }
}




void menu()
{
  printf("\tSet read lock on byte\t-\t1\n");
  printf("\tSet write lock on byte\t-\t2\n");
  printf("\tList byte locks\t-\t3\n");
  printf("\tUnlock byte\t-\t4\n");
  printf("\tRead byte\t-\t5\n");
  printf("\tChange byte\t-\t6\n");
  printf("\tExit\t-\tq\n");
}

int main(int argc, const char* argv[])
{
  if(argc<2) usage(argv[0]);
  if(argc>2) printf("Unnecessary arguments\n");
  int fd = open(argv[1],O_RDWR);
  if(fd==-1)
  {
    fprintf(stderr, "Error opening file: %s\n", argv[1]);
  }
  int flag = 1;
  char key;
  while(flag)
  {
    printf("\n#############################################\n\n");
    menu();
    printf("\n\t\tChoose option: \t");
    scanf(" %c",&key);
    printf("#############################################\n\n");
    switch(key)
    {
      case '1':
        set_read_lock(fd);
        break;
      case '2':
        set_write_lock(fd);
        break;
      case '3':
        list_locks(fd);
        break;
      case '4':
        unlock(fd);
        break;
      case '5':
        read_byte(fd);
        break;
      case '6':
        change_byte(fd);
        break;
      case 'q':
        flag = 0;
        break;
      default:
        break;
    }
  }
  if(close(fd)==-1)
  {
    fprintf(stderr, "Error while closing file\n");
  }
}
