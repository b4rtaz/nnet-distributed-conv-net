#include "ConvolutionalLayer.h"
#include "ConvolutionalBlockLayer.h"
#include "MaxpoolLayer.h"
#include <algorithm>
#include <stdexcept>

using namespace nNet;
using namespace std;

ConvolutionalBlockLayer::ConvolutionalBlockLayer(ConvolutionalBlockLayerParams *params) {
	layerType = BLOCK;
	layers = params->layers;
}

ConvolutionalBlockLayer::~ConvolutionalBlockLayer() {
	for (auto i = layers.begin(); i != layers.end(); i++) {
		delete (*i);
	}
}

void CalculateExtendFragment(int inFragmentTop, int inFragmentHeight, int inHeight, int offsetTop, int offsetBottom, int *extendInFragmentTop, int *extendInFragmentHeight) {
	*extendInFragmentTop = max(0, inFragmentTop - offsetTop);
	int oldBottom = inFragmentTop + inFragmentHeight;
	int newBottom = min(oldBottom + offsetBottom, inHeight);
	*extendInFragmentHeight = newBottom - *extendInFragmentTop;
}

void ConvolutionalBlockOffsetCalculator::Calculate(vector<Layer*> *layers, int *inFragmentTop, int *inFragmentHeight, vector<BlockLayerFragmentParams> *fragments) {
	fragments->resize(layers->size());

	int currentInTop = *inFragmentTop;
	int currentInHeight = *inFragmentHeight;

	auto l = layers->begin();
	auto f = fragments->begin();
	for (; l != layers->end(); l++, f++) {
		Layer *layer = *l;

		f->layer = layer;
		f->inFragmentTop = currentInTop;
		f->inFragmentHeight = currentInHeight;

		if (layer->layerType == MAXPOOL) {
			MaxpoolFragmentParams maxpParams = { currentInTop, currentInHeight };
			MaxpoolFragmentHelper::GetFragmentOutputCoords((MaxpoolLayer*)layer, &maxpParams, &currentInTop, &currentInHeight);
		}
	}

	int offsetInTop = 0;
	int offsetInBottom = 0;

	auto rf = fragments->rbegin();
	for (; rf != fragments->rend(); rf++) {
		if (rf->layer->layerType == CONVOLUTIONAL) {
			auto convLayer = (ConvolutionalLayer*)rf->layer;

			rf->offsetInTop = offsetInTop;
			rf->offsetInBottom = offsetInBottom;
			CalculateExtendFragment(rf->inFragmentTop, rf->inFragmentHeight,
				convLayer->inHeight, offsetInTop, offsetInBottom, &rf->extendInFragmentTop, &rf->extendInFragmentHeight);

			offsetInTop += convLayer->size / 2;
			offsetInBottom += convLayer->size / 2;
		}
		else if (rf->layer->layerType == MAXPOOL) {
			auto maxpLayer = (MaxpoolLayer*)rf->layer;
			auto scale = (double)maxpLayer->inHeight / maxpLayer->outHeight;

			offsetInTop = (int)(offsetInTop * scale);
			offsetInBottom = (int)(offsetInBottom * scale);

			rf->offsetInTop = offsetInTop;
			rf->offsetInBottom = offsetInBottom;
			CalculateExtendFragment(rf->inFragmentTop, rf->inFragmentHeight,
				maxpLayer->inHeight, offsetInTop, offsetInBottom, &rf->extendInFragmentTop, &rf->extendInFragmentHeight);

			if (rf->extendInFragmentTop % maxpLayer->size != 0)
				offsetInTop += maxpLayer->size / 2;
			if (maxpLayer->stride == 1)
				offsetInBottom += maxpLayer->size / 2;
		}
		else {
			throw logic_error("Unknow layer type.");
		}
	}

	auto firstLayer = (ConvolutionalLayer*)layers->front();
	auto firstFragment = &fragments->front();
	CalculateExtendFragment(firstFragment->inFragmentTop, firstFragment->inFragmentHeight,
		firstLayer->inHeight, offsetInTop, offsetInBottom, inFragmentTop, inFragmentHeight);
}
