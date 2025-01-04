/**
 * This script's purpose is to generate DAOs from an already existing database.
 * It accomplishes this by connecting to said DB, querying the tables of the 'public' schema,
 * and generating structs as well as utility functions for each table after performing
 * SQL to C++ type mappings. This script is meant to run >after< the database has been migrated!
 **/

#include <string>
#include <iostream>
#include <algorithm>
#include <cfg/Config.hpp>
#include <libpq-fe.h>
#include <map>
#include <tuple>
#include <ordered_map.hpp>

struct type_mapping {
        std::string cpp; // The C++ type
        std::string function; // THe conversion function
};

struct table_field {
        std::string name;
        std::string type;
        bool is_primary;
        bool is_nullable;
};

bool needs_quotes(const std::string &type)
{
        return type == "text" || type.find("varchar") != std::string::npos || type == "uuid" || type == "std::string";
}

std::string prep_value(const table_field &field, std::string param, bool last)
{
        std::string str;

        str.append("\" + ");

        std::string leftQuotes = (needs_quotes(field.type) ? "std::string(\"'\") + " : "");
        std::string rightQuotes = (needs_quotes(field.type) ? " + std::string(\"'\")" : "");

        if (field.is_nullable)
                str.append("(" + param + " ? (" + leftQuotes + "xto_string(*" + param + ")" + rightQuotes +
                           ") : \"null\")");
        else
                str.append(leftQuotes + "xto_string(" + param + ")" + rightQuotes);

        if (last && !needs_quotes(field.type))
                return str;

        str.append(" + \"");

        return str;
}

PGconn *connect_db(std::string user, std::string password, std::string db, std::string host, int port)
{
        std::string conn_str = std::format("user={} password={} dbname={} port={} host={}", user, password, db, port,
                                           host);

        PGconn *conn = PQconnectdb(conn_str.c_str());

        if (PQstatus(conn) != CONNECTION_OK) {
                printf("ERR: Could not connect to the database.\n");
                PQfinish(conn);
                return nullptr;
        }

        return conn;
}

/*
 * This function maps the type from PostgreSQL relations to C++ types
 */
std::pair<type_mapping, std::string> map_types(const table_field &field,
                                               const std::map<std::string, type_mapping> &type_mappings)
{
        type_mapping mapping;

        auto iter = type_mappings.find(field.type);
        if (iter == type_mappings.end())
                mapping = type_mapping{.cpp = "std::any", .function = "std::any"};
        else mapping = iter->second;

        std::string type_str;

        if (field.is_nullable)
                type_str = std::format("std::optional<{}>", mapping.cpp);
        else
                type_str = mapping.cpp;

        return std::make_pair(mapping, type_str);
}

void emit_struct(const std::string &table, const ordered_map<std::string, table_field> &layout,
                 const std::map<std::string, type_mapping> &type_mappings)
{
        std::cout << "struct " << table << " {" << std::endl;
        layout.for_each([&](const auto &column, const auto &field) {
                std::cout << "\t";
                if (field.is_nullable)
                        std::cout << "std::optional<";
                std::cout << map_types(field, type_mappings).second;
                if (field.is_nullable)
                        std::cout << ">";
                std::cout << " " << column << ";" << std::endl;
        });
        std::cout << "};\n\n";
}

void emit_spread_macro(const std::string &table, const ordered_map<std::string, table_field> &layout,
                       const std::map<std::string, type_mapping> &type_mappings)
{
        std::string upper_table = std::string(table);
        std::transform(table.begin(), table.end(), upper_table.begin(), ::toupper);

        size_t nFields = layout.size();

        std::cout << "#define SPREAD_" << upper_table << "(" << table << "_struct" << ") ";
        int index = 0;
        layout.for_each([&](const auto &column, const auto &field) {
                std::cout << table << "_struct." << column;
                if ((++index) < nFields)
                        std::cout << ", ";
        });
        std::cout << std::endl;

        index = 0;
        std::cout << "#define SPREAD_" << upper_table << "_PTR(" << table << "_struct" << ") ";
        layout.for_each([&](const auto &column, const auto &field) {
                std::cout << table << "_struct->" << column;
                if ((++index) < nFields)
                        std::cout << ", ";
        });

        std::cout << "\n\n";
}


