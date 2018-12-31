#include "YoloWeightsLoader.h"
#include "ConvolutionalLayer.h"
#include <cstdio>
#include <exception>

using namespace nNet;

void YoloWeightsLoader::Read(Network *network, char *path) {
	FILE *fp = fopen(path, "rb");
	if (!fp) {
		throw exception("Cannot open weights file.");
	}

	int major;
	int minor;
	int revision;
	size_t seen;
	fread(&major, sizeof(int), 1, fp);
	fread(&minor, sizeof(int), 1, fp);
	fread(&revision, sizeof(int), 1, fp);
	if ((major * 10 + minor) >= 2) {
		fread(&seen, sizeof(size_t), 1, fp);
	} else {
		int iseen = 0;
		fread(&iseen, sizeof(int), 1, fp);
		seen = iseen;
	}
	int transpose = (major > 1000) || (minor > 1000);

	for (auto i = network->layers.begin(); i != network->layers.end(); i++) {
		Layer *l = *i;

		if (l->layerType == CONVOLUTIONAL) {
			ConvolutionalLayer *cl = (ConvolutionalLayer*)l;

			fread(cl->biases, sizeof(float), cl->filters, fp);
			if (cl->normalize/* && (!l.dontloadscales)*/) {
				fread(cl->scales, sizeof(float), cl->filters, fp);
				fread(cl->rollingMean, sizeof(float), cl->filters, fp);
				fread(cl->rollingVariance, sizeof(float), cl->filters, fp);
			}
			fread(cl->weights, sizeof(float), cl->weightCount, fp);
		}
	}

	fclose(fp);
}
