#include <Database.h>
#include <utility>
#include <crow_all.h>

Database::Database(std::string user, std::string password, std::string db) : db_user(std::move(user)),
                                                                             db_password(std::move(password)),
                                                                             db_db(std::move(db)),
                                                                             conn(nullptr)
{
        this->db_conn_str = "user=" + db_user + " password=" + db_password + " dbname=" + db_db + " port=5432 host=localhost";
        CROW_LOG_INFO << "PostgreSQL connection string: \"" << this->db_conn_str << "\"";
}

Database::~Database()
{
        free(this->conn);
}

bool Database::connect()
{
        this->conn = PQconnectdb(this->db_conn_str.c_str());
        auto status = PQstatus(this->conn);

        if (status != CONNECTION_OK) {
                CROW_LOG_CRITICAL << "Could not establish database connection.";
                return false;
        }

        CROW_LOG_INFO << "Database connection established.";

        return true;
}