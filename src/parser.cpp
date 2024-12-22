#include <parser.hpp>

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