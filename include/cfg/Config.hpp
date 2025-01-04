#pragma once

#include <string>
#include <vector>
#include <functional>
#include <map>
#include <crow_all.h>
#include <ordered_map.hpp>
#include <util.hpp>

/**
 * A structure that represents a single configuration property
 * @code
 * &#91;section&#93;
 * key = value
 * @endcode
 */
struct CfgProperty {
        std::string key;
        std::string value;
        std::string section;

        bool operator==(const char *path) const;
};

enum DaoSelectType {
        SELECT_SINGLE,
        SELECT_MULTI
};

/**
 * Represents a single SELECT function
 */
struct DaoFunction {
        DaoSelectType type;
        std::string identifier;
        std::string query;
        std::string type_mapping;
        ordered_map<std::string, std::string> params;
};

/**
 * A structure that represents the configuration used for generating
 * select functions for retrieving DAOs from the database
 */
struct DaoConfig {
        std::vector<DaoFunction> functions;
};

[[nodiscard]] bool parse_dao_config(DaoConfig *dst);

struct DbConfig {
        std::string user;
        std::string password;
        std::string db;
        std::string host;
        int port;
};

[[nodiscard]] bool parse_db_config(DbConfig *dst);

struct MalphasConfig {
        int port;

        MalphasConfig() : port(0) {}
};

[[nodiscard]] bool parse_malphas_config(MalphasConfig *dst);

/**
 * Config file parser and validator.
 */
class Config {
        private:
                std::vector<CfgProperty> props;
                const char *path;
                char *_buffer;
                unsigned int buff_len;
                bool parsed;

        private:
                std::unordered_map<std::string, std::function<bool(std::string)> > required;

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
                [[nodiscard]] std::string operator[](const std::string &path);

                [[nodiscard]] std::unordered_map<std::string, CfgProperty> get_sections() const;
};
