#include <stdio.h>

int g;

void sub(int p){
	int i = 1000;
	static int s = 100;
	register int r;
	
	printf("s: %d, i: %d, p: %d\n", s, i, p);

	
	s++; i++;
}

int l;

void main(){
	int i;
	sub(100);
	sub(101);
}
