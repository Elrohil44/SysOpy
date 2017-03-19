#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int generate(int x, int y)
{
	int source = open("/dev/urandom", O_RDONLY);
	if(source<0) return -1;
	int target = open("generated",O_WRONLY | O_CREAT, S_IRWXU);
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


int main(int argc, char* argv[])
{
	generate(1024,6);
	return 0;
}
