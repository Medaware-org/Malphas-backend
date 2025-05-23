#include <Api.hpp>
#include <crow_all.h>
#include <middleware/AuthFilter.hpp>

#ifndef __CMAKE_BUILD__
#include <Bcrypt.cpp/include/bcrypt.h>
#else
#include "bcrypt.h"
#endif

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <utility>
#include <dao/dao_extras.h>

MalphasApi::MalphasApi(Database &db) : db(db)
{
}

template<typename... T>
void MalphasApi::register_endpoints(crow::App<T...> &crow) const
{
        /* (Authorization handled by the proxy) */

        /* Scenes */

        CROW_ROUTE(crow, "/scene")
                .methods(crow::HTTPMethod::Post)
                ([&, this](const crow::request &req) {
                        JSON_BODY(body)
                        AUTH_CONTEXT(ctx)
                        return post_scene(ctx, body);
                });

        //TODO: extend this to use a wrapping method to incorporate "get_all_scene" and "get_one_scene"
        CROW_ROUTE(crow, "/scene")
                .methods(crow::HTTPMethod::Get)
                ([&, this](const crow::request &req) {
                        AUTH_CONTEXT(ctx)
                        return get_scene(ctx);
                });

        CROW_ROUTE(crow, "/scene")
                .methods(crow::HTTPMethod::Delete)
                ([&, this](const crow::request &req) {
                        AUTH_CONTEXT(ctx)
                        QUERY_PARAM(id, "id")
                        return delete_scene(ctx, id);
                });

        CROW_ROUTE(crow, "/scene")
                .methods(crow::HTTPMethod::Put)
                ([&, this](const crow::request &req) {
                        AUTH_CONTEXT(ctx)
                        JSON_BODY(body)
                        return put_scene(ctx, body);
                });

        /* Circuits */

        CROW_ROUTE(crow, "/circuit")
                .methods(crow::HTTPMethod::Post)
                ([this](const crow::request &req) {
                        JSON_BODY(body)
                        return post_circuit(body);
                });

        CROW_ROUTE(crow, "/circuit")
                .methods(crow::HTTPMethod::Get)
                ([this](const crow::request &req) {
                        QUERY_PARAM(scene, "scene")
                        return get_circuit(scene);
                });

        CROW_ROUTE(crow, "/circuit")
                .methods(crow::HTTPMethod::Delete)
                ([this](const crow::request &req) {
                        QUERY_PARAM(id, "id");
                        return circuit_delete(id);
                });

        CROW_ROUTE(crow, "/circuit")
                .methods(crow::HTTPMethod::Put)
                ([this](const crow::request &req) {
                        QUERY_PARAM(id, "id");
                        JSON_BODY(body);
                        return put_circuit(id, body);
                });

        /* Wires */

        CROW_ROUTE(crow, "/wire")
                .methods(crow::HTTPMethod::Post)
                ([this](const crow::request &req) {
                        JSON_BODY(body)
                        return post_wire(body);
                });

        CROW_ROUTE(crow, "/wire")
                .methods(crow::HTTPMethod::Get)
                ([this](const crow::request &req) {
                        QUERY_PARAM(scene, "scene")
                        return get_wire(scene);
                });

        CROW_ROUTE(crow, "/wire")
                .methods(crow::HTTPMethod::Delete)
                ([this](const crow::request &req) {
                        QUERY_PARAM(id, "id");
                        return wire_delete(id);
                });
}

template void MalphasApi::register_endpoints<>(crow::App<crow::CORSHandler, AuthFilter> &) const;

std::string MalphasApi::generate_token()
{
        std::string token;
        for (int i = 0; i < SESSION_TOKEN_LENGTH; i++) {
                char c = 1 + rand() % 255;
                token.append(1, c);
        }
        return bcrypt::generateHash(token);
}

crow::json::wvalue MalphasApi::error_dto(std::string brief, std::string detail)
{
        crow::json::wvalue result;
        result["summary"] = brief;
        result["description"] = detail;
        return result;
}

crow::response MalphasApi::post_scene(const AuthFilter::context &ctx, const crow::json::rvalue &body) const
{
        REQUIRE(body, scene_name, "name");
        REQUIRE(body, description, "description");

        std::string author_s = ctx.user_id;
        std::string scene_name_s = scene_name.s();
        std::string description_s = description.s();

        if (author_s.empty() || scene_name_s.empty() || description_s.empty()) {
                CROW_LOG_INFO << "PostScene: author and/or scene_name and/or description is/are empty.";
                return {400, error_dto("Invalid Credentials", "author and/or scene_name and/or description are empty")};
        }

        boost::uuids::uuid id = boost::uuids::random_generator()();

        if (!scene_save(db, to_string(id), author_s, scene_name_s, description_s)) {
                CROW_LOG_CRITICAL << "Could not store scene!";
                return {500, error_dto("Internal error", "Could not create scene")};
        }

        CROW_LOG_INFO << "Scene registered: '" << scene_name_s << "'";
        return {200, "OK"};
}

