#pragma once

#include <libpq-fe.h>

#include <string>

/**
 * A high-level wrapper for performing database operations
 */
class Database {
    private:
        PGconn *conn;
    public:
        Database();
        ~Database();
    public:
        bool connect(const std::string &user, const std::string &password, const std::string &db, const std::string &host, int port);
};