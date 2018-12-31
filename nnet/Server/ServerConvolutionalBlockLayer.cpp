#include "ServerConvolutionalBlockLayer.h"

using namespace Server;

ServerConvolutionalBlockLayer::ServerConvolutionalBlockLayer(ConvolutionalBlockLayerParams *params)
	: MultiCpuConvolutionalBlockLayer::MultiCpuConvolutionalBlockLayer(params) {
	auto firstLayer = params->layers.front();
	input = new float[firstLayer->inputCount];
}

ServerConvolutionalBlockLayer::~ServerConvolutionalBlockLayer() {
	delete[] input;
}
