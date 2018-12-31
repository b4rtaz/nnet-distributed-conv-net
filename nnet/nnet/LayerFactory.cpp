#include "LayerFactory.h"
#include "MultiCpuConvolutionalLayer.h"
#include "SingleCpuConvolutionalLayer.h"
#include "SingleCpuConvolutionalBlockLayer.h"
#include "MultiCpuConvolutionalBlockLayer.h"

using namespace nNet;

ConvolutionalLayer* LayerFactory::CreateConvolutionalLayer(ConvolutionalLayerParams *params) {
	return new SingleCpuConvolutionalLayer(params);
	//return new MultiCpuConvolutionalLayer(params);
}

ConvolutionalBlockLayer* LayerFactory::CreateConvolutionalBlockLayer(ConvolutionalBlockLayerParams *params) {
	//return new SingleConvolutionalBlockLayer(params);
	return new MultiCpuConvolutionalBlockLayer(params);
}

MaxpoolLayer* LayerFactory::CreateMaxpoolLayer(MaxpoolLayerParams *params) {
	return new MaxpoolLayer(params);
}

ReorgLayer* LayerFactory::CreateReorgLayer(ReorgLayerParams *params) {
	return new ReorgLayer(params);
}

RouteLayer* LayerFactory::CreateRouteLayer(RouteLayerParams *params) {
	return new RouteLayer(params);
}

YoloRegionLayer* LayerFactory::CreateYoloRegionLayer(YoloRegionLayerParams *params) {
	return new YoloRegionLayer(params);
}
