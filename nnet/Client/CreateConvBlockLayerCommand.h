#pragma once
#include "../nNet/ConvolutionalBlockLayer.h"
#include "ClientsManager.h"

using namespace nNet;

namespace Client {

	class CreateConvBlockLayerCommand {
	private:
		ConvolutionalBlockLayer *_block;
		ClientsManager *_clientsManager;

	public:
		CreateConvBlockLayerCommand(ConvolutionalBlockLayer *block, ClientsManager *clientsManager);
		void Execute();
		void DataHandler(int clientIndex, TransferPacket *header, TransferClient *client);
	};
}
