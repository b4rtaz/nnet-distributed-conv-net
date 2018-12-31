#pragma once
#include <cstdlib>

namespace nNet {

	typedef enum {
		BLOCK,
		CONVOLUTIONAL,
		MAXPOOL,
		ROUTE,
		REORG,
		YOLO_REGION
	} LayerType;

	class Layer {
	public:
		int index;
		LayerType layerType;

		float *input;
		int inputCount;
		float *output;
		int outputCount;

		virtual ~Layer() {}
		virtual void Forward() {}
		virtual void PropagateState() {}
	};
}
