#pragma once
#include "../Transfer/TransferPacket.h"
#include "../Transfer/TransferClient.h"
#include "../nNet/Layer.h"
#include "../nNet/ConvolutionalFragment.h"
#include <vector>

using namespace Transfer;
using namespace nNet;
using namespace std;

namespace Server {

	class ForwardConvBlockLayerCommandHandler {
	private:
		TransferClient *_client;
		vector<Layer*> *_layers;

	public:
		ForwardConvBlockLayerCommandHandler(TransferClient *client, vector<Layer*> *layers);
		void Handle(TransferPacket *header);
	};
}
