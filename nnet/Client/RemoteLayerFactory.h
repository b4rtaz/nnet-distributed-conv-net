#include "../nNet/LayerFactory.h"
#include "ClientsManager.h"

using namespace nNet;

namespace Client {

	class RemoteLayerFactory : public LayerFactory {
	private:
		ClientsManager *_clientsManager;
		bool _withBlocks;

	public:
		RemoteLayerFactory(ClientsManager *clientsManager, bool withBlocks);
		ConvolutionalLayer* CreateConvolutionalLayer(ConvolutionalLayerParams *params) override;
		ConvolutionalBlockLayer* CreateConvolutionalBlockLayer(ConvolutionalBlockLayerParams *params) override;
	};
}
