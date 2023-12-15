#include <unistd.h>
#include <stdio.h>
#include <string.h>
	 
int main(int argc, char **argv)
{
	printf("-- START -- %d\n", getpid());
	printf("hit a key "); getchar(); printf("\n");
	execl("./exec_test","exec_test", NULL);
	printf("-- END - NOT EXECUTE --\n");
	return 1;
}
