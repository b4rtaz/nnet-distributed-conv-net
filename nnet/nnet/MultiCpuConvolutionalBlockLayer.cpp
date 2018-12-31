#include "MultiCpuConvolutionalBlockLayer.h"
#include "ConvolutionalFragment.h"
#include "MaxpoolLayer.h"
#include "Layer.h"

using namespace nNet;
using namespace std;

void MultiCpuConvBlockForward(MultiCpuConvBlockThreadInfo *info) {
	if (info->layer->layerType == CONVOLUTIONAL) {
		ConvolutionalFragmentParams convParams;
		convParams.top = info->fragment->inFragmentTop;
		convParams.height = info->fragment->inFragmentHeight;

		auto convLayer = (ConvolutionalLayer*)info->layer;
		convLayer->Forward(&convParams);
	}
	else if (info->layer->layerType == MAXPOOL) {
		MaxpoolFragmentParams maxpParams;
		maxpParams.top = info->fragment->inFragmentTop;
		maxpParams.height = info->fragment->inFragmentHeight;
		auto maxpLayer = (MaxpoolLayer*)info->layer;
		maxpLayer->Forward(&maxpParams);
	}
}

DWORD WINAPI MultiCpuConvBlockThread(LPVOID param) {
	MultiCpuConvBlockThreadInfo *info = (MultiCpuConvBlockThreadInfo*)param;

	while (true) {
		WaitForSingleObject(info->jobLock, INFINITE);

		MultiCpuConvBlockForward(info);

		ResetEvent(info->jobLock);
		SetEvent(info->doneLock);
	}
	return 0;
}

MultiCpuConvolutionalBlockLayer::MultiCpuConvolutionalBlockLayer(ConvolutionalBlockLayerParams *params)
	: ConvolutionalBlockLayer::ConvolutionalBlockLayer(params) {
	int p;
	MultiCpuConvBlockThreadInfo *i;
	DWORD threadId;

	SYSTEM_INFO systemInfo;
	GetSystemInfo(&systemInfo);
	_processors = systemInfo.dwNumberOfProcessors;

	_jobLocks = new HANDLE[_processors];
	_doneLocks = new HANDLE[_processors];
	_threadInfos = new MultiCpuConvBlockThreadInfo[_processors];

	for (p = 0; p < _processors; p++) {
		_jobLocks[p] = CreateEvent(NULL, TRUE, FALSE, NULL);
		_doneLocks[p] = CreateEvent(NULL, TRUE, FALSE, NULL);

		i = &_threadInfos[p];
		i->no = p;
		i->jobLock = _jobLocks[p];
		i->doneLock = _doneLocks[p];
		i->thread = CreateThread(NULL, 0, MultiCpuConvBlockThread, (LPVOID)i, 0, &threadId);

		SetThreadAffinityMask(i->thread, 1LL << (p % _processors));
		SetThreadPriority(i->thread, THREAD_PRIORITY_TIME_CRITICAL);
	}
}

MultiCpuConvolutionalBlockLayer::~MultiCpuConvolutionalBlockLayer() {
	MultiCpuConvBlockThreadInfo *i;
	int p;
	for (p = 0; p < _processors; p++) {
		i = &_threadInfos[p];
		CloseHandle(i->thread);

		CloseHandle(_jobLocks[p]);
		CloseHandle(_doneLocks[p]);
	}
	delete[] _jobLocks;
	delete[] _doneLocks;
	delete[] _threadInfos;
}

void MultiCpuConvolutionalBlockLayer::Forward() {
	ConvolutionalLayer *first = (ConvolutionalLayer*)layers.front();
	vector<BlockLayerFragmentParams> fragments;
	int top = 0;
	int height = first->inHeight;
	ConvolutionalBlockOffsetCalculator::Calculate(&layers, &top, &height, &fragments);
	Forward(&fragments);
}

void MultiCpuConvolutionalBlockLayer::Forward(vector<BlockLayerFragmentParams> *fragments) {
	auto firstFragment = fragments->front();

	ConvolutionalFragmentParams zeroParams;
	zeroParams.top = firstFragment.extendInFragmentTop;
	zeroParams.height = firstFragment.extendInFragmentHeight;

	MultiCpuConvBlockThreadInfo *i;
	int p, l;
	for (p = 0; p < _processors; p++) {
		i = &_threadInfos[p];

		ConvolutionalFragmentParams partParams;
		ConvolutionalFragmentHelper::SplitFragment(&zeroParams, &partParams, _processors, p);

		i->fragments.clear();
		ConvolutionalBlockOffsetCalculator::Calculate(&layers, &partParams.top, &partParams.height, &i->fragments);
	}

	float *in = input;

	for (l = 0; l < layers.size(); l++) {
		auto layer = layers[l];
		layer->input = in;

		for (p = 0; p < _processors; p++) {
			i = &_threadInfos[p];

			i->layer = layer;
			i->fragment = &i->fragments[l];
			SetEvent(i->jobLock);
		}

		WaitForMultipleObjects(_processors, _doneLocks, true, INFINITE);
		for (p = 0; p < _processors; p++) {
			ResetEvent(_threadInfos[p].doneLock);
		}
		in = layer->output;
	}

	output = in;
}
