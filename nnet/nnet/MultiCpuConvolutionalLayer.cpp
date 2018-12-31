#include "ConvolutionalLayer.h"
#include "MultiCpuConvolutionalLayer.h"
#include "FastCpuConvolutional.h"
#include <cstring>

using namespace nNet;

DWORD WINAPI MultiCpuConvThread(LPVOID param) {
	MultiCpuConvThreadInfo *info = (MultiCpuConvThreadInfo*)param;
	MultiCpuConvolutionalLayer *layer = info->layer;

	while (1) {
		WaitForSingleObject(info->jobLock, INFINITE);

		FastCpuConvolutional::Forward(layer, &info->params, info->workspace);

		ResetEvent(info->jobLock);
		SetEvent(info->doneLock);
	}
	return 0;
}

MultiCpuConvolutionalLayer::MultiCpuConvolutionalLayer(ConvolutionalLayerParams *params)
	: ConvolutionalLayer::ConvolutionalLayer(params)
{
	SYSTEM_INFO systemInfo;
	GetSystemInfo(&systemInfo);
	_processors = systemInfo.dwNumberOfProcessors;

	_threads = new MultiCpuConvThreadInfo[_processors];

	int workspaceSize = FastCpuConvolutional::GetWorkspaceSize(outWidth, outHeight, inChannels, size);
	_workspace = new float[workspaceSize];

	DWORD threadId;
	MultiCpuConvThreadInfo *i;
	for (int p = 0; p < _processors; p++) {
		i = &_threads[p];
		i->no = p;
		i->jobLock = CreateEvent(NULL, TRUE, FALSE, NULL);
		i->doneLock = CreateEvent(NULL, TRUE, FALSE, NULL);
		i->thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MultiCpuConvThread, (LPVOID)i, 0, &threadId);
		i->layer = this;
		i->workspace = _workspace;

		SetThreadAffinityMask(i->thread, 1LL << (p % _processors));
		SetThreadPriority(i->thread, THREAD_PRIORITY_TIME_CRITICAL);
	}
}

MultiCpuConvolutionalLayer::~MultiCpuConvolutionalLayer() {
	MultiCpuConvThreadInfo *i;
	for (int p = 0; p < _processors; p++) {
		i = &_threads[p];
		CloseHandle(i->thread);
		CloseHandle(i->doneLock);
		CloseHandle(i->jobLock);
	}

	delete[] _threads;
	delete[] _workspace;
}

void MultiCpuConvolutionalLayer::Forward(ConvolutionalFragmentParams *params) {
	MultiCpuConvThreadInfo *i;
	int p;
	for (p = 0; p < _processors; p++) {
		i = &_threads[p];
		ConvolutionalFragmentHelper::SplitFragment(params, &i->params, _processors, p);
		SetEvent(i->jobLock);
	}

	for (p = 0; p < _processors; p++) {
		i = &_threads[p];
		WaitForSingleObject(i->doneLock, INFINITE);
		ResetEvent(i->doneLock);
	}
}

void MultiCpuConvolutionalLayer::Forward() {
	ConvolutionalFragmentParams params;
	params.top = 0;
	params.height = inHeight;
	Forward(&params);
}
