#pragma once
#include "../Transfer/TransferPacket.h"
#include "../Transfer/TransferClient.h"
#include "../nNet/ConvolutionalFragment.h"
#include "../nNet/MultiCpuConvolutionalLayer.h"
#include "../nNet/LayerFactory.h"
#include <vector>

using namespace Transfer;
using namespace nNet;
using namespace std;

namespace Server {

	class CreateConvLayerCommandHandler {
	private:
		TransferClient *_client;
		vector<Layer*> *_layers;
		LayerFactory *_layerFactory;

	public:
		CreateConvLayerCommandHandler(TransferClient *client, vector<Layer*> *layers, LayerFactory *layerFactory);
		void Handle(TransferPacket *header);

	private:
		void HandleCreate(TransferPacket *header);
		void HandleSetWeights(TransferPacket *header);
	};

	class ConvLayerCommandHelper {
	public:
		static int ReadLayerIndex(TransferClient *client, int layerIndexLength);
		static ConvolutionalLayer *GetLayerByIndex(int index, vector<Layer*> *layers);
	};
}
