#include "YoloRegionLayer.h"
#include "Activation.h"
#include <cfloat>
#include <cmath>
#include <cstring>

using namespace nNet;

void correct_region_boxes(YoloBox *boxes, int n, int w, int h, int netw, int neth, int relative) {
	int i;
	int new_w = 0;
	int new_h = 0;
	if (((float)netw / w) < ((float)neth / h)) {
		new_w = netw;
		new_h = (h * netw) / w;
	}
	else {
		new_h = neth;
		new_w = (w * neth) / h;
	}
	for (i = 0; i < n; ++i) {
		YoloBox b = boxes[i];
		b.x = (float)(b.x - (netw - new_w) / 2. / netw) / ((float)new_w / netw);
		b.y = (float)(b.y - (neth - new_h) / 2. / neth) / ((float)new_h / neth);
		b.w *= (float)netw / new_w;
		b.h *= (float)neth / new_h;
		if (!relative) {
			b.x *= w;
			b.w *= w;
			b.y *= h;
			b.h *= h;
		}
		boxes[i] = b;
	}
}

YoloBox get_region_box(float *x, float *biases, int n, int index, int i, int j, int w, int h, int stride) {
	YoloBox b;
	b.x = (float)(i + x[index + 0 * stride]) / w;
	b.y = (float)(j + x[index + 1 * stride]) / h;
	b.w = (float)std::exp(x[index + 2 * stride]) * biases[2 * n] / w;
	b.h = (float)std::exp(x[index + 3 * stride]) * biases[2 * n + 1] / h;
	return b;
}

int entry_index(YoloRegionLayer *l, int batch, int location, int entry) {
	int n = location / (l->inWidth*l->inHeight);
	int loc = location % (l->inWidth*l->inHeight);
	return n*l->inWidth*l->inHeight*(l->coords + l->classes + 1) + entry*l->inWidth*l->inHeight + loc;
}

void get_region_boxes(YoloRegionLayer *l, int w, int h, int netw, int neth,
	float thresh, float **probs, YoloBox *boxes, float **masks, int only_objectness, int *map, float tree_thresh, int relative)
{
	int i, j, n;
	float *predictions = l->output;

	for (i = 0; i < l->inWidth*l->inHeight; ++i) {
		int row = i / l->inWidth;
		int col = i % l->inWidth;
		for (n = 0; n < l->num; ++n) {
			int index = n*l->inWidth*l->inHeight + i;
			for (j = 0; j < l->classes; ++j) {
				probs[index][j] = 0;
			}
			int obj_index = entry_index(l, 0, n*l->inWidth*l->inHeight + i, l->coords);
			int box_index = entry_index(l, 0, n*l->inWidth*l->inHeight + i, 0);
			//int mask_index = entry_index(l, 0, n*l->inWidth*l->inHeight + i, 4);
			float scale = l->background ? 1 : predictions[obj_index];
			boxes[index] = get_region_box(predictions, l->biases, n, box_index, col, row, l->inWidth, l->inHeight, l->inWidth*l->inHeight);
			if (masks) {
				//for (j = 0; j < l->coords - 4; ++j) {
				//	masks[index][j] = l->output[mask_index + j*l->w*l->h];
				//}
			}

			int class_index = entry_index(l, 0, n*l->inWidth*l->inHeight + i, l->coords + !l->background);
			if (l->softmax_tree) {
				//hierarchy_predictions(predictions + class_index, l->classes, l->softmax_tree, 0, l->inWidth*l->inHeight);
				//if (map) {
				//	for (j = 0; j < 200; ++j) {
				//		int class_index = entry_index(l, 0, n*l->inWidth*l->inHeight + i, l->coords + 1 + map[j]);
				//		float prob = scale*predictions[class_index];
				//		probs[index][j] = (prob > thresh) ? prob : 0;
				//	}
				//}
				//else {
				//	int j = hierarchy_top_prediction(predictions + class_index, l->softmax_tree, tree_thresh, l->inWidth*l->inHeight);
				//	probs[index][j] = (scale > thresh) ? scale : 0;
				//	probs[index][l->classes] = scale;
				//}
			}
			else {
				float max = 0;
				for (j = 0; j < l->classes; ++j) {
					int class_index = entry_index(l, 0, n*l->inWidth*l->inHeight + i, l->coords + 1 + j);
					float prob = scale*predictions[class_index];
					probs[index][j] = (prob > thresh) ? prob : 0;
					if (prob > max) max = prob;
				}
				probs[index][l->classes] = max;
			}
			if (only_objectness) {
				probs[index][0] = scale;
			}
		}
	}
	correct_region_boxes(boxes, l->inWidth*l->inHeight*l->num, w, h, netw, neth, relative);
}

