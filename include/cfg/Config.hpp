#pragma once

#include <string>
#include <vector>
#include <functional>
#include <map>
#include <crow_all.h>

/**
 * A structure that represents a single configuration property
 * @code
 * &#91;section&#93;
 * key = value
 * @endcode
 */
struct cfg_prop {
        std::string key;
        std::string value;
        std::string section;

        bool operator==(const char *path) const;
};

struct db_config {
        std::string user;
        std::string password;
        std::string db;
        std::string host;
        int port;

};

extern "C" [[nodiscard]] bool parse_db_config(db_config *dst);

/**
 * Config file parser and validator.
 */
class Config {
    private:
        std::vector<cfg_prop> props;
        const char *path;
        char *_buffer;
        unsigned int buff_len;
        bool parsed;
    private:
        std::unordered_map<std::string, std::function<bool(std::string)>> required;
    public:
        Config(const char *path);

        ~Config();

        /**
         * Adds a required field to the validator
         * @param str The parameter identifier in the form <code>section.key</code>
         */
        void add_required(const std::string &str);

        /**
         * Adds a required field with a custom value validation function to the validator.
         * @param str The parameter identifier in the form <code>section.key</code>
         * @param validation The validation function. The property will not pass validation if the function
         * returns <code>false</code>
         */
        void add_required(const std::string &str, std::function<bool(std::string)> validation);

        /**
         * Parse the configuration file. This does <b>not</b> perform config validation.
         * @returns <code>true</code> if parsing succeeded, otherwise <code>false</code>.
         */
        [[nodiscard]] bool parse();

        /**
         * Perform config validation. For the process to work properly, <code>parse()</code> must be invoked
         * beforehand.
         * @returns <code>true</code> if the config is valid, otherwise <code>false</code>.
         */
        [[nodiscard]] bool validate();

        /**
         * Retrieve the value of a property.
         * @param path The path of the property in the form <code>section.key</code>
         */
        [[nodiscard]] std::string operator[](const char *path);
};
