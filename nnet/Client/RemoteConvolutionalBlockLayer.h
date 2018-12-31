#include "../nNet/ConvolutionalBlockLayer.h"
#include "ClientsManager.h"
#include <vector>

using namespace nNet;
using namespace std;

namespace Client {

	class RemoteConvolutionalBlockLayer : public ConvolutionalBlockLayer {
	private:
		ClientsManager *_clientsManager;
		vector<BlockLayerFragmentParams> _fragments;

	public:
		RemoteConvolutionalBlockLayer(ConvolutionalBlockLayerParams *params, ClientsManager *clientsManager);
		~RemoteConvolutionalBlockLayer();

		void Forward(vector<BlockLayerFragmentParams> *fragments) override;
		void Forward() override;
		void PropagateState() override;
	};
}
