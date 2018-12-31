#include "IniReader.h"
#include <cstdio>
#include <cstring>
#include <cctype>

using namespace nNet;
using namespace std;

typedef int(*ini_handler)(void* user, const char* section, bool is_new_section, const char* name, const char* value);

#define INI_MAX_LINE 256
#define INI_ALLOW_INLINE_COMMENTS 1
#define INI_ALLOW_MULTILINE 1
#define INI_INLINE_COMMENT_PREFIXES "#"
#define MAX_SECTION 50
#define MAX_NAME 50
#define INI_STOP_ON_FIRST_ERROR 1

inline static char* find_chars_or_comment(const char* s, const char* chars)
{
#if INI_ALLOW_INLINE_COMMENTS
	int was_space = 0;
	while (*s && (!chars || !strchr(chars, *s)) &&
		!(was_space && strchr(INI_INLINE_COMMENT_PREFIXES, *s))) {
		was_space = isspace((unsigned char)(*s));
		s++;
	}
#else
	while (*s && (!chars || !strchr(chars, *s))) {
		s++;
	}
#endif
	return (char*)s;
}

inline static char* rstrip(char* s)
{
	char* p = s + strlen(s);
	while (p > s && isspace((unsigned char)(*--p)))
		*p = '\0';
	return s;
}

inline static char* strncpy0(char* dest, const char* src, size_t size)
{
	strncpy(dest, src, size);
	dest[size - 1] = '\0';
	return dest;
}

inline static char* lskip(const char* s)
{
	while (*s && isspace((unsigned char)(*s)))
		s++;
	return (char*)s;
}

inline int ini_parse_stream(FILE* stream, ini_handler handler, void* userParam) {
	char line[INI_MAX_LINE];

	char section[MAX_SECTION] = "";
	bool is_new_section = false;
	char prev_name[MAX_NAME] = "";

	char* start;
	char* end;
	char* name;
	char* value;
	int lineno = 0;
	int error = 0;

	/* Scan through stream line by line */
	while (fgets(line, INI_MAX_LINE, stream) != NULL) {
		lineno++;

		start = line;
#if INI_ALLOW_BOM
		if (lineno == 1 && (unsigned char)start[0] == 0xEF &&
			(unsigned char)start[1] == 0xBB &&
			(unsigned char)start[2] == 0xBF) {
			start += 3;
		}
#endif
		start = lskip(rstrip(start));

		if (*start == ';' || *start == '#') {
			/* Per Python configparser, allow both ; and # comments at the
			start of a line */
		}
#if INI_ALLOW_MULTILINE
		else if (*prev_name && *start && start > line) {

#if INI_ALLOW_INLINE_COMMENTS
			end = find_chars_or_comment(start, NULL);
			if (*end)
				*end = '\0';
			rstrip(start);
#endif

			/* Non-blank line with leading whitespace, treat as continuation
			of previous name's value (as per Python configparser). */
			if (!handler(userParam, section, is_new_section, prev_name, start) && !error)
				error = lineno;
			is_new_section = false;
		}
#endif
		else if (*start == '[') {
			/* A "[section]" line */
			end = find_chars_or_comment(start + 1, "]");
			if (*end == ']') {
				*end = '\0';
				strncpy0(section, start + 1, sizeof(section));
				*prev_name = '\0';
				is_new_section = true;
			}
			else if (!error) {
				/* No ']' found on section line */
				error = lineno;
			}
		}
		else if (*start) {
			/* Not a comment, must be a name[=:]value pair */
			end = find_chars_or_comment(start, "=:");
			if (*end == '=' || *end == ':') {
				*end = '\0';
				name = rstrip(start);
				value = lskip(end + 1);
#if INI_ALLOW_INLINE_COMMENTS
				end = find_chars_or_comment(value, NULL);
				if (*end)
					*end = '\0';
#endif
				rstrip(value);

				/* Valid name[=:]value pair found, call handler */
				strncpy0(prev_name, name, sizeof(prev_name));
				if (!handler(userParam, section, is_new_section, name, value) && !error)
					error = lineno;
				is_new_section = false;
			}
			else if (!error) {
				/* No '=' or ':' found on name[=:]value line */
				error = lineno;
			}
		}

#if INI_STOP_ON_FIRST_ERROR
		if (error)
			break;
#endif
	}

	return error;
}

IniParser::IniParser(void *userParam, IniParserHandler handler) {
	this->userParam = userParam;
	this->handler = handler;
}

int IniParser::Parse(const char *path) {
	FILE* file = fopen(path, "r");

	int err = ini_parse_stream(file, ParserHandler, this);
	if (err == 0 && !sectionParams.empty()) {
		handler(userParam, this);
	}

	fclose(file);
	return err;
}

int IniParser::ParserHandler(void* user, const char* section, bool isNewSection, const char* name, const char* value) {
	IniParser *parser = (IniParser*)user;
	if (isNewSection) {
		if (!parser->sectionName.empty()) {
			parser->handler(parser->userParam, parser);
		}
		parser->sectionName = section;
		parser->sectionParams.clear();
	}
	parser->sectionParams[name] = value;
	return 1;
}

const char* IniParser::GetSectionName() {
	return sectionName.c_str();
}

bool IniParser::GetBoolParam(char *name, bool defaultValue) {
	return GetIntParam(name, defaultValue) == 1;
}

int IniParser::GetIntParam(char *name, int defaultValue) {
	const char *value = GetStringParam(name);
	if (value != NULL) {
		return atoi(value);
	}
	return defaultValue;
}

const char* IniParser::GetStringParam(char *name) {
	if (sectionParams.find(name) != sectionParams.end()) {
		return sectionParams[name].c_str();
	}
	return NULL;
}
