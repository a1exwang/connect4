#include <stdlib.h>
#include <stdio.h>

static unsigned x = 123456789, y = 362436069, z = 521288629;

static unsigned shabbyRandom() {
	unsigned t;
	x ^= x << 16;
	x ^= x >> 5;
	x ^= x << 1;

	t = x;
	x = y;
	y = z;
	z = t ^ x ^ y;

	return z;
}

int fast_rand(int x) {
	int ret = shabbyRandom() % x;
	return ret;
}
