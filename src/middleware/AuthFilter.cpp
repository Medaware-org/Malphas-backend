#include <middleware/AuthFilter.hpp>

AuthFilter::AuthFilter(const std::vector<std::string> &public_routes)
{
        for (const auto &route: public_routes)
                this->public_routes.emplace_back(route);
}

void AuthFilter::before_handle(crow::request &req, crow::response &res, context &ctx)
{
        const std::string &route = req.url;

        if (isSecured(route)) {
                res.code = 401;
                res.body = "Unauthorized";
                res.end();
                return;
        }
}

void AuthFilter::after_handle(crow::request &req, crow::response &res, context &ctx)
{
}

bool AuthFilter::isSecured(const std::string &route)
{
        for (const auto &pattern : this->public_routes)
                if (std::regex_match(route, pattern))
                        return false;

        return true;
}
