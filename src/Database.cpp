#include <Database.hpp>
#include <utility>
#include <crow_all.h>
#include <util.hpp>
#include <filesystem>

Database::Database() : conn(nullptr)
{
}

Database::~Database()
{
        free(this->conn);
}

bool Database::connect(DbConfig &cfg)
{
        return connect(cfg.user, cfg.password, cfg.db, cfg.host, cfg.port);
}

bool Database::connect(const std::string &user, const std::string &password, const std::string &db,
                       const std::string &host, int port)
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
                CROW_LOG_CRITICAL << PQresultErrorMessage(res);
                ok = false;
        } else
                CROW_LOG_INFO << "Executed script: " << path;

        PQclear(res);

        return ok;
}

bool Database::run_migrations()
{
        // Get the number of migration that we're currently at
        PGresult *res = PQexec(this->conn, "SELECT MAX(number) FROM db_migration;");

        if (PQresultStatus(res) != PGRES_TUPLES_OK) {
                CROW_LOG_CRITICAL << "Could not retrieve current migration number.";
                PQclear(res);
                return false;
        }

        const char *mgr = PQgetvalue(res, 0, 0);
        int mgr_num = 1;

        if (mgr[0] != '\0')
                mgr_num = std::stoi(mgr) + 1;

        PQclear(res);

        bool migrated = false;

        std::map<int, std::filesystem::path> files;

        for (auto &entry: std::filesystem::directory_iterator("migrations")) {
                if (!entry.is_regular_file())
                        continue;

                const auto &path = entry.path();
                std::string path_str = path.string();

                // The init file is not a migration - let's ignore it.
                if (path.filename().string() == "init.sql")
                        continue;

                if (!path_str.ends_with(".sql"))
                        continue;

                files.emplace(std::stoi(entry.path().filename().string()), path);
        }

        for (int i = mgr_num; i <= files.size(); i++) {
                auto file = files.find(i);
                if (file == files.end())
                        break;

                std::string filename = file->second.filename().string();
                std::string path = file->second.string();

                if (!execute_script(path.c_str())) {
                        CROW_LOG_CRITICAL << "Database migration failed on migration #" << i;
                        return false;
                }

                CROW_LOG_INFO << "Ran migration script #" << i << ": " << filename;
                migrated = true;

                if (!add_migration_entry(i)) {
                        CROW_LOG_CRITICAL << "Migrations failed due to an error.";
                        return false;
                }
        }

        if (!migrated) {
                CROW_LOG_INFO << "No migration scripts were invoked.";
        } else
                CROW_LOG_INFO << "Done running migrations.";

        return true;
}

[[nodiscard]] PGresult *Database::query(std::string &query, ExecStatusType *status)
{
        PGresult *res = PQexec(this->conn, query.c_str());
        (*status) = PQresultStatus(res);
        return res;
}

bool Database::add_migration_entry(int number)
{
        std::string query = "INSERT INTO db_migration VALUES (" + std::to_string(number) + ")";
        PGresult *res = PQexec(this->conn, query.c_str());

        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
                CROW_LOG_CRITICAL << "Could not add migration entry for migration #" << number;
                return false;
        }

        return true;
}
