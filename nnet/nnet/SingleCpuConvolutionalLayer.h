#pragma once
#include "ConvolutionalLayer.h"

namespace nNet {

	class SingleCpuConvolutionalLayer : public ConvolutionalLayer {
	private:
		float *_workspace;

	public:
		SingleCpuConvolutionalLayer(ConvolutionalLayerParams *params);
		~SingleCpuConvolutionalLayer() override;

		void Forward(ConvolutionalFragmentParams *params) override;
		void Forward() override;
	};
}
