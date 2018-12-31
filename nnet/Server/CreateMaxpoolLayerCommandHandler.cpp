#include "../Client/Packets.h"
#include "../Transfer/TransferException.h"
#include "../nNet/MaxpoolLayer.h"
#include "CreateMaxpoolLayerCommandHandler.h"

using namespace Server;
using namespace nNet;

CreateMaxpoolLayerCommandHandler::CreateMaxpoolLayerCommandHandler(TransferClient *client, vector<Layer*> *layers) {
	_client = client;
	_layers = layers;
}

void CreateMaxpoolLayerCommandHandler::Handle(TransferPacket *header) {
	if (header->dataCount != 2)
		throw TransferException("Invalid packet data count.");

	if (header->dataLengths[0] != sizeof(MaxpoolLayerParams))
		throw TransferException("Invalid data size, expected size of maxpool layer params.");
	MaxpoolLayerParams params;
	_client->ReadData((char*)&params, sizeof(MaxpoolLayerParams));

	int layerIndex = _client->ReadInt();

	auto layer = new MaxpoolLayer(&params);
	layer->index = layerIndex;
	_layers->push_back(layer);

	printf("%4d: maxp %4d %4d %4d\n", layerIndex, params.inputWidth, params.inputHeight, params.inputChannels);

	_client->SendHeader(PACKET_ACTION_MAXP_CREATE, 0, NULL);
}
