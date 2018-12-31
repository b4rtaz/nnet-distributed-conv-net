#include "CreateConvLayerCommand.h"
#include "ForwardConvLayerCommand.h"
#include "RemoteConvolutionalLayer.h"
#include <cstdlib>

using namespace Client;

RemoteConvolutionalLayer::RemoteConvolutionalLayer(ConvolutionalLayerParams *params, ClientsManager *clientsManager)
	: ConvolutionalLayer::ConvolutionalLayer(params) {
	_clientsManager = clientsManager;
}

void RemoteConvolutionalLayer::PropagateState() {
	CreateConvLayerCommand command(this, _clientsManager);
	command.Execute();
}

void RemoteConvolutionalLayer::Forward(ConvolutionalFragmentParams *params) {
	throw logic_error("Not implemented.");
}

void RemoteConvolutionalLayer::Forward() {
	ForwardConvLayerCommand command(this, _clientsManager);
	command.Execute();
}
