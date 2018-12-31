#pragma once
#include "Network.h"
#include "Layer.h"
#include <vector>

using namespace std;

namespace nNet {

	typedef struct {
		int inWidth;
		int inHeight;
		int inChannels;

		int stride;
		bool reverse;
	} ReorgLayerParams;

	class ReorgLayer : public Layer {
	public:
		int inWidth;
		int inHeight;
		int inChannels;
		int outWidth;
		int outHeight;
		int outChannels;

		int stride;
		bool reverse;

		ReorgLayer(ReorgLayerParams *params);
		~ReorgLayer();

		void Forward() override;
	};
}
