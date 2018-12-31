#pragma once
#include "Layer.h"
#include "Activation.h"

namespace nNet {

	class ConvolutionalLayerHelper {
	public:
		static int GetOutputLength(int inLength, int padding, int size, int stride);
	};

	typedef struct {
		int inputWidth;
		int inputHeight;
		int inputChannels;
		int filters;
		int padding;
		int size;
		int stride;
		Activation activation;
		bool normalize;
	} ConvolutionalLayerParams;

	typedef struct {

		// Poni¿sze parametry okreœlaj¹ jaki wycinek wejœcia ma zostaæ przetworzony przez sieæ.
		//   ______________
		//   |            |
		//   |XXXXXXXXXXXX|
		//   |XXXXXXXXXXXX|
		//   |____________|
		int top;
		int height;
	} ConvolutionalFragmentParams;

	class ConvolutionalLayer: public Layer {
	public:
		Activation activation;

		int inWidth;
		int inHeight;
		int inChannels;

		int outWidth;
		int outHeight;
		int outChannels;

		int filters;
		int size;
		int stride;
		int padding;

		float *weights;
		int weightCount;
		float *biases;

		int inputChannelCount;
		int outputFilterCount;

		bool normalize;
		float *rollingMean;
		float *rollingVariance;
		float *scales;

		ConvolutionalLayer(ConvolutionalLayerParams *params);
		~ConvolutionalLayer() override;

		virtual void Forward(ConvolutionalFragmentParams *params) = 0;
	};
}
