#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	pid_t pid;
	int i = 100;

	printf("Program Start!!\n");

	printf("hit a key "); getchar(); printf("\n");
	pid = fork();
	printf("fork !!!\n");
	if (pid < 0)
	{
		printf("fork failure\n");
		return 1;
	}

	if (pid > 0)
	{
		printf("I'm parent Process %d\n", getpid());
		while(1)
		{
			printf("P : %d\n", i);
			i++;
			sleep(1);
		}
	}
	else if (pid == 0)
	{
		printf("I'm Child Process %d\n", getpid());
		while(1)
		{
			printf("C : %d\n", i);
			i+=2;
			sleep(1);
		}
	}
}
