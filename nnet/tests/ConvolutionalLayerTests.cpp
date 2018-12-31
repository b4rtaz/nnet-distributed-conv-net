#include "Tests.h"
#include "../nNet/Benchmark.h"
#include "../nNet/ConvolutionalLayer.h"
#include "../nNet/ConvolutionalFragment.h"
#include "../nNet/SingleCpuConvolutionalLayer.h"
#include "../nNet/MultiCpuConvolutionalLayer.h"
#include <cstdio>

using namespace nNet;
using namespace Tests;

// DATA

ConvolutionalLayerParams CreateConvParams(int inputWidth, int inputHeight, int inputChannels,
	int filters, int padding, int size, int stride, Activation activation, bool normalize)
{
	ConvolutionalLayerParams params;
	params.inputWidth = inputWidth;
	params.inputHeight = inputHeight;
	params.inputChannels = inputChannels;
	params.filters = filters;
	params.padding = padding;
	params.size = size;
	params.stride = stride;
	params.activation = activation;
	params.normalize = normalize;
	return params;
}

// layer8:
ConvolutionalLayerParams GetConvLayer8Params() {
	return CreateConvParams(26, 26, 128, 256, 1, 3, 1, LEAKY, true);
}

float *LoadConvLayer8TestData(ConvolutionalLayer *layer) {
	ReadFloatsToBuffer(TESTS_BASE_PATH "conv_8_biases_256.data", layer->biases, layer->filters);
	ReadFloatsToBuffer(TESTS_BASE_PATH "conv_8_weights_294912.data", layer->weights, layer->weightCount);
	ReadFloatsToBuffer(TESTS_BASE_PATH "conv_8_rolling_mean_256.data", layer->rollingMean, layer->filters);
	ReadFloatsToBuffer(TESTS_BASE_PATH "conv_8_rolling_variance_256.data", layer->rollingVariance, layer->filters);
	ReadFloatsToBuffer(TESTS_BASE_PATH "conv_8_scales_256.data", layer->scales, layer->filters);

	layer->input = ReadFloatsToMemory(layer->inputCount, TESTS_BASE_PATH "conv_8_inputs_86528.data");
	return ReadFloatsToMemory(layer->outputCount, TESTS_BASE_PATH "conv_8_output_173056.data");
}

// layer14:
ConvolutionalLayerParams GetConvLayer14Params() {
	return CreateConvParams(13, 13, 512, 45, 0, 1, 1, LINEAR, 0);
}

float *LoadConvLayer14TestData(ConvolutionalLayer *layer) {
	ReadFloatsToBuffer(TESTS_BASE_PATH "conv_14_biases_45.data", layer->biases, layer->filters);
	ReadFloatsToBuffer(TESTS_BASE_PATH "conv_14_weights_23040.data", layer->weights, layer->weightCount);

	layer->input = ReadFloatsToMemory(layer->inputCount, TESTS_BASE_PATH "conv_14_inputs_86528.data");
	return ReadFloatsToMemory(layer->outputCount, TESTS_BASE_PATH "conv_14_output_7605.data");
}

// HELPERS

void ConvLayerForward(SingleCpuConvolutionalLayer *layer) {
	Benchmark b;
	layer->Forward();
	std::printf("[conv] single, forwarded in %.4lf seconds\n", b.TotalSeconds());
}

void ConvLayerForward(MultiCpuConvolutionalLayer *layer, ConvolutionalFragmentParams *params, double *time) {
	Benchmark b;
	layer->Forward(params);
	*time = b.TotalSeconds();
	std::printf("[conv] multi, forwarded in %.4lf seconds\n", *time);
}

// TESTS

bool Tests::TestConvSingleThread() {
	ConvolutionalLayerParams params = GetConvLayer8Params();
	SingleCpuConvolutionalLayer *layer = new SingleCpuConvolutionalLayer(&params);
	float *expectedOutput = LoadConvLayer8TestData(layer);

	ConvLayerForward(layer);
	bool res = CompareFloats(layer->output, expectedOutput, layer->outputCount);

	delete[] expectedOutput;
	delete[] layer->input;
	delete layer;
	return res;
}

