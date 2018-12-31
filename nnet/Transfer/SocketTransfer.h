#pragma once
#include <winsock.h>

namespace Transfer {

	typedef SOCKET SocketTransfer;

	class SocketUtils {
	public:
		static void InitIfNeed();

		static SocketTransfer Connect(char *host, int port);
		static SocketTransfer Bind(int port);
		static SocketTransfer WaitForClient(SocketTransfer socket);

		static void Recv(SocketTransfer socket, char *buffer, int size);
		static void Send(SocketTransfer socket, char* data, int size);
	};
}
