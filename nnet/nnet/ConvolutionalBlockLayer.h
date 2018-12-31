#pragma once
#include "Layer.h"
#include <vector>

using namespace std;

namespace nNet {

	class ConvolutionalBlockLayerParams {
	public:
		vector<Layer*> layers;
		bool forwardInput;
	};

	typedef struct {
		Layer *layer;

		int inFragmentTop;
		int inFragmentHeight;

		int offsetInTop;
		int offsetInBottom;
		int extendInFragmentTop;
		int extendInFragmentHeight;
	} BlockLayerFragmentParams;

	class ConvolutionalBlockLayer: public Layer {
	public:
		vector<Layer*> layers;

		ConvolutionalBlockLayer(ConvolutionalBlockLayerParams *params);
		~ConvolutionalBlockLayer();

		void Forward() override = 0; // ??
		virtual void Forward(vector<BlockLayerFragmentParams> *fragments) = 0;
	};

	class ConvolutionalBlockOffsetCalculator {
	public:
		static void Calculate(vector<Layer*> *layers, int *inFragmentTop, int *inFragmentHeight, vector<BlockLayerFragmentParams> *fragments);
	};
}
