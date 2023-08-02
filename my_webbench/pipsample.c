#include <unistd.h>
#include <sys/param.h>
#include <rpc/types.h>
#include <getopt.h>
#include <strings.h>
#include <time.h>
#include <signal.h>
#include <stdio.h>

int main(void)
{
  int mypipe[2];
  char buf[10];
  char wbuf[] = "hello";
  int a = 1, b = 2, c = 3;
  int i, j , k;
  pid_t pid;
  FILE *f;
  if(pipe(mypipe))
  {
    perror("cannot do pipe function\n");
  }
  
  pid = fork();
  if(pid == 0)
  {
    close(mypipe[0]);
    f = fdopen(mypipe[1], "w");
    if(f == NULL)
    {
      perror("fd open failed\n");
    }
    fprintf(f, "%d %d %d", a,b,c);
    fclose(f);
  }
  else if(pid > 0)
  {
     close(mypipe[1]);
     f = fdopen(mypipe[0], "r");
     if(f == NULL)
     {
       perror("fd open read failed\n");
     }
     fscanf(f, "%d %d %d", &i,&j,&k);
     fclose(f);
     printf("a b c = %d %d %d", a,b,c);
  }
  return 0;
}
