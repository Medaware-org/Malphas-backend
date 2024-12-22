#include <cfg/Config.hpp>
#include <cstdio>
#include <crow_all.h>
#include <cstdlib>
#include <utility>
#include <util.hpp>

bool cfg_prop::operator==(const char *path) const
{
        return path == (this->section + "." + this->key);
}

Config::Config(const char *path) : path(path), _buffer(nullptr), parsed(false)
{
}

void Config::add_required(const std::string &str)
{
        add_required(str, [](const std::string &) { return true; });
}

void Config::add_required(const std::string &str, std::function<bool(std::string)> validation)
{
        auto iter = required.find(str);

        if (iter != this->required.end())
                return;

        this->required[str] = std::move(validation);
}

Config::~Config()
{
        if (!this->_buffer)
                return;

        free((void *) this->_buffer);
}

#define IS_SPACE(c) (c == ' ' || c == '\t')

int skip_spaces(char **buff)
{
        int nSkip = 0;
        while (IS_SPACE(**buff)) {
                (*buff)++;
                nSkip++;
        }
        return nSkip;
}

#undef IS_SPACE

#define IS_IDEN(c) ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_')

int parse_identifier(char **buff, std::string &dst)
{
        int nLen = 0;
        while (IS_IDEN(**buff)) {
                dst.push_back(**buff);
                (*buff)++;
                nLen++;
        }
        return nLen;
}

#undef IS_IDEN

int parse_value(char **buff, std::string &dst)
{
        int nLen = 0;
        while (**buff != '\n' && **buff != '\0') {
                dst.push_back(**buff);
                (*buff)++;
                nLen++;
        }
        return nLen;
}

bool Config::parse()
{
        if (parsed)
                return true;

        this->_buffer = read_file(this->path, &this->buff_len);

        if (!_buffer)
                return false;

        char *buffer = this->_buffer;
        char c;

        // Temporary buffers
        std::string key;
        std::string value;
        std::string section = "default";

        while (true) {
                skip_spaces(&buffer);
                c = *buffer;

                if (c == '\n') {
                        buffer++;
                        continue;
                }

                // line comment
                if (c == '#') {
                        while (c != '\n' && c)
                                c = *(++buffer);
                        continue;
                }

                if (!c) break; // EOF

                // Beginning of a section
                if (c == '[') {
                        buffer++;
                        section.clear();
                        if (parse_identifier(&buffer, section) <= 0) {
                                CROW_LOG_CRITICAL << "Could not parse config section identifier starting with '"
                                                  << (*buffer) << "'";
                                return false;
                        }
                        c = *buffer;
                        if (c != ']') {
                                CROW_LOG_CRITICAL << "Expected ']' after section identifier \"" << section
                                                  << "\", got '" << c << "'";
                                return false;
                        }
                        buffer++;
                        continue;
                }

                key.clear();
                value.clear();

                if (parse_identifier(&buffer, key) <= 0) {
                        CROW_LOG_CRITICAL << "Could not parse property identifier starting with '" << c << "'";
                        return false;
                }

                skip_spaces(&buffer);

                c = *buffer;

                if (c != '=') {
                        CROW_LOG_CRITICAL << "Expected '=' after property identifier: " << key << ", got '" << c << "'";
                        return false;
                }

                buffer++; // Skip '='

                skip_spaces(&buffer);

                if (parse_value(&buffer, value) <= 0) {
                        CROW_LOG_CRITICAL << "Could not parse value of property: " << key;
                        return false;
                }

                props.push_back(cfg_prop{
                        .key = key,
                        .value = value,
                        .section = section
                });

                skip_spaces(&buffer);

                c = *buffer;

                if (c == '\n')
                        buffer++;
        }

        // We've already parsed the config, so there's no need to keep the original buffer around
        free(_buffer);
        _buffer = nullptr;

        return (parsed = true);
}

bool Config::validate()
{
        if (!this->parsed)
                return false;

        bool ok = true;

        std::unordered_map<std::string, bool> statuses;
        for (auto &[key, value]: required)
                statuses[key] = false;
        std::for_each(this->props.begin(), this->props.end(), [&](cfg_prop &prop) {
                std::string key = prop.section + "." + prop.key;
                if (statuses.find(key) == statuses.end()) {
                        CROW_LOG_CRITICAL << "Unknown configuration property: " << key;
                        ok = false;
                        return;
                }
                statuses[key] = true;
        });
        for (auto &[key, value]: statuses) {
                if (value) {
                        auto cb = required[key];
                        if (!cb(std::string((*this)[key.c_str()]))) {
                                CROW_LOG_CRITICAL << "Property '" << key << "' did not pass value validation";
                                ok = false;
                        }
                        continue;
                }

                CROW_LOG_CRITICAL << "Required configuration property not set: " << key;
                ok = false;
        }
        return ok;
}

std::string Config::operator[](const char *path)
{
        for (auto &prop: this->props)
                if (path == (prop.section + "." + prop.key))
                        return prop.value;

        return "";
}