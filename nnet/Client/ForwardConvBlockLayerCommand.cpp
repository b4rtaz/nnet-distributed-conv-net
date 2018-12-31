#include "../nNet/ConvolutionalLayer.h"
#include "../nNet/ConvolutionalFragment.h"
#include "../nNet/MaxpoolLayer.h"
#include "../Transfer/TransferException.h"
#include "Packets.h"
#include "ForwardConvBlockLayerCommand.h"

using namespace Client;
using namespace nNet;

ForwardConvBlockLayerCommand::ForwardConvBlockLayerCommand(ConvolutionalBlockLayer *block, ClientsManager *clientsManager) {
	_block = block;
	_clientsManager = clientsManager;
}

void ForwardConvBlockCommandDataHandler(void* handlerUserParam, int clientIndex, TransferPacket *responsePacketHeader, TransferClient *client) {
	((ForwardConvBlockLayerCommand*)handlerUserParam)->DataHandler(clientIndex, responsePacketHeader, client);
}

void ForwardConvBlockLayerCommand::Execute() {
	int i;
	auto n = _clientsManager->GetClientCount();
	auto firstLayer = (ConvolutionalLayer*)_block->layers.front();

	auto convParams = new ConvolutionalFragmentParams[n];
	auto fragments = new vector<BlockLayerFragmentParams>[n];
	auto dataCount = 4 + firstLayer->inChannels;
	auto lengths = new int[n * dataCount];
	auto datas = new char*[n * dataCount];
	auto inFragmentOffsets = new int[n];
	auto inFragmentLengths = new int[n];

	ConvolutionalFragmentParams zeroParams = { 0, firstLayer->inHeight };
	ConvolutionalFragmentParams *convParam;
	ConvolutionalFragmentParams convParamExtend;

	_totalSeconds.resize(n);
	_processSeconds.resize(n);
	_bytes.resize(n, 0);
	_benchmark.Reset();

	__try {
		for (i = 0; i < n; i++) {
			char **pdatas = &datas[i * dataCount];
			int *plengths = &lengths[i * dataCount];

			convParam = &convParams[i];
			ConvolutionalFragmentHelper::SplitFragment(&zeroParams, convParam, n, i);

			convParamExtend = *convParam;
			ConvolutionalBlockOffsetCalculator::Calculate(&_block->layers, &convParamExtend.top, &convParamExtend.height, &fragments[i]);

			int inFragmentOffset;
			int inFragmentLength;
			ConvolutionalFragmentHelper::GetFragmentInputSingleChannelOffsetWithoutPadding(firstLayer, &convParamExtend, &inFragmentOffset, &inFragmentLength);

			inFragmentOffsets[i] = inFragmentOffset;
			inFragmentLengths[i] = inFragmentLength;

			plengths[0] = sizeof(int);
			pdatas[0] = (char*)&_block->index;

			plengths[1] = sizeof(ConvolutionalFragmentParams);
			pdatas[1] = (char*)convParam;

			plengths[2] = sizeof(int);
			pdatas[2] = (char*)&inFragmentOffsets[i];

			plengths[3] = sizeof(int);
			pdatas[3] = (char*)&inFragmentLengths[i];

			for (int ch = 0; ch < firstLayer->inChannels; ch++) {
				float *part = &_block->input[(ch * firstLayer->inputChannelCount) + inFragmentOffset];
				plengths[4 + ch] = inFragmentLength * sizeof(float);
				pdatas[4 + ch] = (char*)part;
			}

			_bytes[i] += inFragmentLength * sizeof(float) * firstLayer->inChannels;
		}

		_clientsManager->SendData(PACKET_ACTION_CONV_BLOCK_FORWARD, dataCount, lengths, datas, this, ForwardConvBlockCommandDataHandler);
	}
	__finally {
		delete[] inFragmentLengths;
		delete[] inFragmentOffsets;
		delete[] convParams;
		delete[] fragments;
		delete[] lengths;
		delete[] datas;
	}

	for (i = 0; i < n; i++) {
		float processed = _processSeconds[i];
		float transfered = _totalSeconds[i] - processed;
		int bts = _bytes[i];

		printf("[remote %d/%d] processed: %.5f seconds, transfered: %.5f seconds (%d bytes)\n", i + 1, n, processed, transfered, bts);
	}
}

void ForwardConvBlockLayerCommand::DataHandler(int clientIndex, TransferPacket *header, TransferClient *client) {
	if (header->action != PACKET_ACTION_CONV_BLOCK_FORWARD) {
		throw TransferException("Invalid packet. Expected response from forwarding conv block.");
	}

	// TODO
	auto lastLayer = (MaxpoolLayer*)_block->layers.back();
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

	if (header->dataCount != 1 + outChannels) {
		throw TransferException("Invalid data count.");
	}

	if (header->dataLengths[0] != 2 * sizeof(int) + sizeof(float)) {
		throw TransferException("Invalid size of first part packet.");
	}

	int outFragmentOffset = client->ReadInt();
	int outFragmentLength = client->ReadInt();
	client->ReadData((char*)&_processSeconds[clientIndex], sizeof(float));

	for (int ch = 0; ch < outChannels; ch++) {
		float *part = &_block->output[(ch * outputChannelCount) + outFragmentOffset];
		client->ReadData((char*)part, outFragmentLength * sizeof(float));
	}

	_totalSeconds[clientIndex] = _benchmark.TotalSeconds();
	_bytes[clientIndex] += outFragmentLength * sizeof(float) * outChannels;
}
