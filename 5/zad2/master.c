#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define BUF_SIZE 50
int isNumber(const char* arg)
{
  for(int i=0;i<strlen(arg);i++) if(arg[i]<'0' || arg[i]>'9') return 0;
  return 1;
}


int main(int argc, char const *argv[]) {
  if(argc != 3 || !isNumber(argv[2])) exit(EXIT_FAILURE);
  int R = atoi(argv[2]);
  int** T = malloc(R * sizeof(int*));
  char K[]="100";
  char N[]="100000";
  char buf[BUF_SIZE] = "";
  int slavesc = 100;
  double x, y;
  int iters;
  for (int i=0; i<R; i++)
  {
    T[i] = malloc(R * sizeof(int));
    for(int j=0; j<R; j++) T[i][j]=0;
  }


  mkfifo(argv[1], 0600);
  for(int i=0; i<slavesc; i++)
  {
    if(fork() == 0)
    {
      execlp("./slave", "./slave", argv[1], N, K, NULL);
      exit(EXIT_SUCCESS);
    }
  }

  int pipe = open(argv[1], O_RDONLY);
  while(read(pipe, buf, BUF_SIZE))
  {
      sscanf(buf, " %lf %lf %d ", &x, &y, &iters);
      T[((int)((x+2)/3 * R))%R][((int)((y+1)/2*R))%R] = iters;
  }
  close(pipe);
  FILE* file = fopen("data", "w");
  for(int i=0; i<R; i++) for(int j=0; j<R;j++)
  {
    fprintf(file, "%d %d %d\n", i, j, T[i][j]);
  }
  fclose(file);
  for (int i=0; i<R; i++) free(T[i]);
  free(T);
  FILE* stream = popen("gnuplot", "w");
  fprintf(stream, "set view map\n");
  fprintf(stream, "set xrange [0:%d]\n", R);
  fprintf(stream, "set yrange [0:%d]\n", R);
  fprintf(stream, "plot 'data' with image\n");
  fflush(stream);
  getc(stdin);
  pclose(stream);
  return 0;
}
