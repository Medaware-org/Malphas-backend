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

        std::string token;
        session s;
        const std::string bearer = "Bearer ";

        auto auth = req.headers.find("Authorization");
        if (auth == req.headers.end())
                goto unauthorized;

        token = auth->second;

        if (token.starts_with(bearer))
                token.erase(0, bearer.length());

        if (!get_one_session(db, &s, token))
                goto unauthorized;

        ctx.user_id = s.user_id;
        ctx.token = token;

        return;

unauthorized:
        res.code = 401;
        res.body = "Unauthorized";
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
