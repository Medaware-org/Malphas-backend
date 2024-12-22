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

        Config cfg = Config("config.cfg");

        if (!cfg.parse())
                return 1;

        cfg.add_required("database.user");
        cfg.add_required("database.password");
        cfg.add_required("database.db");

        if (!cfg.validate())
                return 1;

        Database db = Database(cfg["database.user"], cfg["database.password"], cfg["database.db"]);
        if (!db.connect())
                return 1;

        crow::SimpleApp app;
        app.loglevel(crow::LogLevel::Info);

        CROW_ROUTE(app, "/")([]() {
                return "Hello world";
        });

        app.port(1234).run();
}
