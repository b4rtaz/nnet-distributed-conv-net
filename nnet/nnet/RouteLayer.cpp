#include "RouteLayer.h"
#include "ConvolutionalLayer.h"
#include "ReorgLayer.h"
#include <stdexcept>

using namespace nNet;
using namespace std;

void GetLayertOutParams(Layer *layer, int *outWidth, int *outHeight, int *outChannels) {
	if (layer->layerType == CONVOLUTIONAL) {
		auto conv = (ConvolutionalLayer*)layer;
		*outWidth = conv->outWidth;
		*outHeight = conv->outHeight;
		*outChannels = conv->outChannels;
	}
	else if (layer->layerType == REORG) {
		auto reorg = (ReorgLayer*)layer;
		*outWidth = reorg->outWidth;
		*outHeight = reorg->outHeight;
		*outChannels = reorg->outChannels;
	}
	else {
		throw logic_error("Not implemented layer type.");
	}
}

RouteLayer::RouteLayer(RouteLayerParams *params) {
	layerType = ROUTE;
	network = params->network;
	layerIndexes = params->layerIndexes;
	layerCount = (int)params->layerIndexes.size();

	Layer *layer;
	int i;
	int total = 0;
	for (i = 0; i < layerCount; ++i) {
		layer = network->GetLayerByIndex(layerIndexes[i]);
		total += layer->outputCount;
	}

	inputCount = total;
	outputCount = total;
	output = new float[total];

	layer = network->GetLayerByIndex(layerIndexes[0]);
	GetLayertOutParams(layer, &outWidth, &outHeight, &outChannels);

	int ow, oh, oc;
	for (i = 1; i < layerCount; i++) {
		layer = network->GetLayerByIndex(layerIndexes[i]);
		GetLayertOutParams(layer, &ow, &oh, &oc);
		if (ow != outWidth || oh != outHeight) {
			throw logic_error("Strange... Invalid widths...");
		}
		outChannels += oc;
	}
}

RouteLayer::~RouteLayer() {
	delete[] output;
}

void CopyCpu(int N, float *X, int INCX, float *Y, int INCY) {
	int i;
	for (i = 0; i < N; ++i) Y[i*INCY] = X[i*INCX];
}

void RouteLayer::Forward() {
	Layer *layer;
	int i;
	int offset = 0;
	float *input;

	for (i = 0; i < layerCount; ++i) {
		layer = network->GetLayerByIndex(layerIndexes[i]);
		input = layer->output;

		CopyCpu(layer->outputCount, input, 1, output + offset, 1);

		offset += layer->outputCount;
	}
}
