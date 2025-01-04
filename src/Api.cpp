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
            ([this](const crow::request& req) {
                    JSON_BODY(body);
                    return post_scene(body);
                });

        //TODO: extend this to use a wrapping method to incorporate "get_all_scene" and "get_one_scene"
        CROW_ROUTE(crow, "/scene")
            .methods(crow::HTTPMethod::Get)
            ([this](const crow::request& req) {
                    std::vector<scene> dst;
                    get_all_scene(db, dst);
                    std::string res = "Get Scenes: ";
                    for (scene s : dst)
                        res += "{ " + scene_toString(s) + "}";
                    return crow::response(200, res);
                });

        CROW_ROUTE(crow, "/circuit")
            .methods(crow::HTTPMethod::Post)
            ([this](const crow::request& req) {
                    JSON_BODY(body);
                    return post_circuit(body);
                });
}

template void MalphasApi::register_endpoints<>(crow::App<crow::CORSHandler, AuthFilter> &) const;

std::string MalphasApi::generate_token() const
{
        std::string token;
        for (int i = 0; i < SESSION_TOKEN_LENGTH; i++) {
                char c = '(' + rand() % ('Z' - '(');
                token.append(1, c);
        }
        return token;
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
                CROW_LOG_DEBUG << "Login: User '" << username.s() << "' does not exist.";
                return {401, errorDto};
        }

        // Passwords don't match
        if (!bcrypt::validatePassword(password.s(), usr.passwd_hash)) {
                CROW_LOG_DEBUG << "Login: Password for user '" << username.s() << "' is incorrect.";
                return {401, errorDto};
        }

        std::string token = generate_token();

        // This should not happen
        if (!session_save(db, token, usr.id)) {
                CROW_LOG_CRITICAL << "Could not create session for user '" << username.s() << "' !";
                return {500, error_dto("Internal Error", "Could not create session")};
        }

        CROW_LOG_DEBUG << "Session granted for user '" << username.s() << "'";

        return {200, token};
}


crow::response MalphasApi::user_register(const crow::json::rvalue &body) const
{
        REQUIRE(body, username, "username");
        REQUIRE(body, password, "password");

        std::string username_s = username.s();
        std::string password_s = password.s();

        if (username_s.empty() || password_s.empty()) {
                CROW_LOG_DEBUG << "Register: Username and/or password is/are empty.";
                return {400, error_dto("Invalid Credentials", "Username and/or password are empty")};
        }

        user usr;

        // Username taken
        if (get_user_by_username(db, &usr, username.s())) {
                CROW_LOG_DEBUG << "Register: Username '" << username.s() << "' is already taken.";
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

        CROW_LOG_DEBUG << "User registered: '" << username.s() << "'";

        return {200, "OK"};
}

crow::response MalphasApi::user_get_scenes(const crow::request &req) const
{
        // req.middleware_context()
        return {200, "OK"};
}

crow::response MalphasApi::post_scene(const crow::json::rvalue& body) const
{
    REQUIRE(body, author     , "author");
    REQUIRE(body, scene_name , "scene_name");
    REQUIRE(body, description, "description");

    std::string author_s = author.s();
    std::string scene_name_s = scene_name.s();
    std::string description_s = description.s();

    if (author_s.empty() || scene_name_s.empty() || description_s.empty())
    {
        CROW_LOG_DEBUG << "PostScene: author and/or scene_name and/or description is/are empty.";
        return { 400, error_dto("Invalid Credentials", "author and/or scene_name and/or description are empty") };
    }

    boost::uuids::uuid id = boost::uuids::random_generator()();

    if (!scene_save(db, author_s, description_s, to_string(id), scene_name_s))
        return { 500, error_dto("Internal error", "Could not create scene") };

    CROW_LOG_DEBUG << "Scene registered: '" << scene_name_s << "'";
    return { 200, "OK" };
}

crow::response MalphasApi::post_circuit(const crow::json::rvalue& body) const
{
    return crow::response();
}
