#pragma once
#include "Network.h"
#include "LayerFactory.h"
#include "IniReader.h"

using namespace std;

namespace nNet {

	class YoloNetParser {
	private:
		LayerFactory *_layerFactory;

		Network *_network;
		int _outWidth;
		int _outHeight;
		int _outChannels;
		bool _verbose;

	public:

		YoloNetParser(bool verbose, LayerFactory *layerFactory);
		Network* Read(char *path);
		void ParseSection(IniParser *parser);

	private:
		static bool ParserHandler(void* user, IniParser *parser);
	};
}
