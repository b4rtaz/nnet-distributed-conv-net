#include "../Transfer/TransferException.h"
#include "CreateConvLayerCommand.h"
#include "Packets.h"

using namespace Client;
using namespace Transfer;

CreateConvLayerCommand::CreateConvLayerCommand(ConvolutionalLayer *layer, ClientsManager *clientsManager) {
	_layer = layer;
	_clientsManager = clientsManager;
}

void CreateConvLayerCommandDataHandler(void* handlerUserParam, int clientIndex, TransferPacket *responsePacketHeader, TransferClient *client) {
	((CreateConvLayerCommand*)handlerUserParam)->DataHandler(clientIndex, responsePacketHeader, client);
}

void SetSameLengthsAndDatas(int n, int *lengths, char **datas, int *layerIndex, int numberCount, float *numbers) {
	for (int i = 0; i < n; i++) {
		int ir = i * 2;
		lengths[ir] = (int)sizeof(int);
		datas[ir] = (char*)layerIndex;
		lengths[ir + 1] = numberCount * sizeof(float);
		datas[ir + 1] = (char*)numbers;
	}
}

void FillConvParamsByLayer(ConvolutionalLayer *layer, ConvolutionalLayerParams *p) {
	p->inputWidth = layer->inWidth;
	p->inputHeight = layer->inHeight;
	p->inputChannels = layer->inChannels;
	p->activation = layer->activation;
	p->filters = layer->filters;
	p->size = layer->size;
	p->stride = layer->stride;
	p->padding = layer->padding;
	p->normalize = layer->normalize;
}

void CreateConvLayerCommand::Execute() {
	int n = _clientsManager->GetClientCount();
	int n2 = n * 2;

	ConvolutionalLayerParams params;
	FillConvParamsByLayer(_layer, &params);

	int *lengths = new int[n2];
	char **datas = new char*[n2];
	for (int i = 0; i < n2; i += 2) {
		lengths[i] = (int)sizeof(ConvolutionalLayerParams);
		datas[i] = (char*)&params;
		lengths[i + 1] = (int)sizeof(int);
		datas[i + 1] = (char*)&_layer->index;
	}

	__try {
		_clientsManager->SendData(PACKET_ACTION_CONV_CREATE, 2, lengths, datas, this, CreateConvLayerCommandDataHandler);

		SetSameLengthsAndDatas(n, lengths, datas, &_layer->index, _layer->weightCount, _layer->weights);
		_clientsManager->SendData(PACKET_ACTION_CONV_SET_WEIGHTS, 2, lengths, datas, NULL, NULL);

		SetSameLengthsAndDatas(n, lengths, datas, &_layer->index, _layer->filters, _layer->biases);
		_clientsManager->SendData(PACKET_ACTION_CONV_SET_BIASES, 2, lengths, datas, NULL, NULL);

		if (_layer->normalize) {
			SetSameLengthsAndDatas(n, lengths, datas, &_layer->index, _layer->filters, _layer->rollingMean);
			_clientsManager->SendData(PACKET_ACTION_CONV_SET_ROLLING_MEAN, 2, lengths, datas, NULL, NULL);

			SetSameLengthsAndDatas(n, lengths, datas, &_layer->index, _layer->filters, _layer->rollingVariance);
			_clientsManager->SendData(PACKET_ACTION_CONV_SET_ROLLING_VARIANCE, 2, lengths, datas, NULL, NULL);

			SetSameLengthsAndDatas(n, lengths, datas, &_layer->index, _layer->filters, _layer->scales);
			_clientsManager->SendData(PACKET_ACTION_CONV_SET_SCALES, 2, lengths, datas, NULL, NULL);
		}
	}
	__finally {
		delete[] lengths;
		delete[] datas;
	}
}

void CreateConvLayerCommand::DataHandler(int clientIndex, TransferPacket *header, TransferClient *client) {
	if (header->action == PACKET_ACTION_CONV_CREATE) {
		if (header->dataCount != 0) {
			throw TransferException("Exepcted no data (create conv layer command).");
		}
	}
}
