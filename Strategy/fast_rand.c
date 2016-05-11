#include <stdlib.h>
#include <stdio.h>

int fast_rand(int x) {
	int ret = rand() % x;
	return ret;
}
