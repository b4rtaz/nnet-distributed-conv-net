#pragma once
#include "../nNet/LayerFactory.h"
#include "../Transfer/TransferServer.h"

using namespace Transfer;
using namespace nNet;

namespace Server {

	class WorkerServer {
	private:
		TransferServer *_server;
		LayerFactory *_layerFactory;

	public:
		WorkerServer(TransferServer *server, LayerFactory *layerFactory);
		void Listen();
	};
}
