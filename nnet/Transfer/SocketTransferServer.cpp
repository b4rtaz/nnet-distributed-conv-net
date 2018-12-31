#include "SocketTransferServer.h"
#include "SocketTransferClient.h"

using namespace Transfer;

SocketTransferServer::SocketTransferServer(int port) {
	_port = port;
}

void SocketTransferServer::Bind() {
	SocketUtils::InitIfNeed();
	_socket = SocketUtils::Bind(_port);
}

TransferClient* SocketTransferServer::WaitForClient() {
	SOCKET sock = SocketUtils::WaitForClient(_socket);
	return new SocketTransferClient(sock);
}
