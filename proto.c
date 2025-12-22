#include<stdio.h>

int A (int);
int B (int);
int C (int);

int main () {
	A (B(C(1)));
}

int A (int dummy) {
	printf("This is A\n");
	return dummy;
}

int B (int dummy) {
	printf("This is B\n");
	return dummy;
}

int C (int dummy) {
	printf("This is C\n");
	return dummy;
}
