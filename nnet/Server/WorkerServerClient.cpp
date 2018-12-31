#include "../nNet/MultiCpuConvolutionalLayer.h"
#include "../Client/Packets.h"
#include "WorkerServerClient.h"
#include "CreateConvLayerCommandHandler.h"
#include "ForwardConvLayerCommandHandler.h"
#include "CreateMaxpoolLayerCommandHandler.h"
#include "CreateConvBlockLayerCommandHandler.h"
#include "ForwardConvBlockLayerCommandHandler.h"
#include <cstdio>
#include <stdexcept>
#include <memory>

using namespace Server;
using namespace std;

WorkerServerClient::WorkerServerClient(TransferClient *client, LayerFactory *layerFactory) {
	_client = client;
	_layerFactory = layerFactory;
}

WorkerServerClient::~WorkerServerClient() {
	auto it = _layers.begin();
	for (; it != _layers.end(); ++it) {
		delete *it;
	}
}

void WorkerServerClient::Listen() {
	while (true) {
		TransferPacket *packet = _client->WaitForHeader();
		__try {
			HandlePacket(packet);
		}
		__finally {
			delete packet;
		}
	}
}

void WorkerServerClient::HandlePacket(TransferPacket *header) {
	switch (header->action) {
	case PACKET_ACTION_HELLO:
		HandleHelloPacket();
		break;
	case PACKET_ACTION_CONV_CREATE:
	case PACKET_ACTION_CONV_SET_WEIGHTS:
	case PACKET_ACTION_CONV_SET_BIASES:
	case PACKET_ACTION_CONV_SET_ROLLING_MEAN:
	case PACKET_ACTION_CONV_SET_ROLLING_VARIANCE:
	case PACKET_ACTION_CONV_SET_SCALES:
		HandleCreateConvPacket(header);
		break;
	case PACKET_ACTION_CONV_FORWARD:
		HandleForwardConvPacket(header);
		break;
	case PACKET_ACTION_MAXP_CREATE:
		HandleCreateMaxpoolPacket(header);
		break;
	case PACKET_ACTION_CONV_BLOCK_CREATE:
		HandleCreateConvBlockPacket(header);
		break;
	case PACKET_ACTION_CONV_BLOCK_FORWARD:
		HandleForwardConvBlockPacket(header);
		break;
	default:
		throw logic_error("Unsuported packet.");
	}
}

void WorkerServerClient::HandleHelloPacket() {
	_client->SendHeader(PACKET_ACTION_HELLO, 0, NULL);
}

void WorkerServerClient::HandleCreateConvPacket(TransferPacket *header) {
	CreateConvLayerCommandHandler handler(_client, &_layers, _layerFactory);
	handler.Handle(header);
}

void WorkerServerClient::HandleForwardConvPacket(TransferPacket *header) {
	ForwardConvLayerCommandHandler handler(_client, &_layers);
	handler.Handle(header);
}

void WorkerServerClient::HandleCreateMaxpoolPacket(TransferPacket *header) {
	CreateMaxpoolLayerCommandHandler handler(_client, &_layers);
	handler.Handle(header);
}

void WorkerServerClient::HandleCreateConvBlockPacket(TransferPacket *header) {
	CreateConvBlockLayerCommandHandler handler(_client, &_layers, _layerFactory);
	handler.Handle(header);
}

void WorkerServerClient::HandleForwardConvBlockPacket(TransferPacket *header) {
	ForwardConvBlockLayerCommandHandler handler(_client, &_layers);
	handler.Handle(header);
}
