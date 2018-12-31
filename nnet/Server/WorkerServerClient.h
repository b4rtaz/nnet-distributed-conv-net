#pragma once
#include "../nNet/Layer.h"
#include "../nNet/LayerFactory.h"
#include "../Transfer/TransferClient.h"
#include <vector>

using namespace std;
using namespace nNet;
using namespace Transfer;

namespace Server {

	class WorkerServerClient {
	private:
		TransferClient *_client;
		vector<Layer*> _layers;
		LayerFactory *_layerFactory;

	public:
		WorkerServerClient(TransferClient *client, LayerFactory *layerFactory);
		~WorkerServerClient();

		void Listen();
		void HandlePacket(TransferPacket *header);

	private:
		void HandleHelloPacket();
		void HandleCreateConvPacket(TransferPacket *header);
		void HandleForwardConvPacket(TransferPacket *header);
		void HandleCreateMaxpoolPacket(TransferPacket *header);
		void HandleCreateConvBlockPacket(TransferPacket *header);
		void HandleForwardConvBlockPacket(TransferPacket *header);
	};
}
