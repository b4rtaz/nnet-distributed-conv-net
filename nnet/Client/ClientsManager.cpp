#include "../Transfer/TransferException.h"
#include "ClientsManager.h"
#include "Packets.h"
#include <stdexcept>
#include <ctime>

using namespace Client;
using namespace std;

void SendDataToRemoteClient(ClientInfo *rc) {
	int offset = rc->index * rc->dataCountPerClient;
	rc->client->SendHeader(rc->action, rc->dataCountPerClient, &rc->dataLengths[offset]);

	for (int i = 0; i < rc->dataCountPerClient; i++) {
		rc->client->SendData(rc->datas[offset + i], rc->dataLengths[offset + i]);
	}

	TransferPacket *header = rc->client->WaitForHeader();
	__try {
		if (rc->handler != NULL) {
			rc->handler(rc->handlerUserParam, rc->index, header, rc->client);
		}
		else if (header->dataCount > 0) {
			throw new TransferException("Not setted handler.");
		}
	}
	__finally {
		delete header;
	}
}

DWORD WINAPI RemoteClientThread(LPVOID param) {
	auto rc = (ClientInfo*)param;

	bool run = true;
	while (run) {
		WaitForSingleObject(rc->jobLock, INFINITE);

		try {
			SendDataToRemoteClient(rc);
		}
		catch (const TransferException &e) {
			printf("Transfer error during sending data to client. Exception: %s\n", e.what());
			rc->error = true;
			run = false;
		}

		ResetEvent(rc->jobLock);
		SetEvent(rc->doneLock);
	}
	return 0;
}

ClientsManager::~ClientsManager() {
	auto it = _clients.begin();
	for (; it != _clients.end(); ++it) {
		ClientInfo *client = *it;

		CloseHandle(client->jobLock);
		CloseHandle(client->doneLock);
		CloseHandle(client->thread);
		delete client;
	}
}

void ClientsManager::AddClient(TransferClient *client) {
	ClientInfo *rc = new ClientInfo();
	rc->client = client;

	clock_t start = clock();

	client->SendHeader(PACKET_ACTION_HELLO, 0, NULL);

	TransferPacket *packet = client->WaitForHeader();
	__try {
		if (packet->action != PACKET_ACTION_HELLO || packet->dataCount != 0)
			throw logic_error("Invalid return packet from client.");
	}
	__finally {
		delete packet;
	}

	printf("ping in %.5f seconds\n", (double)(clock() - start) / CLOCKS_PER_SEC);

	DWORD threadId;

	rc->index = (int)_clients.size();
	rc->jobLock = CreateEvent(NULL, TRUE, FALSE, NULL);
	rc->doneLock = CreateEvent(NULL, TRUE, FALSE, NULL);
	rc->thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RemoteClientThread, (LPVOID)rc, 0, &threadId);
	rc->error = false;

	_clients.push_back(rc);
}

int ClientsManager::GetClientCount() {
	return (int)_clients.size();
}

void ClientsManager::SendData(char action, int dataCountPerClient, int *dataLengths, char **datas, void *handlerUserParam, ClientInfoDataHandler handler) {
	int n = (int)_clients.size();
	int i;

	for (i = 0; i < n; i++) {
		ClientInfo *client = _clients[i];
		client->index = i;

		client->action = action;
		client->dataCountPerClient = dataCountPerClient;
		client->dataLengths = dataLengths;
		client->datas = datas;

		client->handlerUserParam = handlerUserParam;
		client->handler = handler;
		SetEvent(client->jobLock);
	}

	for (i = 0; i < n; i++) {
		ClientInfo *client = _clients[i];
		WaitForSingleObject(client->doneLock, INFINITE);

		if (client->error)
			throw TransferException("Error during sending data.");

		ResetEvent(client->doneLock);
	}
}
