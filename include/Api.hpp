#pragma once

#include <crow_all.h>
#include <Database.hpp>

#define RETURN_ERR return crow::response(400, "Could not parse JSON.");

#define JSON_BODY(id)                           \
        auto id = crow::json::load(req.body);   \
        if (!id) { RETURN_ERR }

#define REQUIRE(body, id, path) if (!body.has(path)) { RETURN_ERR } auto id = body[path]

class MalphasApi {
        Database &db;

        public:
                explicit MalphasApi(Database &db);

                template<typename... T>
                void register_endpoints(crow::App<T...> &crow) const;

        private:
                [[nodiscard]] crow::response login(const crow::json::rvalue &body) const;
};
