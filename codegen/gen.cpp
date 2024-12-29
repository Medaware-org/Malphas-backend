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

struct type_mapping {
        std::string cpp;        // The C++ type
        std::string function;   // THe conversion function
};

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

void emit_struct(const std::string &table, const std::map<std::string, std::string> &layout, const std::map<std::string, type_mapping> &type_mappings)
{
        std::cout << "struct " << table << " {" << std::endl;
        for (const auto &[column, type]: layout)
                std::cout << "\t" << map_types(type, type_mappings).cpp << " " << column << ";" << std::endl;
        std::cout << "};\n\n";
}

void emit_spread_macro(const std::string &table, const std::map<std::string, std::string> &layout, const std::map<std::string, type_mapping> &type_mappings)
{
        std::string upper_table = std::string(table);
        std::transform(table.begin(), table.end(), upper_table.begin(), ::toupper);

        size_t nFields = layout.size();

        std::cout << "#define SPREAD_" << upper_table << "(" << table << "_struct" << ") ";
        int index = 0;
        for (const auto &[column, type]: layout) {
                std::cout << "_struct." << column;
                if ((index ++) + 1 < nFields)
                        std::cout << ", ";
        }
        std::cout << std::endl;

        index = 0;
        std::cout << "#define SPREAD_" << upper_table << "_PTR(" << table << "_struct" << ") ";
        for (const auto &[column, type]: layout) {
                std::cout << "_struct->" << column;
                if ((index ++) + 1 < nFields)
                        std::cout << ", ";
        }

        std::cout << "\n\n";
}


/*
 * Generate an insert function for a given table
 */
void emit_insert(const std::string &table, const std::map<std::string, std::string> &layout, const std::map<std::string, type_mapping> &type_mappings)
{
        size_t nTuples = layout.size();

        std::cout << "bool " << table << "_insert(Database &db, ";
        int index = 0;
        for (const auto &[column, type]: layout) {
                std::cout << map_types(type, type_mappings).cpp << " " << column;
                std::cout << (((index++) + 1 < layout.size()) ? ", " : ") {\n");
        }

        std::cout << "\tstd::string query = \"INSERT INTO " << table << " (";
        index = 0;
        for (const auto &[column, type]: layout) {
                std::cout << column;
                std::cout << (((index++) + 1 < nTuples) ? ", " : ") VALUES (");
        }

        index = 0;
        for (const auto &[column, type]: layout) {
                bool quotes = false;

                if (type == "text" || type.find("varchar") != std::string::npos)
                        quotes = true;

                if (quotes)
                        std::cout << "\\\"";

                std::cout << "\" + " << column << " + \"";

                if (quotes)
                        std::cout << "\\\"";

                std::cout << (((index++) + 1 < nTuples) ? ", " : ")\";\n");
        }

        std::cout << "\treturn finalize_insert_op(dao_query(db, query, PGRES_COMMAND_OK));" << std::endl;
        std::cout << "}\n\n";
}

/*
 * Generate the mapper for a given relation
 */
void emit_dao_mapper(const std::string &table, const std::map<std::string, std::string> &layout, const std::map<std::string, type_mapping> &type_mappings)
{
        std::cout << "static " << table << " dao_map_" << table << "(PGresult *result, int tuple) {" << std::endl
                  << "\treturn " << table << " {\n";

        int index = 0;
        for (const auto &[column, type]: layout) {
                type_mapping mapping = map_types(type, type_mappings);
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

        int nTuples = PQntuples(res);

        std::map<std::string, std::string> fields;
        for (int i = 0; i < nTuples; i++) {
                std::string column = PQgetvalue(res, i, 3);
                std::string type = PQgetvalue(res, i, 7);
                fields[column] = type;
        }

        PQclear(res);

        // Emit the struct
        emit_struct(table, fields, type_mappings);

        // Mapper
        emit_dao_mapper(table, fields, type_mappings);

        // Emit insert function
        emit_insert(table, fields, type_mappings);

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

        auto map_type = [&type_map](const std::string &sql_type, const std::string &cpp_type, const std::string &conv_fun) {
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