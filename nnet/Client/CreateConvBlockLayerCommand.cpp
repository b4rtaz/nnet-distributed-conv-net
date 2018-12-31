#include "../nNet/Layer.h"
#include "../Transfer/TransferException.h"
#include "CreateConvBlockLayerCommand.h"
#include "CreateConvLayerCommand.h"
#include "CreateMaxpoolLayerCommand.h"
#include "Packets.h"

using namespace nNet;
using namespace Client;
using namespace Transfer;

CreateConvBlockLayerCommand::CreateConvBlockLayerCommand(ConvolutionalBlockLayer *block, ClientsManager *clientsManager) {
	_block = block;
	_clientsManager = clientsManager;
}

void CreateConvBlockDataHandler(void* handlerUserParam, int clientIndex, TransferPacket *responsePacketHeader, TransferClient *client) {
	((CreateConvBlockLayerCommand*)handlerUserParam)->DataHandler(clientIndex, responsePacketHeader, client);
}

void CreateRemoteConvBlockLayers(ConvolutionalBlockLayer *block, ClientsManager *cm) {
	for (auto i = block->layers.begin(); i != block->layers.end(); i++) {
		auto layer = (Layer*)*i;

		if (layer->layerType == CONVOLUTIONAL) {
			CreateConvLayerCommand convCommand((ConvolutionalLayer*)layer, cm);
			convCommand.Execute();
		}
		else if (layer->layerType == MAXPOOL) {
			CreateMaxpoolLayerCommand maxpCommand((MaxpoolLayer*)layer, cm);
			maxpCommand.Execute();
		}
	}
}

void CreateConvBlockLayerCommand::Execute() {
	CreateRemoteConvBlockLayers(_block, _clientsManager);

	auto firstLayer = _block->layers.front();
	auto lastLayer = _block->layers.back();

	int n = _clientsManager->GetClientCount();
	int n2 = n * 2;

	int *lengths = new int[n2];
	char **datas = new char*[n2];
	for (int i = 0; i < n2; i += 2) {
		lengths[i] = sizeof(int);
		datas[i] = (char*)&firstLayer->index;
		lengths[i + 1] = sizeof(int);
		datas[i + 1] = (char*)&lastLayer->index;
	}

	__try {
		_clientsManager->SendData(PACKET_ACTION_CONV_BLOCK_CREATE, 2, lengths, datas, NULL, CreateConvBlockDataHandler);
	}
	__finally {
		delete[] lengths;
		delete[] datas;
	}
}

void CreateConvBlockLayerCommand::DataHandler(int clientIndex, TransferPacket *header, TransferClient *client) {
	if (header->action == PACKET_ACTION_CONV_BLOCK_CREATE) {
		if (header->dataCount != 0) {
			throw TransferException("Invalid packet data count (create conv block).");
		}
	}
}
