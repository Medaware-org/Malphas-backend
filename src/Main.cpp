#include <cfg/Config.hpp>
#include <Database.hpp>
#include <crow_all.h>
#include <iostream>
#include <optional>

const char *banner = "  __  __       _       _               \n"
                     " |  \\/  |     | |     | |              \n"
                     " | \\  / | __ _| |_ __ | |__   __ _ ___ \n"
                     " | |\\/| |/ _` | | '_ \\| '_ \\ / _` / __|\n"
                     " | |  | | (_| | | |_) | | | | (_| \\__ \\\n"
                     " |_|  |_|\\__,_|_| .__/|_| |_|\\__,_|___/\n"
                     "                | |                    \n"
                     "                |_|                    \n";

int main()
{
        std::cout << banner << std::endl;

        std::optional<std::string> db_user;
        std::optional<std::string> db_password;
        std::optional<std::string> db_db;
        bool error = false;

        Config cfg = Config("config.cfg");
        if (!cfg.parse([&](cfg_prop &prop) {
                std::optional<std::string> *dst;

                if (error)
                        return;

                if (prop == "database.user")
                        dst = &db_user;
                else if (prop == "database.password")
                        dst = &db_password;
                else if (prop == "database.db")
                        dst = &db_db;
                else {
                        CROW_LOG_CRITICAL << "Unknown configuration property: " << prop.section << "." << prop.key;
                        error = true;
                        return;
                }

                *dst = prop.value;
        })) { return 1; }

        if (error)
                return 1;

        // TODO Implement a better cfg validation system with less redundancies
#define HANDLE_EMPTY(opt, path) if (opt->empty()) { CROW_LOG_CRITICAL << "Required property not set: " << path; return 1; }
        HANDLE_EMPTY(db_user, "database.user")
        HANDLE_EMPTY(db_password, "database.password")
        HANDLE_EMPTY(db_db, "database.db")
#undef HANDLE_EMPTY

        Database db = Database(db_user.value(), db_password.value(), db_db.value());
        if (!db.connect())
                return 1;

        crow::SimpleApp app;
        app.loglevel(crow::LogLevel::Info);

        CROW_ROUTE(app, "/")([]() {
                return "Hello world";
        });

        app.port(1234).run();
}
