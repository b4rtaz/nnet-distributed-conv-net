#pragma once
#include "../nNet/MultiCpuConvolutionalLayer.h"

using namespace nNet;

namespace Server {

	class ServerMultiCpuConvolutionalLayer : public MultiCpuConvolutionalLayer {
	private:
		bool _allocatedInput;
		float *_input;

	public:
		ServerMultiCpuConvolutionalLayer(ConvolutionalLayerParams *params, bool allocateInput);
		~ServerMultiCpuConvolutionalLayer();
	};
}
