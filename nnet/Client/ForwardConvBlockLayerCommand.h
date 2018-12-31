#pragma once
#include "../nNet/ConvolutionalBlockLayer.h"
#include "../nNet/Benchmark.h"
#include "ClientsManager.h"
#include <vector>

using namespace nNet;
using namespace std;

namespace Client {

	class ForwardConvBlockLayerCommand {
	private:
		ConvolutionalBlockLayer *_block;
		ClientsManager *_clientsManager;
		Benchmark _benchmark;
		vector<float> _totalSeconds;
		vector<float> _processSeconds;
		vector<int> _bytes;

	public:

		ForwardConvBlockLayerCommand(ConvolutionalBlockLayer *block, ClientsManager *clientsManager);
		void Execute();
		void DataHandler(int clientIndex, TransferPacket *header, TransferClient *client);
	};
}
