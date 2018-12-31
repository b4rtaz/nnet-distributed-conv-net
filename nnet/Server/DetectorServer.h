#pragma once
#include "../Transfer/TransferServer.h"
#include "../nNet/Network.h"

using namespace nNet;
using namespace Transfer;

namespace Server {

	class DetectorServer {
	private:
		TransferServer *_server;
		Network *_network;

	public:
		DetectorServer(TransferServer *server, Network *network);
		void Listen();

	private:
		void HandleClient(TransferClient *client);
	};
}
