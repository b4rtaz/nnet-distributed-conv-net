#pragma once
#include "Network.h"

using namespace std;

namespace nNet {

	class YoloWeightsLoader {
	public:
		static void Read(Network *network, char *path);
	};
}
