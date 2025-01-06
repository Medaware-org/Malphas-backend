#pragma once

#include <crow_all.h>
#include <Database.hpp>
#include <dao/dao.h>
#include <middleware/AuthFilter.hpp>

#define RETURN_ERR return crow::response(400, "Could not parse JSON.");

#define JSON_BODY(id)                           \
        auto id = crow::json::load(req.body);   \
        if (!id) { return crow::response(400, "Could not parse JSON."); }

#define AUTH_CONTEXT(id) AuthFilter::context &id = crow.template get_context<AuthFilter>(req);

#define QUERY_PARAM(id, param) char *id = req.url_params.get(param); if (!id) { return crow::response(400, "Could not parse query params."); }

#define OPT_PARAM(id, param) char *id = req.url_params.get(param);

#define REQUIRE(body, id, path) if (!body.has(path)) { return crow::response(400, "Invalid JSON body"); } const auto &id = body[path]

#define OPTIONAL(body, id, path) const auto& id = (body.has(path) ? body[path] : crow::json::rvalue());

#define SESSION_TOKEN_LENGTH 128

class MalphasApi {
        Database &db;

        public:
                explicit MalphasApi(Database &db);

                template<typename... T>
                void register_endpoints(crow::App<T...> &crow) const;

        private:
                [[nodiscard]] static std::string generate_token();

                [[nodiscard]] static crow::json::wvalue error_dto(std::string brief, std::string detail);

                [[nodiscard]] crow::response login(const crow::json::rvalue &body) const;

                [[nodiscard]] crow::response user_register(const crow::json::rvalue &body) const;

                [[nodiscard]] crow::response post_scene(const AuthFilter::context &ctx,
                                                        const crow::json::rvalue &body) const;

                [[nodiscard]] crow::response get_scene(const AuthFilter::context &ctx) const;

                [[nodiscard]] crow::response delete_scene(const AuthFilter::context &ctx, const std::string& id) const;

                [[nodiscard]] crow::response put_scene(const AuthFilter::context &ctx, crow::json::rvalue &body) const;

                [[nodiscard]] crow::response get_circuit(const std::string &scene) const;

                [[nodiscard]] crow::response post_circuit(const crow::json::rvalue &body) const;

                [[nodiscard]] crow::response circuit_delete(std::string id) const;

                [[nodiscard]] crow::response put_circuit(std::string id, crow::json::rvalue& body) const;

                [[nodiscard]] crow::response post_wire(const crow::json::rvalue &body) const;

                [[nodiscard]] crow::response get_wire(const std::string &scene) const;

                [[nodiscard]] crow::response wire_delete(std::string id) const;
};

namespace {
        std::string scene_toString(const scene &s)
        {
                return "Scene: ID: " + s.id + ", Author: " + s.author + ", Scene Name: " + s.scene_name +
                       ", Description: " + s.description;
        }
}
