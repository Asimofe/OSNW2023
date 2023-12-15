#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

int main()
{
  int pid;
  int i = 0;

  printf("process start  %d\n", getpid());
  pid = fork();

  if (pid > 0)
  {
    printf("parent process pid(%d)\n", getpid());
    sleep(5);
    exit(0);
  }
  else if (pid == 0)
  {
    printf("child process pid(%d) of parent pid(%d)\n", getpid(), getppid());
    close(0); close(1); close(2);
    setsid();
    sleep(10);
    printf("I'm daemon\n");
    i = 1000;
    while(1)
    { 
      printf("child : %d\n", i);
      i++; 
      sleep(2);
    }
  }
  return 1;
}
