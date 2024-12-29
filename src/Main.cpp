#include <cfg/Config.hpp>
#include <Database.hpp>
#include <crow_all.h>
#include <iostream>
#include <middleware/AuthFilter.hpp>

auto banner = "  __  __       _       _               \n"
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

        db_config cfg;

        if (!parse_db_config(&cfg))
                return 1;

        auto db = Database();

        if (!db.connect(cfg))
                return 1;

        if (!db.init_migrations())
                return 1;

        if (!db.run_migrations())
                return 1;

        crow::App<AuthFilter> app;

        app.loglevel(crow::LogLevel::Info);

        CROW_ROUTE(app, "/")([]() {
                return "Hello world";
        });

        app.port(1234).run();
        return 0;
}