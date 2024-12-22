#pragma once

#include <string>
#include <vector>
#include <functional>

struct cfg_prop {
        std::string key;
        std::string value;
        std::string section;

        bool operator ==(const char *path) const;
};

class Config {
    private:
        std::vector<cfg_prop> props;
        const char *path;
        char *_buffer;
        unsigned int buff_len;
        bool parsed;
    public:
        Config(const char *path);

        ~Config();

        bool parse(std::function<void(cfg_prop &)> cb);

    private:
        bool read_file();
};

//
// Parser Utilities
//

int skip_spaces(char **buff);

int parse_identifier(char **buff, std::string &dst);

int parse_value(char **buff, std::string &dst);
