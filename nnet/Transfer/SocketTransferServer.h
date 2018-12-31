#pragma once
#include "TransferServer.h"
#include "TransferClient.h"
#include "SocketTransfer.h"

namespace Transfer {

	class SocketTransferServer: public TransferServer {
	private:
		int _port;
		SocketTransfer _socket;

	public:
		SocketTransferServer(int port);
		void Bind() override;
		TransferClient* WaitForClient() override;
	};
}
