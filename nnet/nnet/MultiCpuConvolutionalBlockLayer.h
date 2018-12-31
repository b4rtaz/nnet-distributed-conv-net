#pragma once
#include "ConvolutionalFragment.h"
#include "ConvolutionalBlockLayer.h"
#include "ConvolutionalBlockLayer.h"
#include <Windows.h>

namespace nNet {

	struct MultiCpuConvBlockThreadInfo;

	class MultiCpuConvolutionalBlockLayer: public ConvolutionalBlockLayer {
	private:
		int _processors;
		MultiCpuConvBlockThreadInfo *_threadInfos;
		HANDLE *_jobLocks;
		HANDLE *_doneLocks;

	public:
		MultiCpuConvolutionalBlockLayer(ConvolutionalBlockLayerParams *params);
		~MultiCpuConvolutionalBlockLayer();

		void Forward(vector<BlockLayerFragmentParams> *fragments) override;
		void Forward() override;
	};

	typedef struct MultiCpuConvBlockThreadInfo {
		int no;
		HANDLE jobLock;
		HANDLE doneLock;
		HANDLE thread;

		vector<BlockLayerFragmentParams> fragments;

		Layer *layer;
		BlockLayerFragmentParams *fragment;
	} MultiCpuConvBlockThreadInfo;
}
