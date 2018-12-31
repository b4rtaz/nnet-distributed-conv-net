#pragma once

#define TESTS_BASE_PATH "D:\\yolo\\nnet\\nnet.cpp\\tests\\data\\"

namespace Tests {

	void RunTests();

	// Utils

	void ReadFloatsToBuffer(const char *path, float *buffer, int size);
	float *ReadFloatsToMemory(int inputCount, char *path);
	bool CompareFloats(float *a, float *b, int length);

	// Convolutional

	bool TestConvSingleThread();
	bool TestConvMultiThreads();
	bool TestConvWithoutNorm();

	// Block Convolutional

	bool TestConvBlockCollisionTest();
	bool TestConvBlockSamePartsResult();
	bool TestConvBlockSameResultAsWithoutBlock();

	// Maxpool

	bool TestMaxpoolLayer();

	// Yolo

	bool TestYoloRegionLayer();
}
