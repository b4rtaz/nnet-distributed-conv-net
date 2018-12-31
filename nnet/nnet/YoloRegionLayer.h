#pragma once
#include "Layer.h"

namespace nNet {

	typedef struct {
		int inputWidth;
		int inputHeight;
		int classes;
		int coords;
		int num;
	} YoloRegionLayerParams;

	typedef struct {
		float x, y, w, h;
	} YoloBox;

	typedef struct {
		float probability;
		int classIndex;
		YoloBox box;
	} YoloRegion;

	class YoloRegionLayer : public Layer {
	public:
		int inWidth;
		int inHeight;

		int classes;
		int coords;
		int num;
		int background;
		int softmax_tree;
		int softmax;

		float *biases;
		int biasesCount;

		YoloBox *boxes;
		float **probs;

	public:
		YoloRegionLayer(YoloRegionLayerParams *params);
		~YoloRegionLayer() override;

		void Forward() override;

		YoloRegion* ReadRegions(int imageWidth, int imageHeight, int firstLayerWidth, int firstLayerHeight, float thresh, int *regionCount);
	};
}
