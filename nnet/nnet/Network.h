#pragma once
#include "Layer.h"
#include <vector>

using namespace std;

namespace nNet {

	class Network {
	public:
		vector<Layer*> layers;

		~Network();

		void AddLayer(Layer *layer);
		Layer* GetLayerByIndex(int index);

		Layer* Forward(float *input);
		void PropagateState();
	};
}
