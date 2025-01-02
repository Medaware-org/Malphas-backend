#include <Api.hpp>
#include <crow_all.h>
#include <dao/dao.h>
#include <middleware/AuthFilter.hpp>

MalphasApi::MalphasApi(Database &db) : db(db)
{
}

template<typename... T>
void MalphasApi::register_endpoints(crow::App<T...> &crow) const
{
        CROW_ROUTE(crow, "/")([this](const crow::request &req) {
                JSON_BODY(body)
                return login(body);
        });
}

template void MalphasApi::register_endpoints<>(crow::App<AuthFilter> &) const;

crow::response MalphasApi::login(const crow::json::rvalue &body) const
{
        REQUIRE(body, username, "username");
        REQUIRE(body, password, "password");

        user usr;
        if (!get_user_by_username(db, &usr, username.s()))
                return {401, "Unauthorized"};



}