typedef struct {
	int index;
	int cls;
	float **probs;
} sortable_bbox;

float overlap(float x1, float w1, float x2, float w2) {
	float l1 = x1 - w1 / 2;
	float l2 = x2 - w2 / 2;
	float left = l1 > l2 ? l1 : l2;
	float r1 = x1 + w1 / 2;
	float r2 = x2 + w2 / 2;
	float right = r1 < r2 ? r1 : r2;
	return right - left;
}

float box_intersection(YoloBox a, YoloBox b) {
	float w = overlap(a.x, a.w, b.x, b.w);
	float h = overlap(a.y, a.h, b.y, b.h);
	if (w < 0 || h < 0) return 0;
	float area = w*h;
	return area;
}

float box_union(YoloBox a, YoloBox b) {
	float i = box_intersection(a, b);
	float u = a.w*a.h + b.w*b.h - i;
	return u;
}

float box_iou(YoloBox a, YoloBox b) {
	return box_intersection(a, b) / box_union(a, b);
}

int nms_comparator(const void *pa, const void *pb) {
	sortable_bbox a = *(sortable_bbox *)pa;
	sortable_bbox b = *(sortable_bbox *)pb;
	float diff = a.probs[a.index][b.cls] - b.probs[b.index][b.cls];
	if (diff < 0) return 1;
	else if (diff > 0) return -1;
	return 0;
}

void do_nms_obj(YoloBox *boxes, float **probs, int total, int classes, float thresh) {
	int i, j, k;
	sortable_bbox *s = new sortable_bbox[total];

	for (i = 0; i < total; ++i) {
		s[i].index = i;
		s[i].cls = classes;
		s[i].probs = probs;
	}

	qsort(s, total, sizeof(sortable_bbox), nms_comparator);
	for (i = 0; i < total; ++i) {
		if (probs[s[i].index][classes] == 0) continue;
		YoloBox a = boxes[s[i].index];
		for (j = i + 1; j < total; ++j) {
			YoloBox b = boxes[s[j].index];
			if (box_iou(a, b) > thresh) {
				for (k = 0; k < classes + 1; ++k) {
					probs[s[j].index][k] = 0;
				}
			}
		}
	}
	delete[] s;
}

int max_index(float *a, int n) {
	if (n <= 0) return -1;
	int i, max_i = 0;
	float max = a[0];
	for (i = 1; i < n; ++i) {
		if (a[i] > max) {
			max = a[i];
			max_i = i;
		}
	}
	return max_i;
}

void softmax(float *input, int n, float temp, int stride, float *output)
{
	int i;
	float sum = 0;
	float largest = -FLT_MAX;
	for (i = 0; i < n; ++i) {
		if (input[i*stride] > largest) largest = input[i*stride];
	}
	for (i = 0; i < n; ++i) {
		float e = (float)exp(input[i*stride] / temp - largest / temp);
		sum += e;
		output[i*stride] = e;
	}
	for (i = 0; i < n; ++i) {
		output[i*stride] /= sum;
	}
}


