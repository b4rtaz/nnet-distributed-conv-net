#include "ConvolutionalLayer.h"

using namespace nNet;

int ConvolutionalLayerHelper::GetOutputLength(int inLength, int padding, int size, int stride) {
	return ((inLength + (2 * padding) - size) / stride) + 1;
}

ConvolutionalLayer::ConvolutionalLayer(ConvolutionalLayerParams *params) {
	layerType = CONVOLUTIONAL;

	inWidth = params->inputWidth;
	inHeight = params->inputHeight;
	inChannels = params->inputChannels;
	filters = params->filters;
	padding = params->padding;
	size = params->size;
	stride = params->stride;
	activation = params->activation;

	outWidth = ConvolutionalLayerHelper::GetOutputLength(params->inputWidth, params->padding, params->size, params->stride);
	outHeight = ConvolutionalLayerHelper::GetOutputLength(params->inputHeight, params->padding, params->size, params->stride);
	outChannels = filters;

	weightCount = params->inputChannels * params->filters * params->size * params->size;
	weights = new float[weightCount];

	inputChannelCount = params->inputWidth * params->inputHeight;
	inputCount = inputChannelCount * params->inputChannels;

	outputFilterCount = outWidth * outHeight;
	outputCount = outputFilterCount * params->filters;
	output = new float[outputCount];

	biases = new float[params->filters];

	normalize = params->normalize;
	if (normalize) {
		rollingMean = new float[params->filters];
		rollingVariance = new float[params->filters];
		scales = new float[params->filters];
	}
}

ConvolutionalLayer::~ConvolutionalLayer() {
	delete[] weights;
	delete[] output;
	delete[] biases;
	if (normalize) {
		delete[] rollingMean;
		delete[] rollingVariance;
		delete[] scales;
	}
}
