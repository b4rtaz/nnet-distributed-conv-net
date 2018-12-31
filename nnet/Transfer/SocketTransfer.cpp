#include "SocketTransfer.h"
#include "TransferException.h"
#include <cstdio>
#include <winsock.h>
#pragma comment(lib, "ws2_32.lib")

using namespace Transfer;
using namespace std;

static bool _wsaInitialized = false;

void SocketUtils::InitIfNeed() {
	if (_wsaInitialized) {
		return;
	}

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		throw TransferException("Cannot start WSA.");
	}
	_wsaInitialized = true;
}

SocketTransfer SocketUtils::Connect(char *host, int port) {
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) {
		throw TransferException("Cannot create client socket.");
	}

	struct sockaddr_in addr;
	addr.sin_addr.s_addr = inet_addr(host);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		closesocket(sock);
		throw TransferException("Cannot connect to server.");
	}
	return sock;
}

SocketTransfer SocketUtils::Bind(int port) {
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) {
		throw TransferException("Cannot create server socket.");
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);

	if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) {
		closesocket(sock);
		throw TransferException("Cannot bind socket.");
	}

	listen(sock, 0);
	return sock;
}

SocketTransfer SocketUtils::WaitForClient(SocketTransfer socket) {
	struct sockaddr_in addr;
	int addrSize = sizeof(struct sockaddr_in);
	SOCKET sock = accept(socket, (struct sockaddr*)&addr, &addrSize);
	if (sock == INVALID_SOCKET) {
		throw TransferException("Invalid client socket.");
	}
	return sock;
}

void SocketUtils::Recv(SocketTransfer socket, char *buffer, int size) {
	int read;
	while (size > 0) {
		read = recv(socket, buffer, size, 0);
		if (read <= 0) {
			printf("<trace> recv error, read = %d, wsa = %d\n", read, WSAGetLastError());
			throw TransferException("Cannot read data from socket.");
		}
		size -= read;
		buffer += read;
	}
}

void SocketUtils::Send(SocketTransfer socket, char* data, int size) {
	int sent;
	while (size > 0) {
		sent = send(socket, data, size, 0);
		if (send <= 0) {
			printf("<trace> send error, sent = %d, wsa = %d\n", sent, WSAGetLastError());
			throw TransferException("Cannot send data to socket.");
		}
		size -= sent;
		data += sent;
	};
}