#define TEST_CONV_MULTI_PARTS 2
bool Tests::TestConvMultiThreads() {
	ConvolutionalLayerParams sourceLayerParams = GetConvLayer8Params();
	ConvolutionalLayer *sourceLayer = new SingleCpuConvolutionalLayer(&sourceLayerParams);
	float *expectedOutput = LoadConvLayer8TestData(sourceLayer);

	int p, parts = TEST_CONV_MULTI_PARTS;
	ConvolutionalFragmentParams params[TEST_CONV_MULTI_PARTS];
	float *inputFragments[TEST_CONV_MULTI_PARTS];
	float *outputFragments[TEST_CONV_MULTI_PARTS];
	int inputFragmentBytes[TEST_CONV_MULTI_PARTS];
	int outputFragmentBytes[TEST_CONV_MULTI_PARTS];

	ConvolutionalFragmentParams p0 = { 0, sourceLayer->inHeight };

	// Tniemy dane wejœciowe na pakiety.
	for (p = 0; p < parts; p++) {
		ConvolutionalFragmentHelper::SplitFragment(&p0, &params[p], parts, p);

		// Alokujemy pamiêæ z nadmiarem (nigdy fragment nie bêdzie d³u¿szy ni¿ ca³e wejœcie).
		inputFragments[p] = new float[sourceLayer->inputCount];
		ConvolutionalFragmentHelper::WriteFragmentInput(sourceLayer, &params[p], inputFragments[p], &inputFragmentBytes[p]);
	}

	double time = 0.0;

	// Wyliczamy fragmenty wyjœæ za pomoc¹ fragmentów wejœæ.
	for (p = 0; p < parts; p++) {
		MultiCpuConvolutionalLayer *fragmentLayer = new MultiCpuConvolutionalLayer(&sourceLayerParams);
		memcpy(fragmentLayer->biases, sourceLayer->biases, sourceLayer->filters * sizeof(float));
		memcpy(fragmentLayer->weights, sourceLayer->weights, sourceLayer->weightCount * sizeof(float));
		memcpy(fragmentLayer->rollingMean, sourceLayer->rollingMean, sourceLayer->filters * sizeof(float));
		memcpy(fragmentLayer->rollingVariance, sourceLayer->rollingVariance, sourceLayer->filters * sizeof(float));
		memcpy(fragmentLayer->scales, sourceLayer->scales, sourceLayer->filters * sizeof(float));

		fragmentLayer->input = new float[fragmentLayer->inputCount];

		// Wczytujemy pakiet do warstwy.
		ConvolutionalFragmentHelper::ReadFragmentInput(fragmentLayer, &params[p], inputFragments[p]);

		double partTime;
		ConvLayerForward(fragmentLayer, &params[p], &partTime);
		time += partTime;

		int fOffset, fLength;
		ConvolutionalFragmentHelper::GetFragmentOutputSingleFilterOffset(fragmentLayer, &params[p], &fOffset, &fLength);

		// Testujemy czy dany fragment zosta³ wyliczony poprawnie.
		CompareFloats(&fragmentLayer->output[fOffset], &expectedOutput[fOffset], fLength);

		// Pakujemy wyjœciowy fragment.
		outputFragments[p] = new float[fragmentLayer->outputCount];
		ConvolutionalFragmentHelper::WriteFragmentOutput(fragmentLayer, &params[p], outputFragments[p], &outputFragmentBytes[p]);

		delete fragmentLayer->input;
		delete fragmentLayer;
	}

	// Sk³adamy fragmenty wyjœæ w ca³oœæ.
	for (p = 0; p < parts; p++) {
		ConvolutionalFragmentHelper::ReadFragmentOutput(sourceLayer, &params[p], outputFragments[p]);
	}

	// Testujemy czy ca³e wyjœcie zbudowane z wyliczonych fragmentów jest poprawne.
	bool res = CompareFloats(sourceLayer->output, expectedOutput, sourceLayer->outputCount);

	printf("     | %.5f seconds\n", time);

	for (p = 0; p < parts; p++) {
		delete inputFragments[p];
		delete outputFragments[p];
	}

	delete[] expectedOutput;
	delete[] sourceLayer->input;
	delete sourceLayer;
	return res;
}

bool Tests::TestConvWithoutNorm() {
	ConvolutionalLayerParams layerParams = GetConvLayer14Params();
	SingleCpuConvolutionalLayer *layer = new SingleCpuConvolutionalLayer(&layerParams);
	float *expectedOutput = LoadConvLayer14TestData(layer);

	ConvLayerForward(layer);
	bool res = CompareFloats(layer->output, expectedOutput, layer->outputCount);

	delete[] expectedOutput;
	delete[] layer->input;
	delete layer;
	return res;
}
