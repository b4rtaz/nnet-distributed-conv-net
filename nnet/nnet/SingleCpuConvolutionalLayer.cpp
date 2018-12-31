#include "ConvolutionalLayer.h"
#include "SingleCpuConvolutionalLayer.h"
#include "FastCpuConvolutional.h"
#include <cstring>

using namespace nNet;

SingleCpuConvolutionalLayer::SingleCpuConvolutionalLayer(ConvolutionalLayerParams *params)
	: ConvolutionalLayer::ConvolutionalLayer(params)
{
	int workspaceSize = FastCpuConvolutional::GetWorkspaceSize(outWidth, outHeight, inChannels, size);
	_workspace = new float[workspaceSize];
}

SingleCpuConvolutionalLayer::~SingleCpuConvolutionalLayer() {
	delete[] _workspace;
}

void SingleCpuConvolutionalLayer::Forward(ConvolutionalFragmentParams *params) {
	FastCpuConvolutional::Forward(this, params, _workspace);
}

void SingleCpuConvolutionalLayer::Forward() {
	ConvolutionalFragmentParams params;
	params.top = 0;
	params.height = inHeight;
	Forward(&params);
}
