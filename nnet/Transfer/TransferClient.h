#pragma once
#include "TransferPacket.h"

namespace Transfer {

	class TransferClient {
	public:
		TransferClient() {}
		virtual ~TransferClient() {}

		virtual TransferPacket* WaitForHeader() = 0;

		virtual void SendHeader(char action, int dataCount, int *lengths) = 0;
		virtual void SendHeader(TransferPacket *packet) = 0;

		virtual void SendData(char *data, int length) = 0;
		virtual void ReadData(char *data, int length) = 0;
		virtual int ReadInt() = 0;
	};
}