/*
 * Generate an insert function for a given table
 */
void emit_insert(const std::string &table, const ordered_map<std::string, table_field> &layout,
                 const std::map<std::string, type_mapping> &type_mappings)
{
        size_t nTuples = layout.size();

        std::cout << "[[nodiscard]] inline bool " << table << "_insert(Database &db, ";
        int index = 0;
        layout.for_each([&](const auto &column, const auto &field) {
                std::cout << map_types(field, type_mappings).second << ((field.is_primary) ? " /*PK*/ " : " ") <<
                        column;
                std::cout << (((++index) < layout.size()) ? ", " : ") {\n");
        });

        std::cout << "\tstd::string query = \"INSERT INTO \\\"" << table << "\\\" (";
        index = 0;
        layout.for_each([&](const auto &column, const auto &field) {
                std::cout << column;
                std::cout << (((++index) < nTuples) ? ", " : ") VALUES (");
        });

        index = 0;
        layout.for_each([&](const auto &column, const auto &field) {
                bool quotes = needs_quotes(field.type);

                std::cout << prep_value(field, column, (index + 1) >= nTuples);

                std::cout << (((++index) < nTuples) ? ", " : ")\";\n");
        });


        std::cout << "\treturn finalize_op(dao_query(db, query, PGRES_COMMAND_OK));" << std::endl;
        std::cout << "}\n\n";
}

void emit_update(const std::string &table, const ordered_map<std::string, table_field> &layout,
                 const std::map<std::string, type_mapping> &type_mappings)
{
        ordered_map<std::string, table_field> filtered_layout = layout.filter([&](const auto &k, const auto &v) {
                return !v.is_primary;
        });

        if (filtered_layout.empty()) {
                std::cout << "// No update function for '" << table << "': The table only has primary key fields." <<
                        std::endl;
                return;
        }

        ordered_map<std::string, table_field> primary_keys = layout.filter([&](const auto &k, const auto &v) {
                return v.is_primary;
        });

        std::cout << "[[nodiscard]] inline bool " << table << "_update(Database &db, ";
        int index = 0;
        layout.for_each([&](const auto &column, const auto &field) {
                std::cout << map_types(field, type_mappings).second << ((field.is_primary) ? " /*PK*/ " : " ") <<
                        column;
                std::cout << (((++index) < layout.size()) ? ", " : ")\n{\n");
        });

        std::cout << "\tstd::string query = \"UPDATE \\\"" << table << "\\\" SET ";

        index = 0;
        filtered_layout.for_each([&](const auto &column, const auto &field) {
                std::cout << column << " = ";

                std::cout << prep_value(field, column, (index + 1) >= layout.size());

                std::cout << (((++index) < filtered_layout.size()) ? ", " : "");
        });

        std::cout << " WHERE ";
        index = 0;
        primary_keys.for_each([&](const auto &column, const auto &field) {
                bool quotes = needs_quotes(field.type);
                std::cout << column << " = ";
                if (quotes)
                        std::cout << "'";

                std::cout << "\" + xto_string(" << column << ") + \"";

                if (quotes)
                        std::cout << "'";

                if ((++index) < primary_keys.size())
                        std::cout << " AND ";
        });

        std::cout << ";\";" << std::endl;

        std::cout << "\treturn finalize_op(dao_query(db, query, PGRES_COMMAND_OK));" << std::endl;
        std::cout << "}\n\n";
}

