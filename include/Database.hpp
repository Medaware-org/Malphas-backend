#pragma once

#include <libpq-fe.h>

#include <string>
#include <cfg/Config.hpp>

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

        /**
         * Same as the alternative connect function.
         * @code
         * bool connect(const std::string &user, const std::string &password, const std::string &db, const std::string &host, int port);
         * @endcode
         */
        [[nodiscard]] bool connect(DbConfig &cfg);

        /**
         * Initialize the migration system by executing `migrations/init.sql` if needed
         */
        [[nodiscard]] bool init_migrations();

        /**
         * Execute a SQL script stored in a file
         */
        [[nodiscard]] bool execute_script(const char *path);

        /**
         * Run new database migrations
         */
        [[nodiscard]] bool run_migrations();

        /**
         * Run a raw query on the database
         */
        [[nodiscard]] PGresult *query(std::string &query, ExecStatusType *status);

    private:

        /**
         * Used internally by the migration system to increment the number of the current migration
         */
        [[nodiscard]] bool add_migration_entry(int number);
};