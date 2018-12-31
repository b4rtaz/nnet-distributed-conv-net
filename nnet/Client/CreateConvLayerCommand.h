#pragma once
#include "../nNet/ConvolutionalLayer.h"
#include "ClientsManager.h"
#include <vector>

using namespace nNet;
using namespace std;

namespace Client {

	class CreateConvLayerCommand {
	private:
		ConvolutionalLayer *_layer;
		ClientsManager *_clientsManager;

	public:
		CreateConvLayerCommand(ConvolutionalLayer *layer, ClientsManager *clientsManager);
		void Execute();
		void DataHandler(int clientIndex, TransferPacket *header, TransferClient *client);
	};
}
