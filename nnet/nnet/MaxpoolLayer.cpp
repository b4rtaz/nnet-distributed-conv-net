#include "MaxpoolLayer.h"
#include <cfloat>
#include <cstdio>
#include <algorithm>

using namespace nNet;
using namespace std;

MaxpoolLayer::MaxpoolLayer(MaxpoolLayerParams *params) {
	layerType = MAXPOOL;

	inWidth = params->inputWidth;
	inHeight = params->inputHeight;
	inChannels = params->inputChannels;

	outWidth = (params->inputWidth + 2 * params->padding) / params->stride;
	outHeight = (params->inputHeight + 2 * params->padding) / params->stride;
	outChannels = params->inputChannels;

	size = params->size;
	stride = params->stride;
	padding = params->padding;

	inputCount = params->inputWidth * params->inputHeight * params->inputChannels;
	outputCount = outWidth * outHeight * outChannels;
	output = new float[outputCount];
	outputChannelCount = outWidth * outHeight;
}

MaxpoolLayer::~MaxpoolLayer() {
	delete output;
}

void ForwardMaxpool(float *input, int inWidth, int inHeight, int size, int padding, int stride, 
	int outFragmentTop, int outFragmentBottom, int outWidth, int outHeight, int outChannels, float *output)
{
	int i, j, k, m, n;
	int spacial = outWidth * outHeight;

	for (k = 0; k < outChannels; ++k) {
		for (i = outFragmentTop; i < outFragmentBottom; ++i) {
			for (j = 0; j < outWidth; ++j) {
				int outIndex = j + outWidth * i + spacial * k;
				float max = -FLT_MAX;

				for (n = 0; n < size; ++n) {
					for (m = 0; m < size; ++m) {
						int cur_h = -padding + i*stride + n;
						int cur_w = -padding + j*stride + m;
						int index = cur_w + inWidth *(cur_h + inHeight*k);

						int valid = (cur_h >= 0 && cur_h < inHeight && cur_w >= 0 && cur_w < inWidth);
						float val = (valid != 0) ? input[index] : -FLT_MAX;
						max = (val > max) ? val : max;
					}
				}
				output[outIndex] = max;
			}
		}
	}
}

void MaxpoolLayer::Forward() {
	ForwardMaxpool(input, inWidth, inHeight, size, padding, stride, 
		0, outHeight, outWidth, outHeight, outChannels, output);
}

void MaxpoolLayer::Forward(MaxpoolFragmentParams *params) {
	int outFragmentTop;
	int outFragmentHeight;
	MaxpoolFragmentHelper::GetFragmentOutputCoords(this, params, &outFragmentTop, &outFragmentHeight);
	int outFragmentBottom = outFragmentTop + outFragmentHeight;

	ForwardMaxpool(input, inWidth, inHeight, size, padding, stride, 
		outFragmentTop, outFragmentBottom, outWidth, outHeight, outChannels, output);
}

void MaxpoolFragmentHelper::GetFragmentOutputCoords(MaxpoolLayer *layer, MaxpoolFragmentParams *params, int *outFragmentTop, int *outFragmentHeight) {
	// TODO
	double scale = (double)layer->inHeight / layer->outHeight;

	*outFragmentTop = (int)((double)params->top / scale);
	int nextTop = (int)((params->top + params->height) / scale);
	*outFragmentHeight = nextTop - *outFragmentTop;
}

void MaxpoolFragmentHelper::GetFragmentOutputSingleFilterOffset(MaxpoolLayer *layer, MaxpoolFragmentParams *params, int *outFragmentOffset, int *outFragmentLength) {
	int outFragmentTop;
	int outFragmentHeight;
	GetFragmentOutputCoords(layer, params, &outFragmentTop, &outFragmentHeight);
	*outFragmentOffset = outFragmentTop * layer->outWidth;
	*outFragmentLength = outFragmentHeight * layer->outWidth;
}
