#include "ServerMultiCpuConvolutionalLayer.h"

using namespace Server;

ServerMultiCpuConvolutionalLayer::ServerMultiCpuConvolutionalLayer(ConvolutionalLayerParams *params, bool allocateInput)
	: MultiCpuConvolutionalLayer::MultiCpuConvolutionalLayer(params) {
	_allocatedInput = allocateInput;
	if (allocateInput) {
		_input = new float[inputCount];
		input = _input;
	}
}

ServerMultiCpuConvolutionalLayer::~ServerMultiCpuConvolutionalLayer() {
	if (_allocatedInput) {
		delete[] _input;
	}
}
