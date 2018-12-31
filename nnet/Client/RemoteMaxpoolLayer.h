#pragma once
#include "../nNet/MaxpoolLayer.h"
#include "ClientsManager.h"
#include <vector>

using namespace std;
using namespace nNet;

namespace Client {

	class RemoteMaxpoolLayer : public MaxpoolLayer {
	private:
		ClientsManager *_clientsManager;

	public:
		RemoteMaxpoolLayer(MaxpoolLayerParams *params, ClientsManager *clientsManager);

		void Forward(MaxpoolFragmentParams *params) override;
		void Forward() override;
		void PropagateState() override;
	};
}
