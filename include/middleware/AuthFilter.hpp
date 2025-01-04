#pragma once

#include <crow_all.h>
#include <regex>
#include <Database.hpp>

struct AuthFilter {
        struct context {
                std::string user_id;
        };

        explicit AuthFilter(const std::vector<std::string> &public_routes, Database &db);

        void before_handle(crow::request &req, crow::response &res, context &ctx);

        void after_handle(crow::request &req, crow::response &res, context &ctx);

        private:
                std::vector<std::regex> public_routes;
                Database &db;

                bool isSecured(const std::string &route);
};
