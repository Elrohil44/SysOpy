#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>


enum Library {LIB, SYS};
typedef enum Library Library;

int generate(int x, int y, const char* name)
{
    int source = open("/dev/urandom", O_RDONLY);
    if(source<0) return -1;
    int target = open(name,O_WRONLY | O_CREAT, S_IRWXU);
    if(target<0) return -2;
    char* record= malloc(y);
    for(int i=0;i<x;i++)
    {
        size_t recordLen = 0;
        while(recordLen < y)
        {
            recordLen += read(source, record+recordLen, y - recordLen);
        }
        write(target,record,y);

    }
    free(record);
    close(target);
    close(source);
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
        if (file<0) return -1;
        for(int i=x;i>1;i--)
        {
            random = rand()%i;
            if(random!=i-1)
            {	
                lseek(file,random*y,SEEK_SET);
                read(file,from,y);
                lseek(file,(i-1)*y,SEEK_SET);
                read(file,to,y);
                lseek(file,-y,SEEK_CUR);
                write(file,from,y);
                lseek(file,random*y,SEEK_SET);
                write(file,to,y);
            }
        }
        close(file);
    }
    else
    {
        FILE* file = fopen(name,"r+");
        if(file==NULL) return -1;
        for(int i=x;i>1;i--)
        {
            random = rand()%i;
            if(random!=i-1)
            {
                fseek(file,random*y,SEEK_SET);
                fread(from,1,y,file);
                fseek(file,(i-1)*y,SEEK_SET);
                fread(to,1,y,file);
                fseek(file,-y,SEEK_CUR);
                fwrite(from,1,y,file);
                fseek(file,random*y,SEEK_SET);
                fwrite(to,1,y,file);
	    }
	}
	fclose(file);
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

void swap_recordss(int x, int y,int nr1,char* from,int nr2, char* to, int file)
{
    if(nr1==nr2) return;
    lseek(file,nr1*y,SEEK_SET);
    write(file,to,y);
    lseek(file,nr2*y,SEEK_SET);
    write(file,from,y);
}

void swap_recordsl(int x, int y,int nr1,char* from,int nr2, char* to, FILE* file)
{
    if(nr1==nr2) return;
    fseek(file,nr1*y,SEEK_SET);
    fwrite(to,1,y,file);
    fseek(file,nr2*y,SEEK_SET);
    fwrite(from,1,y,file);
}


void sort(int x, int y, const char* name, Library type)
{
    if(type!=LIB && type!=SYS) return -2;
    char* from = malloc(y);
    char* to = malloc(y);
    int sorted=0;
    if(type==SYS)
    {
        int file = open(name,O_RDWR);
        for(int i=0;i<x && !sorted;i++)
        {
            sorted = 1;
            lseek(file,(x-1)*y,SEEK_SET);
            read(file,from,y);
            for(int j=x-1;j>i;j--)
            {
                lseek(file,(j-1)*y,SEEK_SET);
                read(file,to,y);
                if(*from<*to)
                {
                    sorted = 0;
                    swap_recordss(x,y,j,from,j-1,to,file);
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
        for(int i=0;i<x && !sorted;i++)
        {
            sorted = 1;
            fseek(file,(x-1)*y,SEEK_SET);
            printf("%d\n",fread(from,1,y,file));
            for(int j=x-1;j>i;j--)
            {
                fseek(file,(j-1)*y,SEEK_SET);
                fread(to,1,y,file);
                if(*from<*to)
                {
                    sorted = 0;
                    swap_recordsl(x,y,j,from,j-1,to,file);
                }
                else
                {
                    swap(&from,&to);
                }
            }
       }
       fclose(file);

    }
	free(from);
	free(to);
}

int main(int argc, char* argv[])
{
    shuffle(5,5,"aa",SYS);
    int d;
    scanf("%d",&d);
    sort(5,5,"aa",LIB);
    scanf("%d",&d);
    shuffle(5,5,"aa",LIB);
    return 0;
}
