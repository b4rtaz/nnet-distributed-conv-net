#include "../nNet/ConvolutionalLayer.h"
#include "../nNet/ConvolutionalBlockLayer.h"
#include "../nNet/YoloRegionLayer.h"
#include "../Transfer/TransferException.h"
#include "DetectorServer.h"
#include <cstdio>
#include <memory>

using namespace Server;
using namespace nNet;
using namespace std;

DetectorServer::DetectorServer(TransferServer *server, Network *network) {
	_server = server;
	_network = network;
}

void DetectorServer::Listen() {
	_server->Bind();

	while (true) {
		unique_ptr<TransferClient>client(_server->WaitForClient());

		try {
			HandleClient(client.get());
		}
		catch (const TransferException &e) {
			printf("Client disconnected: %s\n", e.what());
		}
	}
}

void DetectorServer::HandleClient(TransferClient *client) {
	int inCount;
	int inWidth;
	int inHeight;
	int inChannels;

	Layer *firstLayer = _network->layers.front();
	if (firstLayer->layerType == BLOCK) {
		firstLayer = ((ConvolutionalBlockLayer*)firstLayer)->layers.front();
	}
	if (firstLayer->layerType == CONVOLUTIONAL) {
		auto conv = (ConvolutionalLayer*)firstLayer;
		inCount = conv->inputCount;
		inWidth = conv->inWidth;
		inHeight = conv->inHeight;
		inChannels = conv->inChannels;
	}

	YoloRegionLayer *lastLayer = (YoloRegionLayer*)_network->layers.back();

	client->SendData((char*)&inWidth, sizeof(int));
	client->SendData((char*)&inHeight, sizeof(int));
	client->SendData((char*)&inChannels, sizeof(int));

	unique_ptr<float[]> input(new float[inCount]);
	int imageWidth;
	int imageHeight;
	int regionCount;
	int r;
	YoloRegion region;

	while (true)
	{
		client->ReadData((char*)&imageWidth, sizeof(int));
		client->ReadData((char*)&imageHeight, sizeof(int));
		client->ReadData((char*)input.get(), inCount * sizeof(float));

		_network->Forward(input.get());

		unique_ptr<YoloRegion[]> regions(lastLayer->ReadRegions(imageWidth, imageHeight, inWidth, inHeight, 0.6f, &regionCount));
		client->SendData((char*)&regionCount, sizeof(int));

		printf("* regions: %d\n", regionCount);

		for (r = 0; r < regionCount; r++) {
			region = regions[r];
			client->SendData((char*)&region.classIndex, sizeof(int));
			client->SendData((char*)&region.probability, sizeof(float));
			client->SendData((char*)&region.box.x, sizeof(float));
			client->SendData((char*)&region.box.y, sizeof(float));
			client->SendData((char*)&region.box.w, sizeof(float));
			client->SendData((char*)&region.box.h, sizeof(float));
		}
	}
}
