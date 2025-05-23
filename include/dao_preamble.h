#pragma once

//
// Type Conversion
//

#define NO_CAST(x) (x)

#define IFNNULL(n, _else) (PQgetisnull(result, tuple, n) ? std::nullopt : std::optional(_else))

inline std::string sql_sanitize(std::string str)
{
        std::string sanitized;
        for (const auto &c: str) {
                switch (c) {
                        case '\\':
                                sanitized += "\\\\";
                                break;
                        case '\'':
                                sanitized += "''";
                                break;
                        case '\"':
                                sanitized += "\\\"";
                                break;
                        default:
                                sanitized += c;
                }
        }
        return sanitized;
}

[[nodiscard]] inline bool cast_bool(std::string &&str) { return (str == "true"); }

template<typename T>
[[nodiscard]] typename std::enable_if<!std::is_same<T, std::string>::value && !std::is_same<T, bool>::value,
        std::string>::type xto_string_impl(T arg)
{
        return std::to_string(arg);
}

template<typename T>
[[nodiscard]] typename std::enable_if<std::is_same<T, std::string>::value, std::string>::type xto_string_impl(T arg)
{
        return arg;
}

template<typename T>
[[nodiscard]] typename std::enable_if<std::is_same<T, bool>::value, std::string>::type xto_string_impl(T arg)
{
        return (arg ? "true" : "false");
}

template<typename T>
[[nodiscard]] std::string xto_string(T arg)
{
        return sql_sanitize(xto_string_impl(arg));
}

//
// SQL Query Utilities
//

inline bool finalize_op(PGresult *res)
{
        if (!res)
                return false;
        PQclear(res);
        return true;
}

inline PGresult *dao_query(Database &db, std::string query, ExecStatusType assert_status)
{
        CROW_LOG_INFO << "Query: " << query;
        ExecStatusType status;
        PGresult *res = db.query(query, &status);
        if (status != assert_status) {
                PQclear(res);
                return NULL;
        }
        return res;
}

//
// Generic Mapper Function
//

template<typename T>
inline void dao_map_all(PGresult *res, std::vector<T> &dst, std::function<T(PGresult *res, int tuple)> mapper)
{
        for (int i = 0; i < PQntuples(res); i++)
                dst.push_back(mapper(res, i));
}
