#include "ForwardConvLayerCommandHandler.h"
#include "CreateConvLayerCommandHandler.h"
#include "../Client/Packets.h"
#include "../Transfer/TransferException.h"
#include "../nNet/Benchmark.h"

using namespace Server;
using namespace nNet;

ForwardConvLayerCommandHandler::ForwardConvLayerCommandHandler(TransferClient *client, vector<Layer*> *layers) {
	_client = client;
	_layers = layers;
}

void ForwardConvLayerCommandHandler::Handle(TransferPacket *header) {
	int layerIndex = ConvLayerCommandHelper::ReadLayerIndex(_client, header->dataLengths[0]);
	ConvolutionalLayer *layer = ConvLayerCommandHelper::GetLayerByIndex(layerIndex, _layers);

	int paramsLength = header->dataLengths[1];
	if (paramsLength != sizeof(ConvolutionalFragmentParams))
		throw TransferException("Invalid packet size (expected size of conv fragment params).");
	ConvolutionalFragmentParams params;
	_client->ReadData((char*)&params, paramsLength);

	int inFragmentOffset;
	int inFragmentLenght;
	ConvolutionalFragmentHelper::GetFragmentInputSingleChannelOffset(layer, &params, &inFragmentOffset, &inFragmentLenght);

	for (int ch = 0; ch < layer->inChannels; ch++) {
		int partLength = header->dataLengths[2 + ch];
		if (partLength != inFragmentLenght * sizeof(float)) {
			printf("<trace> invalid channel part size, expected: %d, got: %d\n", (int)(inFragmentLenght * sizeof(float)), partLength);
			throw TransferException("Invalid data part size (with one channel).");
		}

		float *input = &layer->input[(ch * layer->inputChannelCount) + inFragmentOffset];
		_client->ReadData((char*)input, partLength);
	}

	Benchmark bench;
	layer->Forward(&params);
	float totalSeconds = bench.TotalSeconds();

	int outFragmentOffset;
	int outFragmentLength;
	ConvolutionalFragmentHelper::GetFragmentOutputSingleFilterOffset(layer, &params, &outFragmentOffset, &outFragmentLength);

	int *convForwardLenghts = new int[2 + layer->outChannels];
	convForwardLenghts[0] = sizeof(ConvolutionalFragmentParams);
	convForwardLenghts[1] = sizeof(float);
	for (int f = 0; f < layer->outChannels; f++)
		convForwardLenghts[2 + f] = outFragmentLength * sizeof(float);

	__try {
		_client->SendHeader(PACKET_ACTION_CONV_FORWARD, layer->outChannels + 2, convForwardLenghts);
		_client->SendData((char*)&params, sizeof(ConvolutionalFragmentParams));
		_client->SendData((char*)&totalSeconds, sizeof(float));

		for (int f = 0; f < layer->outChannels; f++)
			_client->SendData((char*)&layer->output[(f * layer->outputFilterCount) + outFragmentOffset],
				outFragmentLength * sizeof(float));
	}
	__finally {
		delete[] convForwardLenghts;
	}
}
