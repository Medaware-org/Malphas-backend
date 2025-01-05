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

MalphasApi::MalphasApi(Database &db) : db(db)
{
}

template<typename... T>
void MalphasApi::register_endpoints(crow::App<T...> &crow) const
{
        CROW_ROUTE(crow, "/login")
                .methods(crow::HTTPMethod::Post)
                ([this](const crow::request &req) -> crow::response {
                        JSON_BODY(body)
                        return login(body);
                });

        CROW_ROUTE(crow, "/register")
                .methods(crow::HTTPMethod::Post)
                ([this](const crow::request &req) -> crow::response {
                        JSON_BODY(body)
                        return user_register(body);
                });

        CROW_ROUTE(crow, "/scene")
                .methods(crow::HTTPMethod::Post)
                ([&, this](const crow::request &req) {
                        JSON_BODY(body);
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

        CROW_ROUTE(crow, "/circuit")
                .methods(crow::HTTPMethod::Post)
                ([this](const crow::request &req) {
                        JSON_BODY(body);
                        return post_circuit(body);
                });

        CROW_ROUTE(crow, "/circuit")
                .methods(crow::HTTPMethod::Get)
                ([this](const crow::request &req) {
                        std::vector<circuit> dst;
                        std::vector<crow::json::wvalue> circuits;
                        if (!get_all_circuit(db, dst))
                                return crow::response(400, "Error occured while GET circuit");
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
                        return crow::response(200, response);
                });

        CROW_ROUTE(crow, "/wire")
                .methods(crow::HTTPMethod::Post)
                ([this](const crow::request &req) {
                        JSON_BODY(body);
                        return post_wire(body);
                });

        CROW_ROUTE(crow, "/wire")
                .methods(crow::HTTPMethod::Get)
                ([this](const crow::request &req) {
                        std::vector<wire> dst;
                        std::vector<crow::json::wvalue> wires;
                        if (!get_all_wire(db, dst))
                                return crow::response(400, "Error occured while GET wire");
                        for (const auto &wire: dst) {
                                crow::json::wvalue wire_json;
                                wire_json["source_circuit"] = wire.source_circuit;
                                wire_json["target_circuit"] = wire.target_circuit;
                                wire_json["init_signal"] = wire.init_signal;
                                wire_json["amount_input"] = wire.amount_input;
                                wire_json["amount_output"] = wire.amount_output;
                                wire_json["location"] = wire.location;
                                wires.push_back(wire_json);
                        }
                        crow::json::wvalue response = wires;
                        return crow::response(200, response);
                });
}

template void MalphasApi::register_endpoints<>(crow::App<crow::CORSHandler, AuthFilter> &) const;

std::string MalphasApi::generate_token() const
{
        std::string token;
        for (int i = 0; i < SESSION_TOKEN_LENGTH; i++) {
                char c = 1 + rand() % 255;
                token.append(1, c);
        }
        return bcrypt::generateHash(token);
}

crow::json::wvalue MalphasApi::error_dto(std::string brief, std::string detail) const
{
        crow::json::wvalue result;
        result["summary"] = brief;
        result["description"] = detail;
        return result;
}

crow::response MalphasApi::login(const crow::json::rvalue &body) const
{
        REQUIRE(body, username, "username");
        REQUIRE(body, password, "password");

        user usr;

        auto errorDto = error_dto("Invalid Credentials", "The credentials don't match any existing user");

        // User does not exist
        if (!get_user_by_username(db, &usr, username.s())) {
                CROW_LOG_INFO << "Login: User '" << username.s() << "' does not exist.";
                return {401, errorDto};
        }

        // Passwords don't match
        if (!bcrypt::validatePassword(password.s(), usr.passwd_hash)) {
                CROW_LOG_INFO << "Login: Password for user '" << username.s() << "' is incorrect.";
                return {401, errorDto};
        }

        std::string token = generate_token();

        // This should not happen
        if (!session_save(db, token, usr.id, false)) {
                CROW_LOG_CRITICAL << "Could not create session for user '" << username.s() << "' !";
                return {500, error_dto("Internal Error", "Could not create session")};
        }

        CROW_LOG_INFO << "Session granted for user '" << username.s() << "'";

        crow::json::wvalue response;
        response["token"] = token;
        return {200, response};
}


crow::response MalphasApi::user_register(const crow::json::rvalue &body) const
{
        REQUIRE(body, username, "username");
        REQUIRE(body, password, "password");

        std::string username_s = username.s();
        std::string password_s = password.s();

        if (username_s.empty() || password_s.empty()) {
                CROW_LOG_INFO << "Register: Username and/or password is/are empty.";
                return {400, error_dto("Invalid Credentials", "Username and/or password are empty")};
        }

        user usr;

        // Username taken
        if (get_user_by_username(db, &usr, username.s())) {
                CROW_LOG_INFO << "Register: Username '" << username.s() << "' is already taken.";
                return {409, error_dto("Username Taken", "A user with this username already exists")};
        }

        boost::uuids::uuid id = boost::uuids::random_generator()();

        // TODO Check if UUID is taken?

        usr.id = to_string(id);
        usr.nickname = username.s();
        usr.passwd_hash = bcrypt::generateHash(password.s());

        // This should not happen
        if (!user_save(db, SPREAD_USER(usr))) {
                CROW_LOG_CRITICAL << "Could not store user '" << username.s() << "' !";
                return {500, error_dto("Internal error", "Could not create user")};
        }

        CROW_LOG_INFO << "User registered: '" << username.s() << "'";

        return {200, "OK"};
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

crow::response MalphasApi::delete_scene(const AuthFilter::context &ctx, std::string id) const
{
        if (!scene_delete(db, id, ctx.user_id))
                return {400, error_dto("Could not delete scene", "The scene '" + id + "' could not be deleted.")};

        CROW_LOG_INFO << "Scene deleted: " << id;
        return {200, "OK"};
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
                return {400, error_dto("Invalid Credentials", "parent_scene and/or gate_type are empty")};
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

crow::response MalphasApi::post_wire(const crow::json::rvalue &body) const
{
        REQUIRE(body, source_circuit, "source_circuit");
        REQUIRE(body, target_circuit, "target_circuit");
        REQUIRE(body, init_signal, "init_signal");
        REQUIRE(body, amount_input, "amount_input");
        REQUIRE(body, amount_output, "amount_output");
        REQUIRE(body, location, "location");

        std::string source_circuit_s = source_circuit.s();
        std::string target_circuit_s = target_circuit.s();
        std::string location_s = location.s();
        bool init_signal_b = init_signal.b();
        int amount_input_i = amount_input.i();
        int amount_output_i = amount_output.i();

        if (source_circuit_s.empty() || target_circuit_s.empty() || location_s.empty()) {
                CROW_LOG_INFO << "PostWire: source_circuit and/or target_circuit and/or location is/are empty.";
                return {
                        400,
                        error_dto("Invalid Credentials",
                                  "source_circuit and/or target_circuit and/or location are empty")
                };
        }

        wire wire;

        boost::uuids::uuid id = boost::uuids::random_generator()();

        wire.id = to_string(id);
        wire.source_circuit = source_circuit_s;
        wire.target_circuit = target_circuit_s;
        wire.init_signal = init_signal_b;
        wire.amount_input = amount_input_i;
        wire.amount_output = amount_output_i;
        wire.location = location_s;

        if (!wire_save(db, SPREAD_WIRE(wire))) {
                CROW_LOG_CRITICAL << "Wire could not be saved!";
                return {500, error_dto("Internal error", "Wire could not be stored!")};
        }

        CROW_LOG_INFO << "Wire registered";
        return {200, "OK"};
}
