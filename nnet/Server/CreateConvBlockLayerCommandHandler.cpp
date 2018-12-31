#include "../nNet/BlockLayerInjector.h"
#include "../Client/Packets.h"
#include "../Transfer/TransferException.h"
#include "CreateConvBlockLayerCommandHandler.h"
#include "WorkerServerLayerFactory.h"

using namespace Server;
using namespace nNet;

CreateConvBlockLayerCommandHandler::CreateConvBlockLayerCommandHandler(TransferClient *client, vector<Layer*> *layers, LayerFactory *layerFactory) {
	_client = client;
	_layers = layers;
	_layerFactory = layerFactory;
}

void CreateConvBlockLayerCommandHandler::Handle(TransferPacket *header) {
	if (header->dataCount != 2 || header->dataLengths[0] != sizeof(int) || header->dataLengths[1] != sizeof(int)) {
		throw TransferException("Invalid packet size.");
	}

	int firstIndex = _client->ReadInt();
	int lastIndex = _client->ReadInt();

	BlockLayerInjector injector(_layerFactory);

	auto blockLayer = injector.Inject(_layers, firstIndex, lastIndex);
	if (blockLayer == NULL) {
		throw TransferException("Cannot create block.");
	}

	int layerCount = (int)blockLayer->layers.size();
	if (layerCount != lastIndex - firstIndex + 1) {
		throw TransferException("Invalid block composition.");
	}
	
	printf("      block #%d, layers: %d - %d\n", lastIndex, firstIndex, lastIndex);

	_client->SendHeader(PACKET_ACTION_CONV_BLOCK_CREATE, 0, NULL);
}
