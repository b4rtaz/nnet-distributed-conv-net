#pragma once
#include "../nNet/MaxpoolLayer.h"
#include "ClientsManager.h"
#include <vector>

using namespace nNet;
using namespace std;

namespace Client {

	class CreateMaxpoolLayerCommand {
	private:
		MaxpoolLayer *_layer;
		ClientsManager *_clientsManager;

	public:
		CreateMaxpoolLayerCommand(MaxpoolLayer *layer, ClientsManager *clientsManager);
		void Execute();
		void DataHandler(int clientIndex, TransferPacket *responsePacketHeader, TransferClient *client);
	};
}
