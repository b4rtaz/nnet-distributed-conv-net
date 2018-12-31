#include "Tests.h"
#include <cstdio>

using namespace std;

void PrintSection(char *name) {
	printf("----------------------\n");
	printf("%s\n", name);
	printf("----------------------\n");
}

void Tests::RunTests() {
	bool all = true;

	printf(" ___  ___  __  ___ __ \n");
	printf("|_ _|| __|/ _||_ _/ _|\n");
	printf(" | | | _| \\_ \\ | |\\_ \\\n");
	printf(" |_| |___||__/ |_||__/\n\n");

	PrintSection("convolutional block (parts)");
	all &= TestConvBlockCollisionTest();
	all &= TestConvBlockSamePartsResult();

	PrintSection("convolutional block (full)");
	all &= TestConvBlockSameResultAsWithoutBlock();

	PrintSection("convolutional");
	all &= TestConvSingleThread();
	all &= TestConvMultiThreads();
	all &= TestConvWithoutNorm();

	PrintSection("maxpool");
	all &= TestMaxpoolLayer();

	PrintSection("yolo region");
	all &= TestYoloRegionLayer();

	printf("Tests: %s\n", all ? "SUCCESS" : "FAIL");
}
