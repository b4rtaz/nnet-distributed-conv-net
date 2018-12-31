#include "Tests.h"
#include "../nNet/MaxpoolLayer.h"

using namespace nNet;
using namespace Tests;

bool Tests::TestMaxpoolLayer() {
	MaxpoolLayerParams params;
	params.inputWidth = 26;
	params.inputHeight = 26;
	params.inputChannels = 256;
	params.size = 2;
	params.stride = 2;
	params.padding = 0;
	MaxpoolLayer *layer = new MaxpoolLayer(&params);

	layer->input = ReadFloatsToMemory(173056, TESTS_BASE_PATH "conv_8_output_173056.data");
	float *expectedOutput = ReadFloatsToMemory(layer->outputCount, TESTS_BASE_PATH "maxpool_9_output_43264.data");

	layer->Forward();
	bool res = CompareFloats(layer->output, expectedOutput, layer->outputCount);

	delete[] expectedOutput;
	delete[] layer->input;
	delete layer;
	return res;
}
