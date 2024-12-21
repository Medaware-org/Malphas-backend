#include <Database.h>
#include <utility>
#include <crow_all.h>

Database::Database(std::string user, std::string password, std::string db) : db_user(std::move(user)),
                                                                             db_password(std::move(password)),
                                                                             db_db(std::move(db)),
                                                                             conn(nullptr)
{
        this->db_conn_str = "user=" + user + " password=" + password + " dbname=" + db;
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

        return true;
}