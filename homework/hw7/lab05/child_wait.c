#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
        int pid;
        int pstatus;

        printf("process start %d\n", getpid());
        pid = fork();

        if (pid > 0) {
                printf("parent process pid(%d)\n", getpid());
                printf("wait for child(%d) to die\n", pid);
                printf("hit a key"); getchar(); printf("\n");
                pid = wait(&pstatus);
                printf("=========================\n");
                printf("terminated child process : %d\n", pid);
                printf("return value from child  : %d\n", pstatus/256);
        } else if (pid == 0) {
                sleep(2);
                printf("I'm Zombie %d\n", getpid());
                exit(100);
        }

        return 1;
}