void emit_save(const std::string &table, const ordered_map<std::string, table_field> &layout,
               const std::map<std::string, type_mapping> &type_mappings)
{
        std::cout << "[[nodiscard]] inline bool " << table << "_save(Database &db, ";

        std::vector<std::string> pk_vec;
        std::vector<std::string> param_vec;

        int index = 0;
        layout.for_each([&](const auto &column, const auto &field) {
                param_vec.push_back(column);
                if (field.is_primary)
                        pk_vec.push_back(column);
                std::cout << map_types(field, type_mappings).second << ((field.is_primary) ? " /*PK*/ " : " ") <<
                        column;
                std::cout << (((++index) < layout.size()) ? ", " : ")\n{\n");
        });

        index = 0;
        std::string sequence;
        for (const auto &param: param_vec) {
                sequence.append(param);
                if ((++index) < param_vec.size())
                        sequence.append(", ");
        }

        index = 0;
        std::string pk_sequence;
        for (const auto &param: pk_vec) {
                pk_sequence.append(param);
                if ((++index) < pk_vec.size())
                        pk_sequence.append(", ");
        }

        std::cout << "\t" << table << " tmp;" << std::endl;
        std::cout << "\tif (!get_one_" << table << "(db, &tmp, " << pk_sequence << "))" << std::endl;
        std::cout << "\t\treturn " << table << "_insert(db, " << sequence << ");" << std::endl;
        std::cout << "\treturn " << table << "_update(db, " << sequence << ");" << std::endl;
        std::cout << "}\n\n";
}

void emit_select(const std::string &table, const ordered_map<std::string, table_field> &layout,
                 const std::map<std::string, type_mapping> &type_mappings)
{
        ordered_map<std::string, table_field> primary_keys = layout.filter([&](const auto &k, const auto &v) {
                return v.is_primary;
        });

        int i = 0;
        std::string part_signature;
        primary_keys.for_each([&](const auto &column, const auto &field) {
                part_signature += map_types(field, type_mappings).second + " " + column;
                part_signature += (((i++) + 1 < primary_keys.size()) ? ", " : ")\n{\n");
        });

        std::string query_str = "\tstd::string query = \"SELECT * FROM \\\"" + table + "\\\" WHERE ";

        i = 0;
        primary_keys.for_each([&](const auto &column, const auto &field) {
                bool quote = needs_quotes(field.type);
                std::string quotes = (quote) ? "'" : "";
                query_str += column + " = " + quotes + "\" + xto_string(" + column + ")";
                query_str += (((i++) + 1 < primary_keys.size())
                                      ? ("\"" + quotes + " AND ")
                                      : ((quote ? "+ \"'\";\n" : ";\n")));
        });

        std::string exec_invoke = "\tPGresult *res = dao_query(db, query, PGRES_TUPLES_OK);\n"
                "\tif (!res) return false;";

        // Single result function
        std::cout << "[[nodiscard]] inline bool get_one_" << table << "(Database &db, " << table << " *dst, " <<
                part_signature;
        std::cout << query_str;
        std::cout << exec_invoke << std::endl;
        std::cout << "\tif (PQntuples(res) != 1) {\n\t\tPQclear(res);\n\t\treturn false;\n\t}" << std::endl;
        std::cout << "\t*dst = dao_map_" << table << "(res, 0);" << std::endl;
        std::cout << "\tPQclear(res);" << std::endl;
        std::cout << "\treturn true;" << std::endl;
        std::cout << "}\n\n";

        // Get all function
        std::cout << "[[nodiscard]] inline bool get_all_" << table << "(Database &db, std::vector<" << table <<
                "> &dst)\n{\n";
        std::cout << "\tstd::string query = \"SELECT * from \\\"" << table << "\\\"\";" << std::endl;
        std::cout << exec_invoke << std::endl;
        std::cout << "\tdao_map_all<" << table << ">(res, dst, [](auto *res, auto tuple) { return dao_map_" << table <<
                "(res, tuple); });" << std::endl;
        std::cout << "\tPQclear(res);" << std::endl;
        std::cout << "\treturn true;" << std::endl;
        std::cout << "}\n\n";
}

