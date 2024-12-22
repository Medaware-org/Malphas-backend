#include <cfg/Config.hpp>
#include <Database.hpp>
#include <crow_all.h>
#include <iostream>

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

        std::string db_user;
        std::string db_password;
        std::string db_db;
        bool error = false;

        Config cfg = Config("config.cfg");
        if (!cfg.parse([&](cfg_prop &prop) {
                std::string *dst;

                if (error)
                        return;

                if (prop.key == "user")
                        dst = &db_user;
                else if (prop.key == "password")
                        dst = &db_password;
                else if (prop.key == "db")
                        dst = &db_db;
                else {
                        CROW_LOG_CRITICAL << "Unknown configuration property: " << prop.key;
                        error = true;
                        return;
                }

                *dst = prop.value;
        })) { return 1; }

        if (error)
                return 1;

        Database db = Database(db_user, db_password, db_db);
        if (!db.connect())
                return 1;

        crow::SimpleApp app;
        app.loglevel(crow::LogLevel::Info);

        CROW_ROUTE(app, "/")([]() {
                return "Hello world";
        });

        app.port(1234).run();
}
