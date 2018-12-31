#pragma once
#include "../Transfer/TransferClient.h"
#include "../Transfer/TransferPacket.h"
#include <Windows.h>
#include <vector>

using namespace std;
using namespace Transfer;

namespace Client {

	typedef void (*ClientInfoDataHandler)(void* handlerUserParam, int clientIndex, TransferPacket *header, TransferClient *client);

	class ClientInfo {
	public:
		TransferClient *client;

		HANDLE thread;
		HANDLE jobLock;
		HANDLE doneLock;
		int index;
		bool error;

		char action;
		int dataCountPerClient;
		int *dataLengths;
		char **datas;

		void *handlerUserParam;
		ClientInfoDataHandler handler;
	};

	class ClientsManager {
	private:
		vector<ClientInfo*> _clients;

	public:
		~ClientsManager();
		void AddClient(TransferClient *client);
		int GetClientCount();

		void SendData(char action, int dataCountPerClient, int *dataLengths, char **datas, void *handlerUserParam, ClientInfoDataHandler handler);
	};
}
