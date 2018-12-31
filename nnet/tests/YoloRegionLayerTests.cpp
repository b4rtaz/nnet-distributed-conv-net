#include "Tests.h"
#include "../nNet/YoloRegionLayer.h"
#include <cstdio>
#include <ctime>

using namespace nNet;
using namespace Tests;

bool Tests::TestYoloRegionLayer() {
	YoloRegionLayerParams params;
	params.inputWidth = 13;
	params.inputHeight = 13;
	params.classes = 4;
	params.coords = 4;
	params.num = 5;
	YoloRegionLayer *layer = new YoloRegionLayer(&params);

	layer->input = ReadFloatsToMemory(layer->inputCount, TESTS_BASE_PATH "conv_14_output_7605.data");

	layer->biases[0] = 0.57273f;
	layer->biases[1] = 0.677385f;
	layer->biases[2] = 1.87446f;
	layer->biases[3] = 2.06253f;
	layer->biases[4] = 3.33843f;
	layer->biases[5] = 5.47434f;
	layer->biases[6] = 7.88282f;
	layer->biases[7] = 3.52778f;
	layer->biases[8] = 9.77052f;
	layer->biases[9] = 9.16828f;

	layer->Forward();

	int regionCount;
	YoloRegion *regions = layer->ReadRegions(1620, 1080, 416, 416, 0.6f, &regionCount);

	delete[] regions;
	delete[] layer->input;
	delete[] layer;
	return regionCount == 5;
}
