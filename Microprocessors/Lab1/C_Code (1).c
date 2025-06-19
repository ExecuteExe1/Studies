
/* M&P Lab 1 Assignment: String Hash In Assembly */

#include <stdio.h>

extern int strhash(char* str);
extern int digitsum(int number);
extern int mod7(int number);
extern int fibonacci(int number);

int main(void)
{
	char* str = "A9b3";
	printf("String: %s\nInitial Hash: %d\nDigit Sum: %d\nMod 7: %d\nFibonacci: %d", str, strhash(str), digitsum(strhash(str)), mod7(digitsum(strhash(str))), fibonacci(mod7(digitsum(strhash(str)))));
	return 0;
}