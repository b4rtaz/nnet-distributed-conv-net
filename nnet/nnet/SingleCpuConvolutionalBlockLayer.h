#pragma once
#include "ConvolutionalFragment.h"
#include "ConvolutionalBlockLayer.h"
#include "ConvolutionalBlockLayer.h"

namespace nNet {

	class SingleCpuConvolutionalBlockLayer : public ConvolutionalBlockLayer {
	public:
		SingleCpuConvolutionalBlockLayer(ConvolutionalBlockLayerParams *params);

		void Forward(vector<BlockLayerFragmentParams> *fragments) override;
		void Forward() override;
	};
}
