#pragma once
#include "../Transfer/TransferPacket.h"
#include "../Transfer/TransferClient.h"
#include "../nNet/Layer.h"
#include "../nNet/LayerFactory.h"
#include <vector>

using namespace Transfer;
using namespace nNet;
using namespace std;

namespace Server {

	class CreateConvBlockLayerCommandHandler {
	private:
		TransferClient *_client;
		vector<Layer*> *_layers;
		LayerFactory *_layerFactory;

	public:
		CreateConvBlockLayerCommandHandler(TransferClient *client, vector<Layer*> *layers, LayerFactory *layerFactory);
		void Handle(TransferPacket *header);
	};
}