crow::response MalphasApi::get_scene(const AuthFilter::context &ctx) const
{
        std::vector<scene> dst;
        std::vector<crow::json::wvalue> scenes;
        if (!get_scenes_of_user(db, dst, ctx.user_id))
                return {400, "Error occured while GET scenes"};
        for (const auto &scene: dst) {
                crow::json::wvalue scene_json;
                scene_json["id"] = scene.id;
                scene_json["author"] = scene.author;
                scene_json["name"] = scene.scene_name;
                scene_json["description"] = scene.description;
                scenes.push_back(scene_json);
        }
        crow::json::wvalue response = scenes;
        return {200, response};
}

crow::response MalphasApi::delete_scene(const AuthFilter::context &ctx, const std::string &id) const
{
        if (!scene_delete(db, id, ctx.user_id))
                return {400, error_dto("Could not delete scene", "The scene '" + id + "' could not be deleted.")};

        CROW_LOG_INFO << "Scene deleted: " << id;
        return {200, "OK"};
}

crow::response MalphasApi::put_scene(const AuthFilter::context &ctx, crow::json::rvalue &body) const
{
        REQUIRE(body, id, "id");
        REQUIRE(body, name, "name");
        REQUIRE(body, description, "description");

        std::string name_s = name.s();
        std::string description_s = description.s();
        std::string id_s = id.s();

        if (!scene_update_basic(db, name_s, description_s, id_s, ctx.user_id))
                return {400, error_dto("Could not update", "Could not update scene")};

        CROW_LOG_DEBUG << "Scene updated: '" << id_s;
        return {200, "OK"};
}

crow::response MalphasApi::get_circuit(const std::string &scene) const
{
        std::vector<circuit> dst;
        std::vector<crow::json::wvalue> circuits;
        if (!get_circuits_in_scene(db, dst, scene))
                return {400, "Error occurred while GET circuit"};
        for (const auto &circuit: dst) {
                crow::json::wvalue circuit_json;
                circuit_json["id"] = circuit.id;
                circuit_json["parent_scene"] = circuit.parent_scene;
                circuit_json["location_x"] = circuit.location_x;
                circuit_json["location_y"] = circuit.location_y;
                circuit_json["parent_circuit"] = circuit.parent_circuit.value_or("");
                circuit_json["gate_type"] = circuit.gate_type;
                circuits.push_back(circuit_json);
        }
        crow::json::wvalue response = circuits;
        return {200, response};
}

crow::response MalphasApi::post_circuit(const crow::json::rvalue &body) const
{
        REQUIRE(body, parent_scene, "parent_scene");
        REQUIRE(body, location_x, "location_x");
        REQUIRE(body, location_y, "location_y");
        REQUIRE(body, gate_type, "gate_type");

        std::string parent_scene_s = parent_scene.s();
        std::string gate_type_s = gate_type.s();
        int location_x_i = location_x.i();
        int location_y_i = location_y.i();

        if (parent_scene_s.empty() || gate_type_s.empty()) {
                CROW_LOG_INFO << "PostCircuit: parent_scene and/or gate_type is/are empty.";
                return {400, error_dto("Invalid parameters", "parent_scene and/or gate_type are empty")};
        }

        circuit circuit;

        boost::uuids::uuid id = boost::uuids::random_generator()();

        circuit.id = to_string(id);
        circuit.parent_scene = parent_scene_s;
        circuit.gate_type = gate_type_s;
        circuit.location_x = location_x_i;
        circuit.location_y = location_y_i;

        if (body.has("parent_circuit"))
                circuit.parent_circuit = body["parent_circuit"].s();

        if (!circuit_save(db, SPREAD_CIRCUIT(circuit))) {
                CROW_LOG_CRITICAL << "Could not store circuit!";
                return {500, error_dto("Internal error", "Could not create circuit")};
        }

        CROW_LOG_INFO << "Circuit registered";
        return {200, "OK"};
}

crow::response MalphasApi::circuit_delete(std::string id) const
{
        if (!delete_circuit(db, id))
                return {400, error_dto("Error deleting", "Could not delete circuit id: '" + id + "'")};

        CROW_LOG_DEBUG << "Deleted circuit!";
        return {200, "OK"};
}

