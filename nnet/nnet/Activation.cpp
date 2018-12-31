#include "Activation.h"
#include <exception>

using namespace nNet;
using namespace std;

void ActivationHelper::ActivateArray(float *arr, const int size, Activation activation) {
	if (activation == LINEAR)
		return;

	float (*activate)(float);
	if (activation == LEAKY)
		activate = ActivateLeaky;
	else if (activation == LOGISTIC)
		activate = ActivateLogistic;
	else
		new exception("Unknow activation.");

	for (int i = 0; i < size; ++i) {
		arr[i] = activate(arr[i]);
	}
}
