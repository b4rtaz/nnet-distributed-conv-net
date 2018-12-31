#pragma once
#include <cstdio>

namespace Transfer {

	class TransferPacket {
	public:
		char action;
		int dataCount;
		int *dataLengths = NULL;

		~TransferPacket();
	};
}
