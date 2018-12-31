#include "SocketTransferClient.h"
#include "TransferException.h"
#include <memory>

using namespace Transfer;

SocketTransferClient::SocketTransferClient(char *host, int port)
	: TransferClient::TransferClient() {
	SocketUtils::InitIfNeed();
	_socket = SocketUtils::Connect(host, port);
}

SocketTransferClient::SocketTransferClient(SocketTransfer socket)
	: TransferClient::TransferClient() {
	_socket = socket;
}

SocketTransferClient::~SocketTransferClient() {
	closesocket(_socket);
}

TransferPacket* SocketTransferClient::WaitForHeader() {
	TransferPacket *packet = new TransferPacket();
	try {
		SocketUtils::Recv(_socket, (char*)&packet->action, sizeof(char));
		SocketUtils::Recv(_socket, (char*)&packet->dataCount, sizeof(int));
		if (packet->dataCount > 0) {
			packet->dataLengths = new int[packet->dataCount];
			SocketUtils::Recv(_socket, (char*)packet->dataLengths, packet->dataCount * sizeof(int));
		}
	} catch (const TransferException &e) {
		delete packet;
		throw e;
	}
	return packet;
}

void SocketTransferClient::SendHeader(char action, int dataCount, int *lengths) {
	SocketUtils::Send(_socket, (char*)&action, sizeof(char));
	SocketUtils::Send(_socket, (char*)&dataCount, sizeof(int));
	if (dataCount > 0) {
		SocketUtils::Send(_socket, (char*)lengths, dataCount * sizeof(int));
	}
}

void SocketTransferClient::SendHeader(TransferPacket *packet) {
	SendHeader(packet->action, packet->dataCount, packet->dataLengths);
}

void SocketTransferClient::SendData(char *data, int length) {
	SocketUtils::Send(_socket, data, length);
}

void SocketTransferClient::ReadData(char *data, int length) {
	SocketUtils::Recv(_socket, data, length);
}

int SocketTransferClient::ReadInt() {
	int value;
	SocketUtils::Recv(_socket, (char*)&value, sizeof(int));
	return value;
}
