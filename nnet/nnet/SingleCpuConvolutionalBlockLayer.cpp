#include "SingleCpuConvolutionalBlockLayer.h"
#include "ConvolutionalFragment.h"
#include "MaxpoolLayer.h"
#include "Layer.h"

using namespace nNet;
using namespace std;

SingleCpuConvolutionalBlockLayer::SingleCpuConvolutionalBlockLayer(ConvolutionalBlockLayerParams *params)
	: ConvolutionalBlockLayer(params) {
}

void SingleCpuConvolutionalBlockLayer::Forward() {
	ConvolutionalLayer *first = (ConvolutionalLayer*)layers.front();
	vector<BlockLayerFragmentParams> fragments;
	int top = 0;
	int height = first->inHeight;
	ConvolutionalBlockOffsetCalculator::Calculate(&layers, &top, &height, &fragments);
	Forward(&fragments);
}

void SingleCpuConvolutionalBlockLayer::Forward(vector<BlockLayerFragmentParams> *fragments) {
	int n = (int)layers.size();

	ConvolutionalLayer *conv;
	BlockLayerFragmentParams *convFp;
	ConvolutionalFragmentParams convParams;
	MaxpoolLayer *maxp;
	BlockLayerFragmentParams *maxpFp;
	MaxpoolFragmentParams maxpParams;

	float *in = input;
	for (int i = 0; i < n; i += 2) {
		conv = (ConvolutionalLayer*)layers.at(i);
		convFp = &fragments->at(i);
		maxp = (MaxpoolLayer*)layers.at(i + 1);
		maxpFp = &fragments->at(i + 1);

		convParams = { convFp->extendInFragmentTop, convFp->extendInFragmentHeight };
		conv->input = in;
		conv->Forward(&convParams);

		maxpParams = { maxpFp->extendInFragmentTop, maxpFp->extendInFragmentHeight };
		maxp->input = conv->output;
		maxp->Forward(&maxpParams);

		in = maxp->output;
	}

	output = in;
}
