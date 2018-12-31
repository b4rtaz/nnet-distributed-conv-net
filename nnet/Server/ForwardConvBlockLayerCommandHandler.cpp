#include "../nNet/ConvolutionalBlockLayer.h"
#include "../nNet/ConvolutionalLayer.h"
#include "../nNet/MaxpoolLayer.h"
#include "../nNet/Benchmark.h"
#include "../Transfer/TransferException.h"
#include "../Client/Packets.h"
#include "ForwardConvBlockLayerCommandHandler.h"

using namespace Server;
using namespace nNet;

ForwardConvBlockLayerCommandHandler::ForwardConvBlockLayerCommandHandler(TransferClient *client, vector<Layer*> *layers) {
	_client = client;
	_layers = layers;
}

void GetLayerFromBlockOutputSingleFilterOffset(Layer *layer, BlockLayerFragmentParams *fragment, int *outFragmentOffset, int *outFragmentLength) {
	if (layer->layerType == CONVOLUTIONAL) {
		auto convLayer = (ConvolutionalLayer*)layer;
		ConvolutionalFragmentParams convParams = {
			fragment->inFragmentTop,
			fragment->inFragmentHeight
		};

		ConvolutionalFragmentHelper::GetFragmentOutputSingleFilterOffset(convLayer, &convParams,
			outFragmentOffset, outFragmentLength);
	}
	else if (layer->layerType == MAXPOOL) {
		auto lastMaxp = (MaxpoolLayer*)layer;
		MaxpoolFragmentParams maxpParams = {
			fragment->inFragmentTop,
			fragment->inFragmentHeight
		};

		MaxpoolFragmentHelper::GetFragmentOutputSingleFilterOffset(lastMaxp, &maxpParams,
			outFragmentOffset, outFragmentLength);
	}
	else {
		throw logic_error("Not supported layer type.");
	}
}

ConvolutionalBlockLayer* GetLayerBlockByIndex(vector<Layer*> *layers, int index) {
	Layer *layer;
	for (auto i = layers->begin(); i != layers->end(); i++) {
		layer = *i;
		if (layer->layerType == BLOCK && layer->index == index) {
			return (ConvolutionalBlockLayer*)layer;
		}
	}

	throw TransferException("Cannot find block by index.");
}

void ForwardConvBlockLayerCommandHandler::Handle(TransferPacket *header) {
	// Block index.
	if (header->dataLengths[0] != sizeof(int))
		throw TransferException("Invalid packet size. Expected size of one int.");
	int index = _client->ReadInt();

	auto block = GetLayerBlockByIndex(_layers, index);
	auto firstLayer = (ConvolutionalLayer*)block->layers.front();
	auto lastLayer = block->layers.back();

	// Conv params.
	if (header->dataLengths[1] != sizeof(ConvolutionalFragmentParams))
		throw TransferException("Invalid packet size. Expected size of conv fragment params.");
	ConvolutionalFragmentParams convParams;
	_client->ReadData((char*)&convParams, sizeof(ConvolutionalFragmentParams));

	// Input parts.
	if (header->dataLengths[2] != sizeof(int) || header->dataLengths[3] != sizeof(int))
		throw TransferException("Invalid packet size. Expected two int sizes.");

	int inFragmentOffset = _client->ReadInt();
	int inFragmentLenght = _client->ReadInt();

	for (int ch = 0; ch < firstLayer->inChannels; ch++) {
		float *part = &block->input[(ch * firstLayer->inputChannelCount) + inFragmentOffset];

		if (header->dataLengths[4 + ch] != inFragmentLenght * sizeof(float))
			throw TransferException("Invalid packet size. Expected size of block input part.");
		_client->ReadData((char*)part, inFragmentLenght * sizeof(float));
	}

	// Forward...
	vector<BlockLayerFragmentParams> *fragments = new vector<BlockLayerFragmentParams>();
	ConvolutionalBlockOffsetCalculator::Calculate(&block->layers, &convParams.top, &convParams.height, fragments);

	Benchmark bench;
	block->Forward(fragments);
	float forwardSeconds = bench.TotalSeconds();

	// Build response
	auto lastFragment = &fragments->back();

	int outFragmentOffset;
	int outFragmentLength;
	GetLayerFromBlockOutputSingleFilterOffset(lastLayer, lastFragment, &outFragmentOffset, &outFragmentLength);

	// TODO
	int outChannels;
	int outputChannelCount;

	if (lastLayer->layerType == MAXPOOL) {
		auto lastMaxp = (MaxpoolLayer*)lastLayer;
		outChannels = lastMaxp->outChannels;
		outputChannelCount = lastMaxp->outputChannelCount;
	}
	else if (lastLayer->layerType == CONVOLUTIONAL) {
		auto lastConv = (ConvolutionalLayer*)lastLayer;
		outChannels = lastConv->outChannels;
		outputChannelCount = lastConv->outputFilterCount;
	}
	else {
		throw logic_error("Not supported layer.");
	}
	// /TODO

	int outDataCount = 1 + outChannels;
	int *outLengths = new int[outDataCount];
	outLengths[0] = sizeof(int) * 2 + sizeof(float);
	for (int f = 0; f < outChannels; f++) {
		outLengths[1 + f] = outFragmentLength * sizeof(float);
	}

	__try {
		_client->SendHeader(PACKET_ACTION_CONV_BLOCK_FORWARD, outDataCount, outLengths);

		_client->SendData((char*)&outFragmentOffset, sizeof(int));
		_client->SendData((char*)&outFragmentLength, sizeof(int));
		_client->SendData((char*)&forwardSeconds, sizeof(float));

		for (int f = 0; f < outChannels; f++) {
			_client->SendData(
				(char*)&lastLayer->output[(f * outputChannelCount) + outFragmentOffset],
				outFragmentLength * sizeof(float));
		}
	}
	__finally {
		delete[] outLengths;
		delete fragments;
	}

	printf("* forwarded block #%d in %.4f seconds, input: <%d; %d>\n", index, forwardSeconds, convParams.top, convParams.top + convParams.height);
}
