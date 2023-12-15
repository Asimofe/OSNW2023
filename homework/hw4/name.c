#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

void main(){
	
	time_t t;
	struct tm *tm;

	sleep(1);
	t = time(NULL);
	tm = localtime(&t);
	printf("Lee cheol min %s\n", asctime(tm));
	
}
