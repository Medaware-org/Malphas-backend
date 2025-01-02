#define CROW_ENABLE_SSL
#include <cfg/Config.hpp>
#include <Database.hpp>
#include <Api.hpp>
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

        MalphasConfig malphasCfg;

        if (!parse_malphas_config(&malphasCfg))
                return 1;

        DbConfig cfg;

        if (!parse_db_config(&cfg))
                return 1;

        auto db = Database();

        if (!db.connect(cfg))
                return 1;

        if (!db.init_migrations())
                return 1;

        if (!db.run_migrations())
                return 1;

        crow::App<AuthFilter> app(
                AuthFilter({"^/", "^/login", "^/register"}, db)
        );

        app.loglevel(crow::LogLevel::Debug);

        const MalphasApi api(db);
        api.register_endpoints(app);

        app.port(malphasCfg.port).run();
        return 0;
}
