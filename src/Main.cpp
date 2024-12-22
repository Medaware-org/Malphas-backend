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

        Config cfg = Config("config.cfg");

        if (!cfg.parse())
                return 1;

        std::string db_user;
        std::string db_password;
        std::string db_db;
        std::string db_host;
        int db_port;

#define ASSIGN(dst) [&](const std::string &prop) { dst = prop; return true; }

        cfg.add_required("database.user", ASSIGN(db_user));
        cfg.add_required("database.password", ASSIGN(db_password));
        cfg.add_required("database.db", ASSIGN(db_db));
        cfg.add_required("database.host", ASSIGN(db_host));
        cfg.add_required("database.port", [&](const std::string &prop) {
                try {
                        size_t pos;
                        db_port = std::stoi(prop, &pos);
                        return pos == prop.size();
                } catch (...) {
                        return false;
                }
        });

#undef ASSIGN

        if (!cfg.validate())
                return 1;

        Database db = Database(db_user, db_password, db_db, db_host, db_port);
        if (!db.connect())
                return 1;

        crow::SimpleApp app;
        app.loglevel(crow::LogLevel::Info);

        CROW_ROUTE(app, "/")([]() {
                return "Hello world";
        });

        app.port(1234).run();
}
