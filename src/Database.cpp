#include <Database.hpp>
#include <utility>
#include <crow_all.h>
#include <util.hpp>

Database::Database() : conn(nullptr)
{
}

Database::~Database()
{
        free(this->conn);
}

bool Database::connect(const std::string &user, const std::string &password, const std::string &db, const std::string &host, int port)
{
        std::string conn_str =
                "user=" + user + " password=" + password + " dbname=" + db + " port=" + std::to_string(port) +
                " host=" + host;

        CROW_LOG_INFO << "PostgreSQL connection string: \"" << conn_str << "\"";

        this->conn = PQconnectdb(conn_str.c_str());
        auto status = PQstatus(this->conn);

        if (status != CONNECTION_OK) {
                CROW_LOG_CRITICAL << "Could not establish database connection.";
                return false;
        }

        CROW_LOG_INFO << "Database connection established.";

        return true;
}

bool Database::init_migrations()
{
        // Check if the migrations table exists
        PGresult *res = PQexec(this->conn, "SELECT to_regclass('public.db_migration') IS NOT NULL;");

        if (PQresultStatus(res) != PGRES_TUPLES_OK) {
                CROW_LOG_CRITICAL << "Could not query database for migrations table.";
                PQclear(res);
                return false;
        }

        // The migrations table exists; no need to run the init script.
        char *value = PQgetvalue(res, 0, 0);

        if (strcmp(value, "t") == 0) {
                CROW_LOG_INFO << "Migrations table present. Init script will not run.";\
                PQclear(res);
                return true;
        }

        PQclear(res);

        return execute_script("migrations/init.sql");
}

[[nodiscard]] bool Database::execute_script(const char *path)
{
        unsigned int buff_len;
        char *script = read_file(path, &buff_len);

        if (!script) {
                CROW_LOG_CRITICAL << "Could not load script from file: " << path;
                return false;
        }

        PGresult *res = PQexec(this->conn, script);
        ExecStatusType status = PQresultStatus(res);
        free(script);

        bool ok = true;

        if (status == PGRES_FATAL_ERROR) {
                CROW_LOG_CRITICAL << "Encountered error when executing script '" << path << "'";
                CROW_LOG_CRITICAL << "Error: " << PQresultErrorMessage(res);
                ok = false;
        } else CROW_LOG_INFO << "Executed script: " << path;

        PQclear(res);

        return ok;
}