#pragma once
#include "ConvolutionalLayer.h"

namespace nNet {

	class ConvolutionalFragmentHelper {
	public:

		static void GetFragmentInputSingleChannelOffset(ConvolutionalLayer *layer, ConvolutionalFragmentParams *params, int *inFragmentOffset, int *inFragmentLength);
		
		static void GetFragmentInputSingleChannelOffsetWithoutPadding(ConvolutionalLayer *layer, ConvolutionalFragmentParams *params, int *inFragmentOffset, int *inFragmentLength);

		static void WriteFragmentInput(ConvolutionalLayer *layer, ConvolutionalFragmentParams *params, float *inFragment, int *inFragmentBytes);

		static void ReadFragmentInput(ConvolutionalLayer *layer, ConvolutionalFragmentParams *params, float *inFragment);

		static void GetFragmentOutputCoords(ConvolutionalLayer *layer, ConvolutionalFragmentParams *params, int *outFragmentTop, int *outFragmentHeight);

		static void GetFragmentOutputSingleFilterOffset(ConvolutionalLayer *layer, ConvolutionalFragmentParams *params, int *outFragmentOffset, int *outFragmentLength);

		static void WriteFragmentOutput(ConvolutionalLayer *layer, ConvolutionalFragmentParams *params, float *outFragment, int *outFragmentBytes);

		static void ReadFragmentOutput(ConvolutionalLayer *layer, ConvolutionalFragmentParams *params, float *outFragment);

		static void SplitFragment(ConvolutionalFragmentParams *in, ConvolutionalFragmentParams *out, int divider, int n);
	};
}
