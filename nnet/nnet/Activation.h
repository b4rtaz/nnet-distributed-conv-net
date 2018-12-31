#pragma once
#include <cmath>

namespace nNet {

	typedef enum {
		LOGISTIC, RELU, RELIE, LINEAR, RAMP, TANH, PLSE, LEAKY, ELU, LOGGY, STAIR, HARDTAN, LHTAN
	} Activation;

	inline float ActivateLogistic(float x) {
		return (float)(1. / (1. + exp(-x)));
	}

	inline float ActivateLeaky(float x) {
		return (x < 0) ? (0.1f * x) : x;
	}

	class ActivationHelper {
	public:
		static void ActivateArray(float *arr, const int size, Activation activation);
	};
}
