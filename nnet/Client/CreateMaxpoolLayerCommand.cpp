#include "../Transfer/TransferException.h"
#include "CreateMaxpoolLayerCommand.h"
#include "Packets.h"

using namespace Client;
using namespace Transfer;

CreateMaxpoolLayerCommand::CreateMaxpoolLayerCommand(MaxpoolLayer *layer, ClientsManager *clientsManager) {
	_layer = layer;
	_clientsManager = clientsManager;
}

void CreateMaxpLayerDataHandler(void* handlerUserParam, int clientIndex, TransferPacket *responsePacketHeader, TransferClient *client) {
	((CreateMaxpoolLayerCommand*)handlerUserParam)->DataHandler(clientIndex, responsePacketHeader, client);
}

void FillMaxpParamsByLayer(MaxpoolLayer *layer, MaxpoolLayerParams *p) {
	p->inputWidth = layer->inWidth;
	p->inputHeight = layer->inHeight;
	p->inputChannels = layer->inChannels;
	p->size = layer->size;
	p->stride = layer->stride;
	p->padding = layer->padding;
}

void CreateMaxpoolLayerCommand::Execute() {
	int n = _clientsManager->GetClientCount();
	int n2 = n * 2;

	MaxpoolLayerParams params;
	FillMaxpParamsByLayer(_layer, &params);

	auto lengths = new int[n2];
	auto datas = new char*[n2];
	for (int i = 0; i < n2; i += 2) {
		lengths[i] = (int)sizeof(MaxpoolLayerParams);
		datas[i] = (char*)&params;
		lengths[i + 1] = (int)sizeof(int);
		datas[i + 1] = (char*)&_layer->index;
	}

	__try {
		_clientsManager->SendData(PACKET_ACTION_MAXP_CREATE, 2, lengths, datas, this, CreateMaxpLayerDataHandler);
	}
	__finally {
		delete[] lengths;
		delete[] datas;
	}
}

void CreateMaxpoolLayerCommand::DataHandler(int clientIndex, TransferPacket *header, TransferClient *client) {
	if (header->action == PACKET_ACTION_MAXP_CREATE) {
		if (header->dataCount != 0) {
			throw TransferException("Exepcted no data (create maxp layer command).");
		}
	}
}
