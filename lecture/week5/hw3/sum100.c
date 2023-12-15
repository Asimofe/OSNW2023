#include <stdio.h>

void sum100(){
	int i = 1;
	int sum = 0;

	for(i = 1; i <= 100; i++){
		sum += i;
	}

	printf("%d\n",sum);
}
