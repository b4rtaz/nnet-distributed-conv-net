#include <string>
#include <map>

using namespace std;

namespace nNet {

	class IniParser;

	typedef bool (*IniParserHandler)(void* userParam, IniParser *parser);

	class IniParser {
	public:
		void *userParam;
		IniParserHandler handler;
		string sectionName;
		map<string, string> sectionParams;

		IniParser(void *userParam, IniParserHandler handler);

		int Parse(const char *initPath);

		const char *GetSectionName();
		bool GetBoolParam(char *name, bool defaultValue);
		int GetIntParam(char *name, int defaultValue);
		const char *GetStringParam(char *name);

	private:
		static int ParserHandler(void* user, const char* section, bool isNewSection, const char* name, const char* value);
	};
}
