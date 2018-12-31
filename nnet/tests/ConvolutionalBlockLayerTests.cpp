#include "../nNet/Benchmark.h"
#include "../nNet/YoloNetParser.h"
#include "../nNet/LayerFactory.h"
#include "../nNet/Network.h"
#include "../nNet/BlockLayerInjector.h"
#include "../nNet/YoloRegionLayer.h"
#include "../nNet/YoloWeightsLoader.h"
#include "../nNet/MaxpoolLayer.h"
#include "../nNet/ConvolutionalFragment.h"
#include "Tests.h"
#include <exception>
#include <ctime>
#include <cstdlib>

using namespace nNet;
using namespace std;

Network* GetConvBlockNetwork(bool injectBlock, int layersLimit = 8) {
	LayerFactory layerFactory;

	YoloNetParser yoloNetParser(false, &layerFactory);
	Network *network = yoloNetParser.Read(TESTS_BASE_PATH "tiny.cfg");

	YoloWeightsLoader::Read(network, TESTS_BASE_PATH "tiny.weights");

	if (injectBlock) {
		BlockLayerInjector blockInjector(&layerFactory);
		if (!blockInjector.Inject(&network->layers, 1, layersLimit))
			throw exception("Invalid network.");
	}
	return network;
}

float* GetConvBlockNetwork() {
	return Tests::ReadFloatsToMemory(519168, TESTS_BASE_PATH "tiny-image.data");
}

bool Tests::TestConvBlockCollisionTest() {
	bool areEqual = true;
	float *input = GetConvBlockNetwork();

	auto matrixNetwork = GetConvBlockNetwork(true, 2);
	auto matrixBlockLayer = (ConvolutionalBlockLayer*)matrixNetwork->layers.front();

	auto lastLayer = matrixBlockLayer->layers.back();

	float *lastOutput = new float[lastLayer->outputCount];

	int attemps = 32;
	for (int test = 0; test < attemps; test++) {
		matrixBlockLayer->input = input;
		matrixBlockLayer->Forward();

		if (test > 0) {
			areEqual = Tests::CompareFloats(lastOutput, matrixBlockLayer->output, lastLayer->outputCount);
			if (!areEqual) {
				printf("collision test %d FAILED\n", test);
				break;
			}
		}

		memcpy(lastOutput, matrixBlockLayer->output, sizeof(float) * lastLayer->outputCount);
	}

	delete[] input;
	delete matrixNetwork;
	return areEqual;
}

bool Tests::TestConvBlockSamePartsResult() {
	float *input = GetConvBlockNetwork();

	// Calculating expected output.
	auto matrixNetwork = GetConvBlockNetwork(true);
	ConvolutionalBlockLayer *matrixBlockLayer = (ConvolutionalBlockLayer*)matrixNetwork->layers.front();
	auto matrixFirstBlockLayer = (ConvolutionalLayer*)matrixBlockLayer->layers.front();
	auto matrixLastBlockLayer = (MaxpoolLayer*)matrixBlockLayer->layers.back();
	matrixBlockLayer->input = input;

	Benchmark mb;
	matrixBlockLayer->Forward();
	printf("[block] input = %d bytes, output = %d bytes\n",
		matrixFirstBlockLayer->inputCount * sizeof(float),
		matrixLastBlockLayer->outputCount * sizeof(float));
	printf("[block] single, forwarded in %.4lf seconds\n", mb.TotalSeconds());

	// Calculating blocks.
	float *gluedOutput = new float[matrixLastBlockLayer->outputCount];
	int parts = 4;

	ConvolutionalFragmentParams zeroParams = { 0, matrixFirstBlockLayer->inHeight };

	for (int i = 0; i < parts; i++) {
		auto partNetwork = GetConvBlockNetwork(true);
		auto partBlockLayer = (ConvolutionalBlockLayer*)partNetwork->layers.front();
		auto partFirstBlockLayer = (ConvolutionalLayer*)partBlockLayer->layers.front();
		auto partLastBlockLayer = (MaxpoolLayer*)partBlockLayer->layers.back();

		ConvolutionalFragmentParams partParams;
		ConvolutionalFragmentHelper::SplitFragment(&zeroParams, &partParams, parts, i);

		vector<BlockLayerFragmentParams> fragments;
		ConvolutionalBlockOffsetCalculator::Calculate(&partBlockLayer->layers, &partParams.top, &partParams.height, &fragments);

		auto firstFragment = &fragments.front();
		float *partInput = new float[partFirstBlockLayer->inputCount];

		int inFragmentOffset;
		int inFragmentLenght;
		ConvolutionalFragmentHelper::GetFragmentInputSingleChannelOffsetWithoutPadding(partFirstBlockLayer, &partParams, &inFragmentOffset, &inFragmentLenght);

		for (int ch = 0; ch < partFirstBlockLayer->inChannels; ch++) {
			memcpy(
				&partInput[(ch * partFirstBlockLayer->inputChannelCount) + inFragmentOffset],
				&input[(ch * partFirstBlockLayer->inputChannelCount) + inFragmentOffset],
				inFragmentLenght * sizeof(float));
		}

		Benchmark pb;
		partBlockLayer->input = partInput;
		partBlockLayer->Forward(&fragments);
		printf("[block] part, forwarded in %.4lf seconds\n", pb.TotalSeconds());

		int outFragmentOffset;
		int outFragmentLength;

		auto lastFragmentParams = &fragments[fragments.size() - 1];
		MaxpoolFragmentParams lastMaxpoolFragmentParams = {
			lastFragmentParams->inFragmentTop,
			lastFragmentParams->inFragmentHeight
		};
		MaxpoolFragmentHelper::GetFragmentOutputSingleFilterOffset(partLastBlockLayer, &lastMaxpoolFragmentParams,
			&outFragmentOffset, &outFragmentLength);

		for (int ch = 0; ch < partLastBlockLayer->outChannels; ch++) {
			int chOffset = (ch * partLastBlockLayer->outputChannelCount);
			memcpy(
				&gluedOutput[chOffset + outFragmentOffset],
				&partLastBlockLayer->output[chOffset + outFragmentOffset],
				outFragmentLength * sizeof(float));
		}

		delete partInput;
		delete partNetwork;
	}

	// And comparision...
	bool areEqual = Tests::CompareFloats(gluedOutput, matrixBlockLayer->output, matrixLastBlockLayer->outputCount);

	delete[] gluedOutput;
	delete matrixNetwork;
	delete[] input;

	return areEqual;
}

bool Tests::TestConvBlockSameResultAsWithoutBlock() {
	float *input = GetConvBlockNetwork();
	
	auto withoutBlockNetwork = GetConvBlockNetwork(false);
	auto withBlockNetwork = GetConvBlockNetwork(true);

	auto woLastLayer = withoutBlockNetwork->Forward(input);
	auto wiLastLayer = (YoloRegionLayer*)withBlockNetwork->Forward(input);

	bool areSameOutputs = Tests::CompareFloats(woLastLayer->output, wiLastLayer->output, wiLastLayer->outputCount);

	int regionCount;
	auto regions = wiLastLayer->ReadRegions(1620, 1080, 416, 416, 0.6f, &regionCount);

	delete withBlockNetwork;
	delete withoutBlockNetwork;
	delete[] input;
	delete[] regions;
	return areSameOutputs && regionCount == 5;
}
