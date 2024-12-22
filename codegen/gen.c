#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libpq-fe.h>

PGconn *connect_db(const char *user, const char *password, const char *db, const char *host, int port)
{
        const char *conn_str_fmt = "user=%s password=%s dbname=%s port=%d host=%s";
        unsigned int fmt_len = strlen(conn_str_fmt);
        unsigned int param_len = strlen(user) + strlen(password) + strlen(db) + strlen(host) + 10;
        char *buffer = calloc(fmt_len + param_len, sizeof(char));
        sprintf(buffer, conn_str_fmt, user, password, db, port, host);

        PGconn *conn = PQconnectdb(buffer);
        free(buffer);

        if (PQstatus(conn) != CONNECTION_OK) {
                printf("ERR: Could not connect to the database.\n");
                PQfinish(conn);
                return NULL;
        }

        return conn;
}

#define MAP(src, dst) if (strcmp(org, src) == 0) return dst;

const char *map_types(char *org)
{
        MAP("text", "std::string")
        MAP("uuid", "std::string")

        return "std::any";
}

#undef MAP

void serialise_table(PGconn *conn, char *table)
{
        printf("struct %s {\n", table);

        const char *query_fmt = "select * from information_schema.columns where table_name = '%s'";
        char *query = calloc(strlen(query_fmt) + strlen(table) + 10, sizeof(char));
        sprintf(query, query_fmt, table);
        PGresult *res = PQexec(conn, query);
        free(query);

        if (PQresultStatus(res) != PGRES_TUPLES_OK) {
                printf("ERR: Could not query fields of table '%s'\n", table);
                PQclear(res);
                return;
        }

        int nTuples = PQntuples(res);
        for (int i = 0; i < nTuples; i++) {
                char *column = PQgetvalue(res, i, 3);
                char *type = PQgetvalue(res, i, 7);
                printf("\t%s %s;\n", map_types(type), column);
        }

        PQclear(res);

        printf("};\n\n");
}

void generate_tables(PGconn *conn)
{
        const char *query = "select * from information_schema.tables where table_schema = 'public'";
        PGresult *res = PQexec(conn, query);
        if (PQresultStatus(res) != PGRES_TUPLES_OK) {
                printf("ERR: Could not query tables.\n");
                PQclear(res);
                return;
        }
        int nTuples = PQntuples(res);
        for (int i = 0; i < nTuples; i++) {
                char *table_name = PQgetvalue(res, i, 2);

                if (strcmp(table_name, "db_migration") == 0)
                        continue;

                serialise_table(conn, table_name);
        }
        PQclear(res);
}

int main(void)
{
        PGconn *conn = connect_db("malphas", "root", "malphas", "localhost", 5432);
        if (!conn)
                return 1;

        printf("/**\n"
               " * This file was created by the Malphas DAO generator\n"
               " */\n\n");
        printf("#pragma once\n\n");
        printf("#include <string>\n");
        printf("#include <any>\n\n");

        generate_tables(conn);
        PQfinish(conn);
        return 0;
}