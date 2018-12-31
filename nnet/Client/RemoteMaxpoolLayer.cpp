#include "RemoteMaxpoolLayer.h"
#include "CreateMaxpoolLayerCommand.h"
#include <cstdlib>

using namespace Client;

RemoteMaxpoolLayer::RemoteMaxpoolLayer(MaxpoolLayerParams *params, ClientsManager *clientsManager)
	: MaxpoolLayer::MaxpoolLayer(params) {
	_clientsManager = clientsManager;
}

void RemoteMaxpoolLayer::PropagateState() {
	CreateMaxpoolLayerCommand command(this, _clientsManager);
	command.Execute();
}

void RemoteMaxpoolLayer::Forward(MaxpoolFragmentParams *params) {
	throw logic_error("Not implemented.");
}

void RemoteMaxpoolLayer::Forward() {
	throw logic_error("Not implemented.");
}
