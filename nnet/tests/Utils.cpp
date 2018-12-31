#include "Tests.h"
#include <cmath>
#include <cstdio>
#include <exception>

using namespace std;

void Tests::ReadFloatsToBuffer(const char *path, float *buffer, int size) {
	FILE *file = fopen(path, "rb");
	if (!file) {
		printf("cannot open: %s\n", path);
		throw new exception("cannot open file.");
	}
	fread(buffer, sizeof(float), size, file);
	fclose(file);
}

float* Tests::ReadFloatsToMemory(int inputCount, char *path) {
	float *input = new float[inputCount];
	ReadFloatsToBuffer(path, input, inputCount);
	return input;
}

bool Tests::CompareFloats(float *a, float *b, int length) {
	float av, bv;
	int wrong = 0;
	for (int i = 0; i < length; i++) {
		av = a[i];
		bv = b[i];
		if (fabs(av - bv) > 0.0001) {
			if (wrong == 0) { // Only first error.
				printf("* compare: FAILED, pos: %d, a: %f, b: %f\n", i, av, bv);
			}
			wrong++;
		}
	}
	if (wrong > 0) {
		printf("         = %.2f%% failed (%d)\n", (float)wrong / length * 100, wrong);
		return false;
	}
	return true;
}
