#pragma once
#include "../nNet/MultiCpuConvolutionalBlockLayer.h"

using namespace nNet;

namespace Server {

	class ServerConvolutionalBlockLayer : public MultiCpuConvolutionalBlockLayer {
	public:
		ServerConvolutionalBlockLayer(ConvolutionalBlockLayerParams *params);
		~ServerConvolutionalBlockLayer();
	};
}
