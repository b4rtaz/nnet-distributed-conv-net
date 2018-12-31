#include "YoloNetParser.h"
#include "ConvolutionalLayer.h"
#include "MultiCpuConvolutionalLayer.h"
#include "SingleCpuConvolutionalLayer.h"
#include "MaxpoolLayer.h"
#include "YoloRegionLayer.h"
#include <exception>

using namespace nNet;
using namespace std;

YoloNetParser::YoloNetParser(bool verbose, LayerFactory *layerFactory) {
	_layerFactory = layerFactory;
	_verbose = verbose;
}

Network* YoloNetParser::Read(char *path) {
	_network = new Network();

	IniParser parser(this, ParserHandler);
	int error = parser.Parse(path);
	if (error != 0) {
		throw exception("Invalid ini config.");
	}

	return _network;
}

bool YoloNetParser::ParserHandler(void* user, IniParser *parser) {
	((YoloNetParser*)user)->ParseSection(parser);
	return true;
}

Activation ParseActivation(const char *str) {
	if (strcmp(str, "leaky") == 0)
		return LEAKY;
	if (strcmp(str, "linear") == 0)
		return LINEAR;
	throw exception("Not supported activation method.");
}

void YoloNetParser::ParseSection(IniParser *parser) {
	const char *sectionName = parser->GetSectionName();
	int nextIndex = (int)_network->layers.size() + 1;

	if (strcmp(sectionName, "net") == 0) {
		_outWidth = parser->GetIntParam("width", 0);
		_outHeight = parser->GetIntParam("height", 0);
		_outChannels = parser->GetIntParam("channels", 0);
	}
	else if (strcmp(sectionName, "convolutional") == 0) {
		ConvolutionalLayerParams params;
		params.inputWidth = _outWidth;
		params.inputHeight = _outHeight;
		params.inputChannels = _outChannels;
		params.activation = ParseActivation(parser->GetStringParam("activation"));
		params.normalize = parser->GetBoolParam("batch_normalize", 0);
		params.filters = parser->GetIntParam("filters", 0);
		params.stride = parser->GetIntParam("stride", 0);
		params.size = parser->GetIntParam("size", 0);
		int pad = parser->GetIntParam("pad", 0);
		if (pad)
			params.padding = params.size / 2;
	
		ConvolutionalLayer *layer = _layerFactory->CreateConvolutionalLayer(&params);
		layer->index = nextIndex;
		_network->AddLayer(layer);

		_outWidth = layer->outWidth;
		_outHeight = layer->outHeight;
		_outChannels = layer->outChannels;

		if (_verbose) {
			printf("%4d: conv  %4d %4d %4d > %4d %4d %4d\n",
				nextIndex,
				layer->inWidth, layer->inHeight, layer->inChannels,
				layer->outWidth, layer->outHeight, layer->outChannels);
		}
	}
	else if (strcmp(sectionName, "maxpool") == 0) {
		MaxpoolLayerParams params;
		params.inputWidth = _outWidth;
		params.inputHeight = _outHeight;
		params.inputChannels = _outChannels;
		params.stride = parser->GetIntParam("stride", 0);
		params.size = parser->GetIntParam("size", 0);
		params.padding = parser->GetIntParam("padding", 0);
	
		MaxpoolLayer *layer = _layerFactory->CreateMaxpoolLayer(&params);
		layer->index = nextIndex;
		_network->AddLayer(layer);
	
		_outWidth = layer->outWidth;
		_outHeight = layer->outHeight;
		_outChannels = layer->outChannels;
	
		if (_verbose) {
			printf("%4d: max   %4d %4d %4d > %4d %4d %4d\n",
				nextIndex,
				layer->inWidth, layer->inHeight, layer->inChannels,
				layer->outWidth, layer->outHeight, layer->outChannels);
		}
	}
	else if (strcmp(sectionName, "region") == 0) {
		YoloRegionLayerParams params;
		params.inputWidth = _outWidth;
		params.inputHeight = _outHeight;
		params.classes = parser->GetIntParam("classes", 0);
		params.num = parser->GetIntParam("num", 0);
		params.coords = parser->GetIntParam("coords", 0);

		YoloRegionLayer *layer = _layerFactory->CreateYoloRegionLayer(&params);
		layer->index = nextIndex;
		_network->AddLayer(layer);

		const char *anchors = parser->GetStringParam("anchors");
		char *anch = (char*)anchors;
		for (int i = 0; i < params.num * 2; i++) {
			layer->biases[i] = (float)atof(anch);
			anch = strstr(anch, ",") + 1;
		}

		if (_verbose) {
			printf("%4d: region\n", nextIndex);
		}
	}
	else if (strcmp(sectionName, "route") == 0) {
		RouteLayerParams params;
		params.network = _network;

		if (_verbose) printf("%4d: route (", nextIndex);

		const char *indexes = parser->GetStringParam("layers");
		char *idx = (char*)indexes;
		for (;;) {
			int index = strtol(idx, NULL, 10);
			if (index < 0) {
				index = nextIndex + index;
			}

			if (_verbose) {
				if (indexes != idx) printf(", ");
				printf("%d", index);
			}

			params.layerIndexes.push_back(index);

			idx = strstr(idx, ",");
			if (idx == NULL) break;
			idx++;
		}

		if (_verbose) printf(")\n");

		RouteLayer *layer = _layerFactory->CreateRouteLayer(&params);
		layer->index = nextIndex;
		_network->AddLayer(layer);

		_outWidth = layer->outWidth;
		_outHeight = layer->outHeight;
		_outChannels = layer->outChannels;


	}
	else if (strcmp(sectionName, "reorg") == 0) {
		ReorgLayerParams params;
		params.inWidth = _outWidth;
		params.inHeight = _outHeight;
		params.inChannels = _outChannels;
		params.reverse = false;
		params.stride = parser->GetIntParam("stride", 0);

		ReorgLayer *layer = _layerFactory->CreateReorgLayer(&params);
		layer->index = nextIndex;
		_network->AddLayer(layer);

		_outWidth = layer->outWidth;
		_outHeight = layer->outHeight;
		_outChannels = layer->outChannels;

		if (_verbose) {
			printf("%4d: reorg\n", nextIndex);
		}
	}
	else {
		throw logic_error("Not supported layer type.");
	}
}
