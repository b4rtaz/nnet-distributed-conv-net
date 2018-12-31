#include "../nNet/LayerFactory.h"

using namespace nNet;

namespace Server {

	class WorkerServerLayerFactory : public LayerFactory {
	private:
		bool _withBlocks;

	public:
		WorkerServerLayerFactory(bool withBlocks);

		ConvolutionalLayer* CreateConvolutionalLayer(ConvolutionalLayerParams *params) override;
		ConvolutionalBlockLayer* CreateConvolutionalBlockLayer(ConvolutionalBlockLayerParams *params) override;
	};
}
