#include <middleware/AuthFilter.hpp>
#include <dao/dao.h>

AuthFilter::AuthFilter(const std::vector<std::string> &public_routes, Database &db) : db(db)
{
        for (const auto &route: public_routes)
                this->public_routes.emplace_back(route);
}

void AuthFilter::before_handle(crow::request &req, crow::response &res, context &ctx)
{
        if (!isSecured(req.url))
                return;

        // Only retrieve user ID; Authorization occurs in the proxy
        const auto id = req.headers.find("X-User-ID");
        const auto username = req.headers.find("X-User-Name");
        user usr;
        std::string idStr, usernameStr;

        if (id == req.headers.end() || username == req.headers.end()) {
                CROW_LOG_INFO << "A secured endpoint received a faulty request which could not be acted upon.";
                goto unauthorized;
        }

        idStr = id->second;
        usernameStr = username->second;

        if (!get_one_user(db, &usr, idStr)) {
                if (!user_insert(db, idStr, usernameStr)) {
                        CROW_LOG_CRITICAL << "Could not register new user: " << idStr;
                        goto iserr;
                }

                CROW_LOG_INFO << "Registered " << usernameStr << " (ID " << idStr << "). Good morning.";
        }

        ctx.user_id = id->second;

        return;

unauthorized:
        res.code = 401;
        res.body = "Unauthorized";
        res.end();
        return;

iserr:
        res.code = 500;
        res.body = "Internal Server Error.";
        res.end();
}

void AuthFilter::after_handle(crow::request &req, crow::response &res, context &ctx)
{
}

bool AuthFilter::isSecured(const std::string &route)
{
        for (const auto &pattern: this->public_routes)
                if (std::regex_match(route, pattern))
                        return false;

        return true;
}
