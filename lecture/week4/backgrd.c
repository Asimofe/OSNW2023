#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

void main(){
	int i = 0;
	time_t t;
	struct tm *tm;

	while(i<100){
		sleep(1);
		i++;
		t = time(NULL);
		tm = localtime(&t);
		printf("%d form background job at %s\n", i, asctime(tm));
	}
}
