#pragma once
#include "ConvolutionalLayer.h"
#include "ConvolutionalFragment.h"

namespace nNet {

	class FastCpuConvolutional {
	public:

		static int GetWorkspaceSize(int outWidth, int outHeight, int inChannels, int size);

		static void Forward(ConvolutionalLayer *layer, ConvolutionalFragmentParams *fragmentParams, float *workspace);
	};
}
