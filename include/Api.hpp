#pragma once

#include <crow_all.h>
#include <Database.hpp>

#define RETURN_ERR return crow::response(400, "Could not parse JSON.");

#define JSON_BODY(id)                           \
        auto id = crow::json::load(req.body);   \
        if (!id) { return crow::response(400, "Could not parse JSON."); }

#define REQUIRE(body, id, path) if (!body.has(path)) { return crow::response(400, "Invalid JSON body"); } auto id = body[path]

#define SESSION_TOKEN_LENGTH 128

class MalphasApi {
        Database &db;

        public:
                explicit MalphasApi(Database &db);

                template<typename... T>
                void register_endpoints(crow::App<T...> &crow) const;

        private:
                [[nodiscard]] std::string generate_token() const;

                [[nodiscard]] crow::json::wvalue error_dto(std::string brief, std::string detail) const;

        private:
                [[nodiscard]] crow::response login(const crow::json::rvalue &body) const;

                [[nodiscard]] crow::response user_register(const crow::json::rvalue &body) const;

                [[nodiscard]] crow::response user_get_scenes(const crow::request &req) const;
};
