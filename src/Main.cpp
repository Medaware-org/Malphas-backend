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
        crow::SimpleApp app;

        std::cout << banner << std::endl;

        CROW_ROUTE(app, "/")([]() {
                return "Hello world";
        });

        app.port(1234).run();
}