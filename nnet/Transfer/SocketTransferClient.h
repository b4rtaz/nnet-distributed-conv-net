#pragma once
#include "SocketTransfer.h"
#include "TransferClient.h"

namespace Transfer {

	class SocketTransferClient: public TransferClient {
	private:
		SocketTransfer _socket;

	public:
		SocketTransferClient(char *host, int port);
		SocketTransferClient(SocketTransfer socket);
		~SocketTransferClient();

		TransferPacket* WaitForHeader() override;

		void SendHeader(char action, int dataCount, int *lengths) override;
		void SendHeader(TransferPacket *packet) override;

		void SendData(char *data, int length) override;
		void ReadData(char *data, int length) override;
		int ReadInt() override;
	};
}
