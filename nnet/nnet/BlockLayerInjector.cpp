#include "BlockLayerInjector.h"
#include "ConvolutionalBlockLayer.h"
#include "Layer.h"
#include <cmath>

using namespace nNet;
using namespace std;

BlockLayerInjector::BlockLayerInjector(LayerFactory *layerFactory) {
	_layerFactory = layerFactory;
}

int GetLayerRealIndex(vector<Layer*> *layers, int index) {
	Layer *layer;
	int n = (int)layers->size();
	for (int i = 0; i < n; i++) {
		layer = layers->at(i);
		if (layer->layerType != BLOCK && layer->index == index) {
			return i;
		}
	}
	return -1;
}

ConvolutionalBlockLayer *BlockLayerInjector::Inject(vector<Layer*> *layers, int indexFrom, int indexTo) {
	int index = GetLayerRealIndex(layers, indexFrom);
	if (index < 0)
		throw logic_error("Cannot find first layer for block.");

	indexTo = index + (indexTo - indexFrom);

	vector<Layer*> res;
	for (; index <= indexTo; indexTo--) {
		auto layer = layers->at(index);
		if (layer->layerType != CONVOLUTIONAL && layer->layerType != MAXPOOL) {
			throw logic_error("Invalid layer type. Block layer supports only conv and maxpool layers.");
		}
		res.push_back(layer);
		layers->erase(layers->begin() + index);
	}

	Layer *first = res.front();
	Layer *last = res.back();

	ConvolutionalBlockLayerParams params = { res };
	ConvolutionalBlockLayer *layer = _layerFactory->CreateConvolutionalBlockLayer(&params);
	layer->index = last->index;
	layer->inputCount = first->inputCount;
	layer->outputCount = last->outputCount;
	layers->insert(layers->begin() + index, layer);
	return layer;
}
