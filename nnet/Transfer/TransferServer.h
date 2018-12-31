#pragma once
#include "TransferClient.h"

namespace Transfer {

	class TransferServer {
	public:
		virtual void Bind() = 0;
		virtual TransferClient* WaitForClient() = 0;
	};
}