crow::response MalphasApi::put_circuit(std::string id, crow::json::rvalue &body) const
{
        OPTIONAL(body, location_x, "location_x");
        OPTIONAL(body, location_y, "location_y");
        OPTIONAL(body, parent_circuit, "parent_circuit");
        OPTIONAL(body, gate_type, "gate_type");

#define CHECK(cond) if (!cond) return { 500, "Database operation failed" }

        if (location_x.has_value())
                CHECK(update_circuit_location_x(db, (*location_x).i(), id));

        if (location_y.has_value())
                CHECK(update_circuit_location_y(db, (*location_y).i(), id));

        if (parent_circuit.has_value())
                CHECK(update_circuit_parent_circuit(db, (*parent_circuit).s(), id));

        if (gate_type.has_value())
                CHECK(update_circuit_gate_type(db, (*gate_type).s(), id));

#undef CHECK

        return {200, "OK"};
}

crow::response MalphasApi::post_wire(const crow::json::rvalue &body) const
{
        REQUIRE(body, source_circuit, "source_circuit");
        REQUIRE(body, target_circuit, "target_circuit");
        REQUIRE(body, init_signal, "init_signal");
        REQUIRE(body, number_input, "number_input");
        REQUIRE(body, number_output, "number_output");
        REQUIRE(body, location, "location");

        std::string source_circuit_s = source_circuit.s();
        std::string target_circuit_s = target_circuit.s();
        std::string location_s = location.s();
        bool init_signal_b = init_signal.b();
        int number_input_i = number_input.i();
        int number_output_i = number_output.i();

        //
        // Check if the wire path is correct
        //

        auto path = crow::json::load(location_s);
        bool path_ok = true;
        std::vector<crow::json::rvalue> points;
        if (!path || path.t() != crow::json::type::List) {
                path_ok = false;
                goto potential_errors;
        }

        points = path.lo();

        // We need at least a beginning and an end point (so 2)
        if (points.size() < 2) {
                path_ok = false;
                goto potential_errors;
        }

        for (auto &point: points) {
                if (point.t() != crow::json::type::List) {
                        path_ok = false;
                        goto potential_errors;
                }
                std::vector<crow::json::rvalue> coordinates = point.lo();

                // [X, Y]
                if (coordinates.size() != 2) {
                        path_ok = false;
                        goto potential_errors;
                }

                for (const auto &coordinate: coordinates)
                        if (coordinate.t() != crow::json::type::Number) {
                                path_ok = false;
                                goto potential_errors;
                        }
        }

potential_errors:
        if (source_circuit_s.empty() || target_circuit_s.empty() || location_s.empty() || !path_ok) {
                CROW_LOG_INFO << "PostWire: source_circuit and/or target_circuit and/or location is/are empty.";
                return {
                        400,
                        error_dto("Invalid parameters",
                                  "source_circuit and/or target_circuit and/or location are empty")
                };
        }

        wire wire;

        boost::uuids::uuid id = boost::uuids::random_generator()();

        wire.id = to_string(id);
        wire.source_circuit = source_circuit_s;
        wire.target_circuit = target_circuit_s;
        wire.init_signal = init_signal_b;
        wire.amount_input = number_input_i;
        wire.amount_output = number_output_i;
        wire.location = location_s;

        if (!wire_save(db, SPREAD_WIRE(wire))) {
                CROW_LOG_CRITICAL << "Wire could not be saved!";
                return {500, error_dto("Internal error", "Wire could not be stored!")};
        }

        CROW_LOG_INFO << "Wire registered";
        return {200, "OK"};
}

crow::response MalphasApi::get_wire(const std::string &scene) const
{
        std::vector<wire> dst;
        std::vector<crow::json::wvalue> wires;
        if (!get_wires_in_scene(db, dst, scene))
                return {400, "Error occurred while GET wire"};
        for (const auto &wire: dst) {
                crow::json::wvalue wire_json;
                wire_json["id"] = wire.id;
                wire_json["source_circuit"] = wire.source_circuit;
                wire_json["target_circuit"] = wire.target_circuit;
                wire_json["init_signal"] = wire.init_signal;
                wire_json["number_input"] = wire.amount_input;
                wire_json["number_output"] = wire.amount_output;
                wire_json["location"] = crow::json::load(wire.location);
                wires.push_back(wire_json);
        }
        crow::json::wvalue response = wires;
        return {200, response};
}

crow::response MalphasApi::wire_delete(std::string id) const
{
        if (!delete_wire(db, id))
                return {400, error_dto("Error deleting", "Could not delete wire with id: '" + id + "'")};

        CROW_LOG_DEBUG << "Deleted wire!";
        return {200, "OK"};
}