void softmax_cpu(float *input, int n, int batch, int batch_offset, int groups, int group_offset, int stride, float temp, float *output)
{
	int g, b;
	for (b = 0; b < batch; ++b) {
		for (g = 0; g < groups; ++g) {
			softmax(input + b*batch_offset + g*group_offset, n, temp, stride, output + b*batch_offset + g*group_offset);
		}
	}
}

//////

YoloRegionLayer::YoloRegionLayer(YoloRegionLayerParams *params) {
	layerType = YOLO_REGION;

	inWidth = params->inputWidth;
	inHeight = params->inputHeight;

	classes = params->classes;
	coords = params->coords;
	num = params->num;
	background = 0;
	softmax = 1;
	softmax_tree = 0;

	inputCount = params->inputWidth * params->inputHeight * params->num * (params->classes + params->coords + 1);
	outputCount = inputCount;
	output = new float[inputCount];

	biasesCount = params->num * 2;
	biases = new float[biasesCount];

	int size = params->inputWidth * params->inputHeight * params->num;
	boxes = new YoloBox[size];
	probs = new float*[size];
	for (int j = 0; j < size; ++j) {
		probs[j] = new float[params->classes + 1];
	}
}

YoloRegionLayer::~YoloRegionLayer() {
	delete[] output;
	delete[] biases;
	delete[] boxes;
	int size = inWidth * inHeight * num;
	for (int i = 0; i < size; i++)
		delete probs[i];
	delete probs;
}

void YoloRegionLayer::Forward() {
	std::memcpy(output, input, inputCount * sizeof(float));

	int index, n;
	int spacial = inWidth * inHeight;

	for (n = 0; n < num; ++n) {
		index = entry_index(this, 0, n * spacial, 0);
		ActivationHelper::ActivateArray(output + index, 2 * spacial, LOGISTIC);
		index = entry_index(this, 0, n * spacial, coords);
		if (!background) {
			ActivationHelper::ActivateArray(output + index, spacial, LOGISTIC);
		}
	}

	if (softmax_tree) {
		//int i;
		//int count = layer->coords + 1;
		//for (i = 0; i < l.softmax_tree->groups; ++i) {
		//	int group_size = l.softmax_tree->group_size[i];
		//	softmax_cpu(output + count, group_size, l.batch, l.inputs, l.n*l.w*l.h, 1, l.n*l.w*l.h, l.temperature, l.output + count);
		//	count += group_size;
		//}
	}
	else if (softmax) {
		index = entry_index(this, 0, 0, coords + !background);
		softmax_cpu(input + index, classes + background, num, inputCount / num,
			spacial, 1, spacial, 1, output + index);
	}
}

YoloRegion* YoloRegionLayer::ReadRegions(int imageWidth, int imageHeight, int firstLayerWidth, int firstLayerHeight, float thresh, int *regionCount) {
	float **masks = 0;
	float hier_thresh = 0.5f;
	float nms = 0.3f;
	int spacial = inWidth * inHeight * num;

	get_region_boxes(this, imageWidth, imageHeight, firstLayerWidth, firstLayerHeight, thresh, probs, boxes, masks, 0, 0, hier_thresh, 1);
	if (nms) {
		do_nms_obj(boxes, probs, spacial, classes, nms);
	}

	float probability;
	int i, classIndex, r = 0;
	for (i = 0; i < spacial; i++) {
		classIndex = max_index(probs[i], classes);
		probability = probs[i][classIndex];
		if (probability > thresh) r++;
	}

	*regionCount = r;
	YoloRegion *regions = new YoloRegion[r];

	r = 0;
	for (i = 0; i < spacial; i++) {
		classIndex = max_index(probs[i], classes);
		probability = probs[i][classIndex];
		if (probability > thresh) {
			regions[r].probability = probability;
			regions[r].box = boxes[i];
			regions[r].classIndex = classIndex;
			r++;
		}
	}

	return regions;
}
