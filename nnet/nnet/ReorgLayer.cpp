#include "ReorgLayer.h"

using namespace nNet;
using namespace std;

ReorgLayer::ReorgLayer(ReorgLayerParams *params) {
	layerType = REORG;

	inWidth = params->inWidth;
	inHeight = params->inHeight;
	inChannels = params->inChannels;
	inputCount = inWidth * inHeight * inChannels;

	stride = params->stride;
	reverse = params->reverse;

	if (reverse) {
		outWidth = inWidth * stride;
		outHeight = inHeight * stride;
		outChannels = inChannels / (stride * stride);
	}
	else {
		outWidth = inWidth / stride;
		outHeight = inHeight / stride;
		outChannels = inChannels * (stride * stride);
	}

	outputCount = outWidth * outHeight * outChannels;
	output = new float[outputCount];
}

ReorgLayer::~ReorgLayer() {
	delete[] output;
}

void ReorgCpu(float *x, int w, int h, int c, int batch, int stride, bool forward, float *out)
{
	int b, i, j, k;
	int out_c = c / (stride*stride);

	for (b = 0; b < batch; ++b) {
		for (k = 0; k < c; ++k) {
			for (j = 0; j < h; ++j) {
				for (i = 0; i < w; ++i) {
					int in_index = i + w*(j + h*(k + c*b));
					int c2 = k % out_c;
					int offset = k / out_c;
					int w2 = i*stride + offset % stride;
					int h2 = j*stride + offset / stride;
					int out_index = w2 + w*stride*(h2 + h*stride*(c2 + out_c*b));
					if (forward) out[out_index] = x[in_index];
					else out[in_index] = x[out_index];
				}
			}
		}
	}
}

void ReorgLayer::Forward() {
	ReorgCpu(input, inWidth, inHeight, inChannels, 1, stride, reverse, output);
}