/*
 * Generate the mapper for a given relation
 */
void emit_dao_mapper(const std::string &table, const ordered_map<std::string, table_field> &layout,
                     const std::map<std::string, type_mapping> &type_mappings)
{
        std::cout << "[[nodiscard]] inline " << table << " dao_map_" << table << "(PGresult *result, int tuple) {" <<
                std::endl
                << "\treturn " << table << " {\n";

        int index = 0;
        layout.for_each([&](const auto &column, const auto &field) {
                type_mapping mapping = map_types(field, type_mappings).first;
                std::string function = mapping.function + "(PQgetvalue(result, tuple," + std::to_string(index) + "))";
                std::cout << "\t\t." << column << " = ";
                if (field.is_nullable)
                        std::cout << "IFNNULL(" << (index++) << ", " << function << ")," << std::endl;
                else {
                        std::cout << function << "," << std::endl;
                        index++;
                }
        });

        std::cout << "\t};\n}\n\n";
}

int serialise_table(PGconn *conn, std::string &table, const std::map<std::string, type_mapping> &type_mappings)
{
        std::string query = std::format(
                "SELECT * FROM information_schema.columns WHERE table_name = '{}' ORDER BY ordinal_position;", table);
        PGresult *res = PQexec(conn, query.c_str());

        if (PQresultStatus(res) != PGRES_TUPLES_OK) {
                std::cerr << "ERR: Could not query fields of table '" << table << "'" << std::endl;
                PQclear(res);
                return -1;
        }

        int nFields = PQntuples(res);

        // Whatever the fuck that does ... (PK checks)
        const std::string pkQuery = "SELECT kcu.column_name "
                                    "FROM information_schema.table_constraints tc "
                                    "JOIN information_schema.key_column_usage kcu "
                                    "ON tc.constraint_name = kcu.constraint_name "
                                    "WHERE tc.table_name = '" + table + "' "
                                    "AND tc.constraint_type = 'PRIMARY KEY';";

        PGresult *pkRes = PQexec(conn, pkQuery.c_str());

        if (PQresultStatus(pkRes) != PGRES_TUPLES_OK) {
                std::cerr << "ERR: Could not query primary keys of table '" << table << "'";
                PQclear(res);
                PQclear(pkRes);
                return -1;
        }

        int nPks = PQntuples(pkRes);
        std::vector<std::string> primary_keys;

        for (int i = 0; i < nPks; i++)
                primary_keys.push_back(PQgetvalue(pkRes, i, 0));

        ordered_map<std::string, table_field> fields;
        std::vector<std::string> fields_order;

        for (int i = 0; i < nFields; i++) {
                std::string column = PQgetvalue(res, i, 3);
                std::string type = PQgetvalue(res, i, 7);

                // Check if this column is nullable
                std::string nullableQuery = "SELECT IS_NULLABLE FROM INFORMATION_SCHEMA.COLUMNS WHERE TABLE_NAME = '"
                                            + table + "' AND COLUMN_NAME = '" + column + "';";

                PGresult *nullableRes = PQexec(conn, nullableQuery.c_str());

                if (PQresultStatus(pkRes) != PGRES_TUPLES_OK) {
                        std::cerr << "ERR: Could not query nullable status of table '" << table << "' column '" <<
                                column << "'";
                        PQclear(nullableRes);
                        PQclear(res);
                        PQclear(pkRes);
                        return -1;
                }

                bool nullable = std::strcmp(PQgetvalue(nullableRes, 0, 0), "YES") == 0;
                PQclear(nullableRes);

                fields.emplace(column, table_field{
                                       .name = column,
                                       .type = type,
                                       .is_nullable = nullable,
                                       .is_primary = std::ranges::find(primary_keys, column) != primary_keys.end()
                               });
        }

        PQclear(pkRes);
        PQclear(res);

        // Emit the struct
        emit_struct(table, fields, type_mappings);

        // Emit mapper
        emit_dao_mapper(table, fields, type_mappings);

        // Emit insert
        emit_insert(table, fields, type_mappings);

        // Update function
        emit_update(table, fields, type_mappings);

        // Emit basic select functions
        if (primary_keys.empty())
                std::cout << "//\n// The table '" << table <<
                        "' is not selectable, for it has no primary keys.\n//\n"
                        << std::endl;
        else
                emit_select(table, fields, type_mappings);

        // Save function
        emit_save(table, fields, type_mappings);

        // Emit the utility spread macros
        emit_spread_macro(table, fields, type_mappings);

        return 0;
}

