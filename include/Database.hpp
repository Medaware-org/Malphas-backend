#pragma once

#include <libpq-fe.h>

#include <string>

/**
 * A high-level wrapper for performing database operations
 */
class Database {
    private:
        PGconn *conn;
        std::string db_user;
        std::string db_password;
        std::string db_db;
        std::string db_conn_str;
    public:
        Database(std::string user, std::string password, std::string db);
        ~Database();
    public:
        bool connect();
};