#include <middleware/AuthFilter.hpp>

void AuthFilter::before_handle(crow::request &req, crow::response &res, context &ctx)
{
        CROW_LOG_INFO << "Middleware ran!";
}

void AuthFilter::after_handle(crow::request &req, crow::response &res, context &ctx)
{
}
