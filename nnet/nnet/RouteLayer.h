#pragma once
#include "Network.h"
#include "Layer.h"
#include <vector>

using namespace std;

namespace nNet {

	typedef struct {
		Network *network;
		vector<int> layerIndexes;
	} RouteLayerParams;

	class RouteLayer : public Layer {
	public:
		Network *network;
		vector<int> layerIndexes;
		int layerCount;

		int outWidth;
		int outHeight;
		int outChannels;

		RouteLayer(RouteLayerParams *params);
		~RouteLayer();

		void Forward() override;
	};
}
