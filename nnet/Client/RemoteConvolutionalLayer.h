#pragma once
#include "../nNet/ConvolutionalFragment.h"
#include "ClientsManager.h"
#include <vector>

using namespace std;
using namespace nNet;

namespace Client {

	class RemoteConvolutionalLayer : public ConvolutionalLayer {
	private:
		ClientsManager *_clientsManager;

	public:
		RemoteConvolutionalLayer(ConvolutionalLayerParams *params, ClientsManager *clientsManager);

		void Forward(ConvolutionalFragmentParams *params) override;
		void Forward() override;
		void PropagateState() override;
	};
}
