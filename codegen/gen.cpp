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
#include <tuple>

struct type_mapping {
        std::string cpp; // The C++ type
        std::string function; // THe conversion function
};

struct table_field {
        std::string name;
        std::string type;
        bool is_primary;
};

bool needs_quotes(const std::string &type)
{
        return type == "text" || type.find("varchar") != std::string::npos || type == "uuid";
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
type_mapping map_types(const std::string &org, const std::map<std::string, type_mapping> &type_mappings)
{
        auto iter = type_mappings.find(org);
        if (iter == type_mappings.end())
                return type_mapping{.cpp = "std::any", .function = "std::any"};
        return iter->second;
}

void emit_struct(const std::string &table, const std::map<std::string, table_field> &layout,
                 const std::map<std::string, type_mapping> &type_mappings)
{
        std::cout << "struct " << table << " {" << std::endl;
        for (const auto &[column, field]: layout)
                std::cout << "\t" << map_types(field.type, type_mappings).cpp << " " << column << ";" << std::endl;
        std::cout << "};\n\n";
}

void emit_spread_macro(const std::string &table, const std::map<std::string, table_field> &layout,
                       const std::map<std::string, type_mapping> &type_mappings)
{
        std::string upper_table = std::string(table);
        std::transform(table.begin(), table.end(), upper_table.begin(), ::toupper);

        size_t nFields = layout.size();

        std::cout << "#define SPREAD_" << upper_table << "(" << table << "_struct" << ") ";
        int index = 0;
        for (const auto &[column, field]: layout) {
                std::cout << table << "_struct." << column;
                if ((index++) + 1 < nFields)
                        std::cout << ", ";
        }
        std::cout << std::endl;

        index = 0;
        std::cout << "#define SPREAD_" << upper_table << "_PTR(" << table << "_struct" << ") ";
        for (const auto &[column, type]: layout) {
                std::cout << table << "_struct->" << column;
                if ((index++) + 1 < nFields)
                        std::cout << ", ";
        }

        std::cout << "\n\n";
}


/*
 * Generate an insert function for a given table
 */
void emit_insert(const std::string &table, const std::map<std::string, table_field> &layout,
                 const std::map<std::string, type_mapping> &type_mappings)
{
        size_t nTuples = layout.size();

        std::cout << "bool " << table << "_insert(Database &db, ";
        int index = 0;
        for (const auto &[column, field]: layout) {
                std::cout << map_types(field.type, type_mappings).cpp << ((field.is_primary) ? " /*PK*/ " : " ") <<
                        column;
                std::cout << (((index++) + 1 < layout.size()) ? ", " : ") {\n");
        }

        std::cout << "\tstd::string query = \"INSERT INTO \\\"" << table << "\\\" (";
        index = 0;
        for (const auto &[column, field]: layout) {
                std::cout << column;
                std::cout << (((index++) + 1 < nTuples) ? ", " : ") VALUES (");
        }

        index = 0;
        for (const auto &[column, field]: layout) {
                bool quotes = needs_quotes(field.type);

                if (quotes)
                        std::cout << "'";

                std::cout << "\" + " << column << " + \"";

                if (quotes)
                        std::cout << "'";

                std::cout << (((index++) + 1 < nTuples) ? ", " : ")\";\n");
        }

        std::cout << "\treturn finalize_insert_op(dao_query(db, query, PGRES_COMMAND_OK));" << std::endl;
        std::cout << "}\n\n";
}

void emit_select(const std::string &table, const std::map<std::string, table_field> &layout,
                 const std::map<std::string, type_mapping> &type_mappings)
{
        std::map<std::string, table_field> primary_keys;
        std::copy_if(layout.begin(), layout.end(),
                     std::inserter(primary_keys, primary_keys.end()),
                     [&](const std::pair<std::string, table_field> &p) {
                             return p.second.is_primary;
                     });

        int i = 0;
        std::string part_signature;
        for (auto &[column, field]: primary_keys) {
                part_signature += map_types(field.type, type_mappings).cpp + " " + column;
                part_signature += (((i++) + 1 < primary_keys.size()) ? ", " : ")\n{\n");
        }

        std::string query_str = "\tstd::string query = \"SELECT * FROM \\\"" + table + "\\\" WHERE ";

        i = 0;
        for (auto &[column, field]: primary_keys) {
                bool quote = needs_quotes(field.type);
                std::string quotes = (quote) ? "'" : "";
                query_str += column + " = " + quotes + "\" + " + column;
                query_str += (((i++) + 1 < primary_keys.size())
                                      ? ("\"" + quotes + " AND ")
                                      : ((quote ? "+ \"'\";\n" : ";\n")));
        }

        std::string exec_invoke = "\tPGresult *res = dao_query(db, query, PGRES_TUPLES_OK);\n"
                "\tif (!res) return false;";

        // Single result function
        std::cout << "bool get_one_" << table << "(Database &db, " << table << " *dst, " << part_signature;
        std::cout << query_str;
        std::cout << exec_invoke << std::endl;
        std::cout << "\t*dst = dao_map_" << table << "(res, 0);" << std::endl;
        std::cout << "\treturn true;" << std::endl;
        std::cout << "}\n\n";

        // Get all function
        std::cout << "bool get_all_" << table << "(Database &db, std::vector<" << table << "> &dst)\n{\n";
        std::cout << "\tstd::string query = \"SELECT * from \\\"" << table << "\\\"\";" << std::endl;
        std::cout << exec_invoke << std::endl;
        std::cout << "\tdao_map_all<" << table << ">(res, dst, [](auto *res, auto tuple) { return dao_map_" << table <<
                "(res, tuple); });" << std::endl;
        std::cout << "\treturn true;" << std::endl;
        std::cout << "}\n\n";
}

/*
 * Generate the mapper for a given relation
 */
void emit_dao_mapper(const std::string &table, const std::map<std::string, table_field> &layout,
                     const std::map<std::string, type_mapping> &type_mappings)
{
        std::cout << "static " << table << " dao_map_" << table << "(PGresult *result, int tuple) {" << std::endl
                << "\treturn " << table << " {\n";

        int index = 0;
        for (const auto &[column, field]: layout) {
                type_mapping mapping = map_types(field.type, type_mappings);
                std::cout << "\t\t." << column << " = " << mapping.function << "(PQgetvalue(result, tuple," << (index++)
                        << "))," << std::endl;
        }

        std::cout << "\t};\n}\n\n";
}

int serialise_table(PGconn *conn, std::string &table, const std::map<std::string, type_mapping> &type_mappings)
{
        std::string query = std::format("SELECT * FROM information_schema.columns WHERE table_name = '{}'", table);
        PGresult *res = PQexec(conn, query.c_str());

        if (PQresultStatus(res) != PGRES_TUPLES_OK) {
                std::cerr << "ERR: Could not query fields of table '" << table << "'" << std::endl;
                PQclear(res);
                return -1;
        }

        int nFields = PQntuples(res);

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

        std::map<std::string, table_field> fields;
        for (int i = 0; i < nFields; i++) {
                std::string column = PQgetvalue(res, i, 3);
                std::string type = PQgetvalue(res, i, 7);

                fields[column] = table_field{
                        .name = column,
                        .type = type,
                        .is_primary = std::ranges::find(primary_keys, column) != primary_keys.end()
                };
        }

        PQclear(pkRes);
        PQclear(res);

        // Emit the struct
        emit_struct(table, fields, type_mappings);

        // Mapper
        emit_dao_mapper(table, fields, type_mappings);

        // Emit insert function
        emit_insert(table, fields, type_mappings);

        // Emit retrieval function
        if (primary_keys.empty())
                std::cout << "//\n// The table '" << table <<
                        "' is not selectable, for it has no primary keys.\n//\n"
                        << std::endl;
        else
                emit_select(table, fields, type_mappings);

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

void gen_preamble()
{
        std::cout << "/**\n"
                " * This file was generated by the Malphas DAO generator\n"
                " */\n"
                "\n"
                "#pragma once\n"
                "\n"
                "#include <string>\n"
                "#include <any>\n"
                "#include <libpq-fe.h>\n"
                "#include <Database.hpp>\n\n"
                "#define PASS(x) (x)\n\n"
                "bool finalize_insert_op(PGresult *res) {\n"
                "        if (!res)\n"
                "                return false;\n"
                "        PQclear(res);\n"
                "        return true;\n"
                "}\n\n"
                "PGresult *dao_query(Database &db, std::string query, ExecStatusType assert_status)\n"
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
                "void dao_map_all(PGresult *res, std::vector<T> &dst, std::function<T(PGresult *res, int tuple)> mapper)\n"
                "{\n"
                "       for (int i = 0; i < PQntuples(res); i++)\n"
                "               dst.push_back(mapper(res, i));\n"
                "}\n\n";
}

int main()
{
        db_config cfg;
        if (!parse_db_config(&cfg))
                return 1;

        PGconn *conn = connect_db(cfg.user, cfg.password, cfg.db, cfg.host, cfg.port);
        if (!conn)
                return 1;

        gen_preamble();

        std::map<std::string, type_mapping> type_map;

        auto map_type = [&type_map](const std::string &sql_type, const std::string &cpp_type,
                                    const std::string &conv_fun) {
                type_map[sql_type] = type_mapping{
                        .cpp = cpp_type,
                        .function = conv_fun
                };
        };

        map_type("text", "std::string", "PASS");
        map_type("int", "int", "std::stoi");
        map_type("uuid", "std::string", "PASS");

        int status = 0;

        if (generate_tables(conn, type_map) < 0)
                status = -1;

        PQfinish(conn);

        return status;
}
