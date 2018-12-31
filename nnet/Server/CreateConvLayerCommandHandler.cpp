#include "../Client/Packets.h"
#include "../Transfer/TransferException.h"
#include "../nNet/ConvolutionalLayer.h"
#include "WorkerServerLayerFactory.h"
#include "CreateConvLayerCommandHandler.h"

using namespace Server;
using namespace nNet;

CreateConvLayerCommandHandler::CreateConvLayerCommandHandler(TransferClient *client, vector<Layer*> *layers, LayerFactory *layerFactory) {
	_client = client;
	_layers = layers;
	_layerFactory = layerFactory;
}

void CreateConvLayerCommandHandler::Handle(TransferPacket *header) {
	if (header->action == PACKET_ACTION_CONV_CREATE) {
		HandleCreate(header);
	}
	else {
		HandleSetWeights(header);
	}
}

void ReadConvLayerParams(TransferClient *client, int dataLength, ConvolutionalLayerParams *params) {
	if (dataLength != sizeof(ConvolutionalLayerParams))
		throw TransferException("Invalid packet data size.");
	client->ReadData((char*)params, sizeof(ConvolutionalLayerParams));
}

void CreateConvLayerCommandHandler::HandleCreate(TransferPacket *header) {
	if (header->dataCount != 2)
		throw TransferException("Invalid packet data count.");

	ConvolutionalLayerParams params;
	ReadConvLayerParams(_client, header->dataLengths[0], &params);

	int layerIndex = ConvLayerCommandHelper::ReadLayerIndex(_client, header->dataLengths[1]);

	auto layer = _layerFactory->CreateConvolutionalLayer(&params);
	layer->index = layerIndex;
	_layers->push_back(layer);

	printf("%4d: conv %4d %4d %4d\n", layerIndex, params.inputWidth, params.inputHeight, params.inputChannels);

	_client->SendHeader(PACKET_ACTION_CONV_CREATE, 0, NULL);
}

void ReadConvLayerWeights(TransferClient *client, int dataLength, float *numbers, int numberCount) {
	if (dataLength != numberCount * sizeof(float)) {
		throw TransferException("Invalid packet data size!");
	}
	client->ReadData((char*)numbers, dataLength);
}

void CreateConvLayerCommandHandler::HandleSetWeights(TransferPacket *header) {
	if (header->dataCount != 2)
		throw TransferException("Expected two datas in conv set packet.");

	int layerIndex = ConvLayerCommandHelper::ReadLayerIndex(_client, header->dataLengths[0]);
	ConvolutionalLayer *layer = ConvLayerCommandHelper::GetLayerByIndex(layerIndex, _layers);

	if (header->action == PACKET_ACTION_CONV_SET_WEIGHTS) {
		ReadConvLayerWeights(_client, header->dataLengths[1], layer->weights, layer->weightCount);
	}
	else if (header->action == PACKET_ACTION_CONV_SET_BIASES) {
		ReadConvLayerWeights(_client, header->dataLengths[1], layer->biases, layer->filters);
	}
	else if (header->action == PACKET_ACTION_CONV_SET_ROLLING_MEAN) {
		ReadConvLayerWeights(_client, header->dataLengths[1], layer->rollingMean, layer->filters);
	}
	else if (header->action == PACKET_ACTION_CONV_SET_ROLLING_VARIANCE) {
		ReadConvLayerWeights(_client, header->dataLengths[1], layer->rollingVariance, layer->filters);
	}
	else if (header->action == PACKET_ACTION_CONV_SET_SCALES) {
		ReadConvLayerWeights(_client, header->dataLengths[1], layer->scales, layer->filters);
	}

	_client->SendHeader(header->action, 0, NULL);
}

int ConvLayerCommandHelper::ReadLayerIndex(TransferClient *client, int layerIndexLength) {
	if (layerIndexLength != sizeof(int))
		throw TransferException("Invalid data size (layer index size).");
	int index;
	client->ReadData((char*)&index, layerIndexLength);
	return index;
}

ConvolutionalLayer* ConvLayerCommandHelper::GetLayerByIndex(int index, vector<Layer*> *layers) {
	Layer *layer;
	for (int i = 0; i < layers->size(); i++) {
		layer = layers->at(i);
		if (layer->index == index) {
			if (layer->layerType != CONVOLUTIONAL)
				throw TransferException("Invalid layer type");
			return (ConvolutionalLayer*)layer;
		}
	}
	throw TransferException("Cannot find conv layer by index.");
}
