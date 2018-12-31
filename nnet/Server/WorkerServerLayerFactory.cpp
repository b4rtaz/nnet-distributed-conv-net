#include "../nNet/SingleCpuConvolutionalLayer.h"
#include "WorkerServerLayerFactory.h"
#include "ServerMultiCpuConvolutionalLayer.h"
#include "ServerConvolutionalBlockLayer.h"
#include <cstdio>

using namespace nNet;
using namespace Server;
using namespace std;

WorkerServerLayerFactory::WorkerServerLayerFactory(bool withBlocks) {
	_withBlocks = withBlocks;
}

ConvolutionalLayer* WorkerServerLayerFactory::CreateConvolutionalLayer(ConvolutionalLayerParams *params) {
	if (_withBlocks) {
		printf("* created conv single cpu layer\n");
		return new SingleCpuConvolutionalLayer(params);
	}
	printf("* created conv multi cpu layer\n");
	return new ServerMultiCpuConvolutionalLayer(params, true);
}

ConvolutionalBlockLayer* WorkerServerLayerFactory::CreateConvolutionalBlockLayer(ConvolutionalBlockLayerParams *params) {
	return new ServerConvolutionalBlockLayer(params);
}
