#include <stdio.h>

int jopa()
{
	return 2;
}

int main()
{
	printf("hi sashsa");

	asm volatile("mov r15, r14" : : :);

	return jopa();
}
