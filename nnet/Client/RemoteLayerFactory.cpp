#include "RemoteLayerFactory.h"
#include "RemoteConvolutionalLayer.h"
#include "RemoteConvolutionalBlockLayer.h"
#include <stdexcept>

using namespace Client;
using namespace std;

RemoteLayerFactory::RemoteLayerFactory(ClientsManager *clientsManager, bool withBlocks) {
	_clientsManager = clientsManager;
	_withBlocks = withBlocks;
}

ConvolutionalLayer* RemoteLayerFactory::CreateConvolutionalLayer(ConvolutionalLayerParams *params) {
	if (_withBlocks) {
		return LayerFactory::CreateConvolutionalLayer(params);
	}
	return new RemoteConvolutionalLayer(params, _clientsManager);
}

ConvolutionalBlockLayer* RemoteLayerFactory::CreateConvolutionalBlockLayer(ConvolutionalBlockLayerParams *params) {
	if (_withBlocks) {
		return new RemoteConvolutionalBlockLayer(params, _clientsManager);
	}
	throw logic_error("Cannot use block layer in without blocks mode.");
}
