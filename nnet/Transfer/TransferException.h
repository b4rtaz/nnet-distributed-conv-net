#pragma once
#include <exception>

using namespace std;

namespace Transfer {

	class TransferException : public exception {
	public:
		TransferException(const char *message) throw()
			: exception(message) {
			//
		}
	};
}
