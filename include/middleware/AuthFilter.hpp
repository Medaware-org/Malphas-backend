#pragma once

#include <crow_all.h>

struct AuthFilter {
        struct context {
        };

        void before_handle(crow::request &req, crow::response &res, context &ctx);

        void after_handle(crow::request &req, crow::response &res, context &ctx);
};
