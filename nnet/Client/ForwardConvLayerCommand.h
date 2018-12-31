#pragma once
#include "../nNet/ConvolutionalLayer.h"
#include "../nNet/Benchmark.h"
#include "ClientsManager.h"
#include <vector>

using namespace nNet;
using namespace std;

namespace Client {

	class ForwardConvLayerCommand {
	private:
		ConvolutionalLayer *_layer;
		ClientsManager *_clientsManager;

		vector<float> _totalSeconds;
		vector<float> _processSeconds;
		Benchmark _benchmark;

	public:
		ForwardConvLayerCommand(ConvolutionalLayer *layer, ClientsManager *clientsManager);
		void Execute();
		void DataHandler(int clientIndex, TransferPacket *responsePacketHeader, TransferClient *client);
	};
}
