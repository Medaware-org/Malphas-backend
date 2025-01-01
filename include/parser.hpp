#pragma once

#include <string>

/**
 * Config parser utility. Increments <code>*buff</code> as long as the base points to a non-space character.
 * @param buff The string buffer
 * @returns Number of characters that were skipped
 */
int skip_spaces(char **buff);

/**
 * Config parser utility. Increments <code>*buff</code> while the current character is alphanumeric and appends
 * every such character to <code>dst</code>.
 * @param buff The string buffer
 * @param dst The destination string that will hold the resulting identifier
 * @returns Number of characters that the buffer was advanced by.
 */
int parse_identifier(char **buff, std::string &dst);

int parse_until(char **buff, std::string &dst, char delim);

/**
 * Config parser utility. Increments <code>*buff</code> until the current character is a <code>\\n</code> or <code>\\0</code>.
 * Each character is appended to <code>dst</code>.
 * @param buff The string buffer
 * @param dst The destination string that will hold the resulting identifier
 * @returns Number of characters that the buffer was advanced by.
 */
int parse_value(char **buff, std::string &dst);