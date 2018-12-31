#pragma once
#include "ConvolutionalLayer.h"
#include "MaxpoolLayer.h"
#include "YoloRegionLayer.h"
#include "ConvolutionalBlockLayer.h"
#include "ReorgLayer.h"
#include "RouteLayer.h"

namespace nNet {

	class LayerFactory {
	public:
		virtual ConvolutionalLayer* CreateConvolutionalLayer(ConvolutionalLayerParams *params);
		virtual ConvolutionalBlockLayer* CreateConvolutionalBlockLayer(ConvolutionalBlockLayerParams *params);
		virtual MaxpoolLayer* CreateMaxpoolLayer(MaxpoolLayerParams *params);
		virtual ReorgLayer* CreateReorgLayer(ReorgLayerParams *params);
		virtual RouteLayer* CreateRouteLayer(RouteLayerParams *params);
		virtual YoloRegionLayer* CreateYoloRegionLayer(YoloRegionLayerParams *params);
	};
}
