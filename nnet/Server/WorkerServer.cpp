#pragma once
#include "../Transfer/TransferException.h"
#include "WorkerServer.h"
#include "WorkerServerClient.h"
#include <cstdio>
#include <memory>

using namespace std;
using namespace Server;

WorkerServer::WorkerServer(TransferServer *server, LayerFactory *layerFactory) {
	_server = server;
	_layerFactory = layerFactory;
}

void WorkerServer::Listen() {
	_server->Bind();

	while (true) {
		unique_ptr<TransferClient> client(_server->WaitForClient());
		unique_ptr<WorkerServerClient> remote(new WorkerServerClient(client.get(), _layerFactory));

		printf("* new client!\n");

		try {
			remote->Listen();
		} catch (const TransferException &e) {
			printf("Client disconnected. Exception: %s\n", e.what());
		}
	}
}
