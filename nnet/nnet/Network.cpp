#include "Network.h"
#include "Benchmark.h"
#include <stdexcept>

using namespace nNet;
using namespace std;

Network::~Network() {
	for (auto i = layers.begin(); i != layers.end(); i++) {
		delete *i;
	}
}

void Network::AddLayer(Layer *layer) {
	if (!layers.empty() && layer->layerType != ROUTE) {
		Layer *previous = layers.back();
		if (layer->inputCount != previous->outputCount) {
			throw logic_error("Invalid input/output pairs in neighboring layers.");
		}
	}
	layers.push_back(layer);
}

Layer* Network::GetLayerByIndex(int index) {
	for (auto i = layers.begin(); i != layers.end(); i++) {
		if ((*i)->index == index) {
			return *i;
		}
	}
	throw logic_error("Cannot found layer by index.");
}

Layer* Network::Forward(float *input) {
	Benchmark bt;
	Layer *layer;
	int index = 1;

	for (auto i = layers.begin(); i != layers.end(); i++) {
		layer = *i;
		layer->input = input;

		Benchmark b;
		layer->Forward();
		printf("%4d: %.5f seconds\n", index, b.TotalSeconds());

		input = layer->output;
		index++;
	}

	printf("      %.5f seconds\n", bt.TotalSeconds());
	return layer;
}

void Network::PropagateState() {
	for (auto i = layers.begin(); i != layers.end(); i++) {
		(*i)->PropagateState();
	}
	printf("Network propagated.\n");
}