int generate_tables(PGconn *conn, const std::map<std::string, type_mapping> &type_mappings)
{
        const std::string query = "SELECT * FROM information_schema.tables WHERE table_schema = 'public'";
        PGresult *res = PQexec(conn, query.c_str());

        if (PQresultStatus(res) != PGRES_TUPLES_OK) {
                std::cerr << "ERR: Could not query tables." << std::endl;
                PQclear(res);
                return -1;
        }

        int nTuples = PQntuples(res);
        for (int i = 0; i < nTuples; i++) {
                std::string table = PQgetvalue(res, i, 2);

                if (table == "db_migration")
                        continue;

                if (serialise_table(conn, table, type_mappings) < 0)
                        return -1;
        }
        PQclear(res);
        return 0;
}

int generate_custom_dao_function(PGconn *conn, const DaoFunction &function)
{
        std::string return_type = (function.type == SELECT_SINGLE)
                                          ? (function.type_mapping + " *")
                                          : ("std::vector<" + function.type_mapping + "> &");

        std::cout << "[[nodiscard]] inline bool " << function.identifier << "(Database &db, " << return_type << "dst";

        int n;

        if (function.params.empty())
                goto body;

        std::cout << ", ";

        n = 0;
        function.params.for_each([&](const auto &iden, const auto &type) {
                std::cout << type << " " << iden;
                if (function.params.size() < (n++))
                        std::cout << ", ";
        });

body:
        std::cout << ")" << std::endl << "{" << std::endl;

        std::string rawQuery = function.query;

        // Sanitize quotes
        size_t pos = 0;
        while (pos < rawQuery.size()) {
                pos = rawQuery.find('\"', pos);
                if (pos == std::string::npos)
                        break;
                rawQuery.replace(pos, 1, "\\\"");
                pos += 2;
        }

        rawQuery = '\"' + rawQuery + '\"';

        std::string query;
        pos = 0;
        int nParam = 0;

        while (!rawQuery.empty()) {
                pos = rawQuery.find('?');
                if (pos == std::string::npos) {
                        query.append(rawQuery);
                        break;
                }
                query.append(rawQuery.substr(0, pos));
                rawQuery.erase(0, pos + 1);
                if (nParam >= function.params.size()) {
                        CROW_LOG_CRITICAL << "Ran out of parameters when parsing query of function '"
                        << function.identifier << "'.";
                        return -1;
                }
                auto param = function.params.pair_n(nParam++);
                bool quotes = needs_quotes(param.second);

                if (quotes)
                        query.append("'");

                query.append("\" + " + param.first);

                if (pos < rawQuery.size() || quotes)
                        query.append(" + \"");

                if (quotes)
                        query.append("'");
        }

        std::cout << "\tstd::string query = " << query << ";" << std::endl;
        std::cout << "\tPGresult *res = dao_query(db, query, PGRES_TUPLES_OK);" << std::endl;
        std::cout << "\tif (!res) return false;" << std::endl;

        if (function.type == SELECT_SINGLE) {
                std::cout << "\tif (PQntuples(res) != 1) {\n\t\tPQclear(res);\n\t\treturn false;\n\t}" << std::endl;
                std::cout << "\t*dst = dao_map_" << function.type_mapping << "(res, 0);" << std::endl;
        } else {
                std::cout << "\tdao_map_all<" << function.type_mapping <<
                        ">(res, dst, [](auto *res, auto tuple) { return dao_map_" << function.type_mapping <<
                        "(res, tuple); });" << std::endl;
        }

        std::cout << "\tPQclear(res);" << std::endl;
        std::cout << "\treturn true;" << std::endl;
        std::cout << "}\n\n";
        return 0;
}

