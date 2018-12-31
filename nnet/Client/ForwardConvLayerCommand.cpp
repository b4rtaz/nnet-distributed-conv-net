#include "../nNet/ConvolutionalFragment.h"
#include "../Transfer/TransferException.h"
#include "ForwardConvLayerCommand.h"
#include "Packets.h"

using namespace Client;
using namespace nNet;
using namespace Transfer;

ForwardConvLayerCommand::ForwardConvLayerCommand(ConvolutionalLayer *layer, ClientsManager *clientsManager) {
	_layer = layer;
	_clientsManager = clientsManager;
}

static void ForwardConvLayerCommandDataHandler(void* handlerUserParam, int clientIndex, TransferPacket *responsePacketHeader, TransferClient *client) {
	((ForwardConvLayerCommand*)handlerUserParam)->DataHandler(clientIndex, responsePacketHeader, client);
}

void ForwardConvLayerCommand::Execute() {
	int n = _clientsManager->GetClientCount();

	ConvolutionalFragmentParams zero;
	zero.top = 0;
	zero.height = _layer->inHeight;

	_totalSeconds.resize(n);
	_processSeconds.resize(n);
	_benchmark.Reset();

	ConvolutionalFragmentParams *params = new ConvolutionalFragmentParams[n];
	int *lengths = new int[n * (2 + _layer->inChannels)];
	char **datas = new char*[n * (2 + _layer->inChannels)];

	__try {
		int inFragmentOffset;
		int inFragmentLenght;

		for (int i = 0; i < n; i++) {
			char **pdatas = &datas[i * (2 + _layer->inChannels)];
			int *plengths = &lengths[i * (2 + _layer->inChannels)];

			plengths[0] = sizeof(int);
			pdatas[0] = (char*)&_layer->index;

			ConvolutionalFragmentHelper::SplitFragment(&zero, &params[i], n, i);
			plengths[1] = sizeof(ConvolutionalFragmentParams);
			pdatas[1] = (char*)&params[i];

			ConvolutionalFragmentHelper::GetFragmentInputSingleChannelOffset(_layer, &params[i], &inFragmentOffset, &inFragmentLenght);

			for (int ch = 0; ch < _layer->inChannels; ch++) {
				float *part = &_layer->input[(ch * _layer->inputChannelCount) + inFragmentOffset];
				plengths[2 + ch] = inFragmentLenght * sizeof(float);
				pdatas[2 + ch] = (char*)part;
			}
		}

		_clientsManager->SendData(PACKET_ACTION_CONV_FORWARD, 2 + _layer->inChannels, lengths, datas, this, ForwardConvLayerCommandDataHandler);
	}
	__finally {
		delete[] params;
		delete[] lengths;
		delete[] datas;
	}

	for (int i = 0; i < n; i++) {
		float processed = _processSeconds[i];
		float transfered = _totalSeconds[i] - processed;

		printf("[remote %d/%d]  processed: %.5f seconds, transfered: %.5f seconds\n", i + 1, n, processed, transfered);
	}
}

void ForwardConvLayerCommand::DataHandler(int clientIndex, TransferPacket *responsePacketHeader, TransferClient *client) {
	if (responsePacketHeader->action == PACKET_ACTION_CONV_FORWARD) {
		int paramsSize = responsePacketHeader->dataLengths[0];
		if (paramsSize != sizeof(ConvolutionalFragmentParams))
			throw TransferException("Invalid conv params size.");

		ConvolutionalFragmentParams params;
		client->ReadData((char*)&params, sizeof(ConvolutionalFragmentParams));

		client->ReadData((char*)&_processSeconds[clientIndex], sizeof(float));

		int outFragmentOffset;
		int outFragmentLength;
		ConvolutionalFragmentHelper::GetFragmentOutputSingleFilterOffset(_layer, &params, &outFragmentOffset, &outFragmentLength);

		for (int f = 0; f < _layer->outChannels; f++) {
			int partLength = responsePacketHeader->dataLengths[2 + f];
			if (partLength != outFragmentLength * sizeof(float))
				throw TransferException("Invalid data part size (with one channel).");

			client->ReadData((char*)&_layer->output[(f * _layer->outputFilterCount) + outFragmentOffset], outFragmentLength * sizeof(float));
		}

		_totalSeconds[clientIndex] = _benchmark.TotalSeconds();
	}
}
