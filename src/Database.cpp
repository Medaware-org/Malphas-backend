#include <Database.hpp>
#include <utility>
#include <crow_all.h>

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