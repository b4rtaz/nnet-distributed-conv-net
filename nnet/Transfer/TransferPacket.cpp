#include "TransferPacket.h"

using namespace Transfer;

TransferPacket::~TransferPacket() {
	if (dataLengths != NULL) {
		delete[] dataLengths;
		dataLengths = NULL;
	}
}
