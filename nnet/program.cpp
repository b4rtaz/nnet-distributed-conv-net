#include "Tests/Tests.h"
#include "Server/WorkerServer.h"
#include "Server/DetectorServer.h"
#include "Server/WorkerServerLayerFactory.h"
#include "Client/ClientsManager.h"
#include "Client/RemoteLayerFactory.h"
#include "nNet/LayerFactory.h"
#include "nNet/YoloNetParser.h"
#include "nNet/YoloWeightsLoader.h"
#include "nNet/BlockLayerInjector.h"
#include "Transfer/SocketTransferServer.h"
#include "Transfer/SocketTransferClient.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <exception>
#include <stdexcept>

using namespace std;
using namespace Server;
using namespace Transfer;
using namespace Client;

#define NNET_VERSION 0.50

void RunWorkerServer(int port, char *blocks) {
	bool withBlocks = strcmp(blocks, "single") != 0;

	WorkerServerLayerFactory layerFactory(withBlocks);
	SocketTransferServer transferServer(port);
	WorkerServer server(&transferServer, &layerFactory);
	server.Listen();
}

void ParseWorkerHosts(ClientsManager *cm, char **args, int argc) {
	for (int c = 0; c < argc; c++) {
		char *word = args[c];
		char *pos = strstr(word, ":");
		*pos = '\0';
		int port = atoi(pos + 1);
		cm->AddClient(new SocketTransferClient(word, port));
	}
}

void ParseBlocks(BlockLayerInjector *injector, Network *network, char *blocks) {
	char *i = blocks;

	do {
		char *a = i;
		char *b = strstr(i, "-") + 1;
		int from = atoi(a);
		int to = atoi(b);

		printf("~ block <%d; %d>\n", from, to);
		injector->Inject(&network->layers, from, to);
		i = strstr(b, ",");
		if (i != NULL) i++;
	}
	while (i != NULL);
}

void RunDetectorServer(char *netPath, char *weightsPath, char *portStr, char *blocks, char **args, int argc) {
	unique_ptr<ClientsManager> clientManager;
	unique_ptr<LayerFactory> layerFactory;

	bool withBlocks = strcmp(blocks, "single") != 0;

	if (argc > 0) {
		clientManager.reset(new ClientsManager());
		ParseWorkerHosts(clientManager.get(), args, argc);
		layerFactory.reset(new RemoteLayerFactory(clientManager.get(), withBlocks));
	}
	else {
		layerFactory.reset(new LayerFactory());
	}

	YoloNetParser yoloNetParser(true, layerFactory.get());
	unique_ptr<Network> network(yoloNetParser.Read(netPath));
	YoloWeightsLoader::Read(network.get(), weightsPath);

	if (withBlocks) {
		BlockLayerInjector injector(layerFactory.get());
		ParseBlocks(&injector, network.get(), blocks);
	}

	network->PropagateState();

	int port = atoi(portStr);
	SocketTransferServer server(port);
	DetectorServer detector(&server, network.get());
	detector.Listen();
}

int main(int argc, char **argv) {
	printf("        _   _      _   \n");
	printf("  _ __ | \\ | | ___| |_ \n");
	printf(" | '_ \\|  \\| |/ _ \\ __|\n");
	printf(" | | | | |\\  |  __/ |_ \n");
	printf(" |_| |_|_| \\_|\\___|\\__|\n");
	printf("~~~~~~~~~~~~~~~~~~~~~~~\n");
	printf("          %.2f\n", NNET_VERSION);
	printf("~~~~~~~~~~~~~~~~~~~~~~~\n");

	try {
		if (argc > 1) {
			if (strcmp(argv[1], "serve-worker") == 0 && argc == 4) {
				int port = (int)strtol(argv[2], NULL, 10);
				RunWorkerServer(port, argv[3]);
			}
			else if (strcmp(argv[1], "serve-detector") == 0 && argc >= 6) {
				RunDetectorServer(argv[2], argv[3], argv[4], argv[5], &argv[6], argc - 6);
			}
			else if (strcmp(argv[1], "test") == 0) {
				Tests::RunTests();
			}
			else {
				printf("usage:\n");
				printf("nnet.exe serve-detector [cfg] [weights] [port] [single/1-5,6-12] ?[ip:port]\n");
				printf("nnet.exe serve-worker [port] [single/block]\n");
			}
		}
	}
	catch (const exception &e) {
		printf("Uncatched exception: %s\n", e.what());
	}

	//printf("Press any key to exit...\n");
	//getchar();
	return 0;
}
