#pragma once
#include "Layer.h"

namespace nNet {

	typedef struct {
		int inputWidth;
		int inputHeight;
		int inputChannels;
		int size;
		int stride;
		int padding;
	} MaxpoolLayerParams;

	typedef struct {
		int top;
		int height;
	} MaxpoolFragmentParams;

	class MaxpoolLayer : public Layer {
	public:
		int inWidth;
		int inHeight;
		int inChannels;

		int outWidth;
		int outHeight;
		int outChannels;

		int outputChannelCount;

		int size;
		int stride;
		int padding;

	public:
		MaxpoolLayer(MaxpoolLayerParams *params);
		~MaxpoolLayer() override;

		void Forward() override;
		virtual void Forward(MaxpoolFragmentParams *params);
	};

	class MaxpoolFragmentHelper {
	public:
		static void GetFragmentOutputCoords(MaxpoolLayer *layer, MaxpoolFragmentParams *params, int *outFragmentTop, int *outFragmentHeight);

		static void GetFragmentOutputSingleFilterOffset(MaxpoolLayer *layer, MaxpoolFragmentParams *params, int *outFragmentOffset, int *outFragmentLength);
	};
}
