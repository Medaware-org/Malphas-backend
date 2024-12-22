#pragma once

#include <string>
#include <vector>
#include <functional>
#include <map>


struct cfg_prop
{
	std::string key;
	std::string value;
	std::string section;

	bool operator==(const char* path) const;
};

class Config
{
private:
	std::vector<cfg_prop> props;
	const char* path;
	char* _buffer;
	unsigned int buff_len;
	bool parsed;
private:
	std::unordered_map<std::string, std::function<bool(std::string)>> required;
public:
	Config(const char* path);

	~Config();

	void add_required(std::string str);
	void add_required(std::string str, std::function<bool(std::string)> validation);

	bool parse();

	bool validate();

	std::string operator[](const char* path);

private:
	bool read_file();
};

//
// Parser Utilities
//

int skip_spaces(char** buff);

int parse_identifier(char** buff, std::string& dst);

int parse_value(char** buff, std::string& dst);