int generate_custom_dao_functions(PGconn *conn, const DaoConfig &config)
{
        std::cout << "//\n// Custom Functions\n//\n\n";

        for (const auto &function: config.functions)
                if (generate_custom_dao_function(conn, function) < 0)
                        return -1;

        return 0;
}

void gen_preamble()
{
        std::cout << "/**\n"
                " * This file was automatically generated by the Malphas DAO generator through database introspection\n"
                " */\n"
                "\n"
                "#pragma once\n"
                "\n"
                "#include <string>\n"
                "#include <any>\n"
                "#include <libpq-fe.h>\n"
                "#include <Database.hpp>\n\n"
                "#define NO_CAST(x) (x)\n"
                "#define IFNNULL(n, _else) (PQgetisnull(result, tuple, n) ? std::nullopt : std::optional(_else))\n"
                "[[nodiscard]] inline bool cast_bool(std::string &&str) { return (str == \"true\"); }\n\n"
                "template<typename T> [[nodiscard]] inline typename std::enable_if<std::is_arithmetic<T>::value, std::string>::type xto_string(T arg) { return std::to_string(arg); }\n"
                "template<typename T> [[nodiscard]] inline typename std::enable_if<std::is_same<T, std::string>::value, std::string>::type xto_string(T arg) { return arg; }\n\n"
                "inline bool finalize_op(PGresult *res) {\n"
                "        if (!res)\n"
                "                return false;\n"
                "        PQclear(res);\n"
                "        return true;\n"
                "}\n\n"
                "inline PGresult *dao_query(Database &db, std::string query, ExecStatusType assert_status)\n"
                "{\n"
                "        ExecStatusType status;\n"
                "        PGresult *res = db.query(query, &status);\n"
                "        if (status != assert_status) {\n"
                "                PQclear(res);\n"
                "                return NULL;\n"
                "        }\n"
                "        return res;\n"
                "}\n\n"
                "template<typename T>\n"
                "inline void dao_map_all(PGresult *res, std::vector<T> &dst, std::function<T(PGresult *res, int tuple)> mapper)\n"
                "{\n"
                "       for (int i = 0; i < PQntuples(res); i++)\n"
                "               dst.push_back(mapper(res, i));\n"
                "}\n\n// BEGIN GENERATED CODE\n\n";
}

int main()
{
        DbConfig cfg;
        if (!parse_db_config(&cfg))
                return 1;

        PGconn *conn = connect_db(cfg.user, cfg.password, cfg.db, cfg.host, cfg.port);
        if (!conn)
                return 1;

        DaoConfig dao_cfg;
        if (!parse_dao_config(&dao_cfg)) {
                PQfinish(conn);
                return 1;
        }

        gen_preamble();

        std::map<std::string, type_mapping> type_map;

        auto map_type = [&type_map](const std::string &sql_type, const std::string &cpp_type,
                                    const std::string &conv_fun) {
                type_map[sql_type] = type_mapping{
                        .cpp = cpp_type,
                        .function = conv_fun
                };
        };

        map_type("text", "std::string", "std::string");
        map_type("integer", "int", "std::stoi");
        map_type("uuid", "std::string", "std::string");
        map_type("boolean", "bool", "cast_bool");
        map_type("float", "float", "std::stof");

        int status = 0;

        if (generate_tables(conn, type_map) < 0)
                status = -1;

        if (generate_custom_dao_functions(conn, dao_cfg) < 0)
                status = -1;

        std::cout << "// END GENERATED CODE" << std::endl;

        PQfinish(conn);

        return status;
}
