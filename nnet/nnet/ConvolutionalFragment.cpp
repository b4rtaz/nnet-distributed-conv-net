#include "ConvolutionalFragment.h"
#include <algorithm>
#include <cstring>

using namespace nNet;
using namespace std;

void ConvolutionalFragmentHelper::GetFragmentInputSingleChannelOffset(ConvolutionalLayer *layer, ConvolutionalFragmentParams *params,
	int *inFragmentOffset, int *inFragmentLength) {
	int start = max(params->top - layer->padding, 0);
	*inFragmentOffset = start * layer->inWidth;
	int end = min(params->top + params->height + layer->padding, layer->inHeight);
	*inFragmentLength = (end - start) * layer->inWidth;
}

void ConvolutionalFragmentHelper::GetFragmentInputSingleChannelOffsetWithoutPadding(ConvolutionalLayer *layer, ConvolutionalFragmentParams *params,
	int *inFragmentOffset, int *inFragmentLength) {
	*inFragmentOffset = params->top * layer->inWidth;
	*inFragmentLength = params->height * layer->inWidth;
}

void ConvolutionalFragmentHelper::WriteFragmentInput(ConvolutionalLayer *layer, ConvolutionalFragmentParams *params, float *inFragment, int *inFragmentBytes) {
	int offset, length, ch;
	GetFragmentInputSingleChannelOffset(layer, params, &offset, &length);

	*inFragmentBytes = length * layer->inChannels * sizeof(float);

	float *input = layer->input;
	for (ch = 0; ch < layer->inChannels; ch++) {
		memcpy(
			&inFragment[ch * length],
			&input[(ch * layer->inputChannelCount) + offset],
			length * sizeof(float));
	}
}

void ConvolutionalFragmentHelper::ReadFragmentInput(ConvolutionalLayer *layer, ConvolutionalFragmentParams *params, float *inFragment) {
	int offset, length, ch;
	GetFragmentInputSingleChannelOffset(layer, params, &offset, &length);

	float *input = layer->input;
	for (ch = 0; ch < layer->inChannels; ch++) {
		memcpy(
			&input[(ch * layer->inputChannelCount) + offset],
			&inFragment[ch * length],
			length * sizeof(float));
	}
}

void ConvolutionalFragmentHelper::GetFragmentOutputCoords(ConvolutionalLayer *layer, ConvolutionalFragmentParams *params, int *outFragmentTop, int *outFragmentHeight) {
	// TODO
	*outFragmentTop = params->top;
	*outFragmentHeight = params->height;
}

void ConvolutionalFragmentHelper::GetFragmentOutputSingleFilterOffset(ConvolutionalLayer *layer, ConvolutionalFragmentParams *params, int *outFragmentOffset, int *outFragmentLength) {
	int top, height;
	GetFragmentOutputCoords(layer, params, &top, &height);

	*outFragmentOffset = top * layer->outWidth;
	*outFragmentLength = height * layer->outWidth;
}

void ConvolutionalFragmentHelper::WriteFragmentOutput(ConvolutionalLayer *layer, ConvolutionalFragmentParams *params, float *outFragment, int *outFragmentBytes) {
	int offset, length, f;
	GetFragmentOutputSingleFilterOffset(layer, params, &offset, &length);

	*outFragmentBytes = layer->filters * length * sizeof(float);

	float *output = layer->output;
	for (f = 0; f < layer->filters; f++) {
		memcpy(
			&outFragment[f * length],
			&output[f * layer->outputFilterCount + offset],
			length * sizeof(float));
	}
}

void ConvolutionalFragmentHelper::ReadFragmentOutput(ConvolutionalLayer *layer, ConvolutionalFragmentParams *params, float *outFragment) {
	int offset, length, f;
	GetFragmentOutputSingleFilterOffset(layer, params, &offset, &length);

	float *output = layer->output;
	for (f = 0; f < layer->filters; f++) {
		memcpy(
			&output[f * layer->outputFilterCount + offset],
			&outFragment[f * length],
			length * sizeof(float));
	}
}

void ConvolutionalFragmentHelper::SplitFragment(ConvolutionalFragmentParams *in, ConvolutionalFragmentParams *out, int divider, int n) {
	int part = in->height / divider;
	out->top = in->top + part * n;
	if (n == divider - 1) {
		out->height = in->height - n * part;
	}
	else {
		out->height = part;
	}
}
