#include "FastCpuConvolutional.h"
#include <cstring>

using namespace nNet;
using namespace std;

void Im2col(float *input, int inWidth, int inHeight, int inChannels,
	int outFragmentTop, int outFragmentBottom, int outWidth, int outHeight, int padding, int size, int stride, float *workspace)
{
	int c, h, hOffset, w, wOffset, wIndex, ch, row, col;
	int css = inChannels * size * size;

	for (c = 0; c < css; ++c) {
		wOffset = c % size;
		hOffset = (c / size) % size;

		ch = c / size / size;
		for (h = outFragmentTop; h < outFragmentBottom; ++h) {
			for (w = 0; w < outWidth; ++w) {
				row = (hOffset + h * stride) - padding;
				col = (wOffset + w * stride) - padding;

				wIndex = (c * outHeight + h) * outWidth + w;

				workspace[wIndex] = (row < 0 || row >= inHeight || col < 0 || col >= inWidth)
					? 0
					: input[col + inWidth * (row + inHeight * ch)];
			}
		}
	}
}

void GemmNn(int inChannels,
	int outFragmentTop, int outFragmentBottom, int outWidth, int spatial, int filters, int size, float *weights, float *workspace, float *output)
{
	int css = inChannels * size * size;

	int wf = outFragmentTop * outWidth;
	int wt = outFragmentBottom * outWidth;

	float *lo, *lw;
	int f, w, c;
	for (f = 0; f < filters; ++f) {

		for (c = 0; c < css; ++c) {
			lw = &workspace[c * spatial + wf];
			lo = &output[f * spatial + wf];

			register float a = weights[f * css + c];
			for (w = wf; w < wt; w++) {
				*lo++ += a * *lw++;
			}
		}
	}
}

void Normalize(float *output,
	int outFragmentTop, int outFragmentBottom, int outWidth, int spacial, int filters, float *scales, float *rollingMean, float *rollingVariance)
{
	float scale, mean, variance;

	int wf = outFragmentTop * outWidth;
	int wt = outFragmentBottom * outWidth;
	int f, w, lwf, lwt;

	for (f = 0; f < filters; f++) {
		scale = scales[f];
		mean = rollingMean[f];
		variance = (float)sqrt(rollingVariance[f]) + .000001f;

		lwf = wf + f * spacial;
		lwt = wt + f * spacial;

		for (w = lwf; w < lwt; w++) {
			output[w] = ((output[w] - mean) / variance) * scale;
		}
	}
}

void AddBiasses(float *output,
	int outFragmentTop, int outFragmentBottom, int outWidth, int spacial, int filters, float *biases)
{
	int wf = outFragmentTop * outWidth;
	int wt = outFragmentBottom * outWidth;
	int f, w, lwf, lwt;

	for (f = 0; f < filters; f++) {
		register float bias = biases[f];

		lwf = wf + f * spacial;
		lwt = wt + f * spacial;

		for (w = lwf; w < lwt; w++) {
			output[w] += bias;
		}
	}
}

void ActivateOutput(float *output, int outFragmentTop, int outFragmentBottom, int outWidth, int spacial, int filters) {
	int wf = outFragmentTop * outWidth;
	int wt = outFragmentBottom * outWidth;
	int f, w, lwf, lwt;

	for (f = 0; f < filters; f++) {
		lwf = wf + f * spacial;
		lwt = wt + f * spacial;

		for (w = lwf; w < lwt; w++) {
			output[w] = ActivateLeaky(output[w]);
		}
	}
}

void ResetOutput(float *output, int outFragmentTop, int outFragmentBottom, int outWidth, int spacial, int filters) {
	int wf = outFragmentTop * outWidth;
	int wt = outFragmentBottom * outWidth;
	int f, lwf, lwt;

	for (f = 0; f < filters; f++) {
		lwf = wf + f * spacial;
		lwt = wt + f * spacial;

		memset((void*)&output[lwf], 0, sizeof(float) * (lwt - lwf));
	}
}

int FastCpuConvolutional::GetWorkspaceSize(int outWidth, int outHeight, int inChannels, int size) {
	return outWidth * outHeight * inChannels * size * size;
}

void FastCpuConvolutional::Forward(ConvolutionalLayer *layer, ConvolutionalFragmentParams *fragmentParams, float *workspace) {
	int top, height;
	ConvolutionalFragmentHelper::GetFragmentOutputCoords(layer, fragmentParams, &top, &height);
	int bottom = top + height;

	ResetOutput(layer->output, top, bottom, layer->outWidth, layer->outputFilterCount, layer->filters);

	Im2col(layer->input, layer->inWidth, layer->inHeight, layer->inChannels,
		top, bottom, layer->outWidth, layer->outHeight, layer->padding, layer->size, layer->stride, workspace);

	GemmNn(layer->inChannels,
		top, bottom, layer->outWidth, layer->outputFilterCount, layer->filters, layer->size, layer->weights, workspace, layer->output);

	if (layer->normalize) {
		Normalize(layer->output,
			top, bottom, layer->outWidth, layer->outputFilterCount, layer->filters, layer->scales, layer->rollingMean, layer->rollingVariance);
	}

	AddBiasses(layer->output,
		top, bottom, layer->outWidth, layer->outputFilterCount, layer->filters, layer->biases);

	if (layer->activation == LEAKY) {
		ActivateOutput(layer->output,
			top, bottom, layer->outWidth, layer->outputFilterCount, layer->filters);
	}
}
