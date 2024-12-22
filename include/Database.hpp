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
        /**
         * Connect to the database.
         * @param user Database username
         * @param password Password of the database user
         * @param db Name of the database
         * @param host The host on which the database is running
         * @param port Port of the database server
         * @returns <code>true</code> if connecting succeeded, or <code>false</code> in case it failed.
         */
        [[nodiscard]] bool connect(const std::string &user, const std::string &password, const std::string &db, const std::string &host, int port);
};