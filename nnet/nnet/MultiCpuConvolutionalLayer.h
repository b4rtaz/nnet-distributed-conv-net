#pragma once
#include "ConvolutionalLayer.h"
#include <Windows.h>

namespace nNet {

	struct MultiCpuConvThreadInfo;

	class MultiCpuConvolutionalLayer : public ConvolutionalLayer {
	private:
		int _processors;
		MultiCpuConvThreadInfo *_threads;
		float *_workspace;

	public:
		MultiCpuConvolutionalLayer(ConvolutionalLayerParams *params);
		~MultiCpuConvolutionalLayer() override;

		void Forward(ConvolutionalFragmentParams *params) override;
		void Forward() override;
	};

	typedef struct MultiCpuConvThreadInfo {
		int no;
		ConvolutionalFragmentParams params;
		HANDLE thread;
		HANDLE jobLock;
		HANDLE doneLock;
		MultiCpuConvolutionalLayer *layer;
		float *workspace;
	} MultiCpuConvThreadInfo;
}
