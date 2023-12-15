#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	int shmid;

	int *cal_num;
	void *shared_memory = NULL;

	shmid = shmget((key_t)1234, sizeof(int), 0);
	if (shmid == -1)
	{
		perror("shmget failed : ");
		exit(0);
	}

	shared_memory = shmat(shmid, NULL, 0);
	if (shared_memory == (void *)-1)
	{
		perror("shmat failed : ");
		exit(0);
	}

	cal_num = (int *)shared_memory;
	while(1)
	{
		int local_var;
		local_var = *cal_num; 
		local_var++;
		sleep(1);
		*cal_num = local_var;
		printf("Consumer : %d\n", *cal_num);
	}
	return 1;
}

