#include "../nNet/ConvolutionalLayer.h"
#include "../nNet/MaxpoolLayer.h"
#include "RemoteConvolutionalBlockLayer.h"
#include "CreateConvBlockLayerCommand.h"
#include "ForwardConvBlockLayerCommand.h"
#include <stdexcept>

using namespace Client;

RemoteConvolutionalBlockLayer::RemoteConvolutionalBlockLayer(ConvolutionalBlockLayerParams *params, ClientsManager *clientsManager)
	: ConvolutionalBlockLayer::ConvolutionalBlockLayer(params) {
	_clientsManager = clientsManager;

	auto lastLayer = params->layers.back();
	output = new float[lastLayer->outputCount];
}

RemoteConvolutionalBlockLayer::~RemoteConvolutionalBlockLayer() {
	delete output;
}

void RemoteConvolutionalBlockLayer::Forward(vector<BlockLayerFragmentParams> *fragments) {
	throw logic_error("Not implemented.");
}

void RemoteConvolutionalBlockLayer::PropagateState() {
	CreateConvBlockLayerCommand command(this, _clientsManager);
	command.Execute();
}

void RemoteConvolutionalBlockLayer::Forward() {
	ForwardConvBlockLayerCommand command(this, _clientsManager);
	command.Execute();
}
