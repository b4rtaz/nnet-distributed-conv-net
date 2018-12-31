#pragma once
#include "Network.h"
#include "LayerFactory.h"
#include "ConvolutionalBlockLayer.h"

namespace nNet {

	class BlockLayerInjector {
	private:
		LayerFactory *_layerFactory;

	public:
		BlockLayerInjector(LayerFactory *layerFactory);
		ConvolutionalBlockLayer *Inject(vector<Layer*> *layers, int indexFrom, int indexTo);
	};
}
