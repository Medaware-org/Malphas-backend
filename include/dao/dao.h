/**
 * This file was automatically generated by the Malphas DAO generator through database introspection
 */

#pragma once

#include <string>
#include <any>
#include <libpq-fe.h>
#include <Database.hpp>
#include <dao_preamble.h>

// BEGIN GENERATED CODE

struct user {
	std::string id;
	std::string nickname;
	std::string passwd_hash;
};

[[nodiscard]] inline user dao_map_user(PGresult *result, int tuple) {
	return user {
		.id = std::string(PQgetvalue(result, tuple,0)),
		.nickname = std::string(PQgetvalue(result, tuple,1)),
		.passwd_hash = std::string(PQgetvalue(result, tuple,2)),
	};
}

[[nodiscard]] inline bool user_insert(Database &db, std::string /*PK*/ id, std::string nickname, std::string passwd_hash) {
	std::string query = "INSERT INTO \"user\" (id, nickname, passwd_hash) VALUES (" + std::string("'") + xto_string(id) + std::string("'") + ", " + std::string("'") + xto_string(nickname) + std::string("'") + ", " + std::string("'") + xto_string(passwd_hash) + std::string("'") + ")";
	return finalize_op(dao_query(db, query, PGRES_COMMAND_OK));
}

[[nodiscard]] inline bool user_update(Database &db, std::string /*PK*/ id, std::string nickname, std::string passwd_hash)
{
	std::string query = "UPDATE \"user\" SET nickname = " + std::string("'") + xto_string(nickname) + std::string("'") + ", passwd_hash = " + std::string("'") + xto_string(passwd_hash) + std::string("'") + " WHERE id = " + std::string("'") + xto_string(id) + std::string("'") + ";";
	return finalize_op(dao_query(db, query, PGRES_COMMAND_OK));
}

[[nodiscard]] inline bool get_one_user(Database &db, user *dst, std::string id)
{
	std::string query = "SELECT * FROM \"user\" WHERE id = '" + xto_string(id)+ "'";
	PGresult *res = dao_query(db, query, PGRES_TUPLES_OK);
	if (!res) return false;
	if (PQntuples(res) != 1) {
		PQclear(res);
		return false;
	}
	*dst = dao_map_user(res, 0);
	PQclear(res);
	return true;
}

[[nodiscard]] inline bool get_all_user(Database &db, std::vector<user> &dst)
{
	std::string query = "SELECT * from \"user\"";
	PGresult *res = dao_query(db, query, PGRES_TUPLES_OK);
	if (!res) return false;
	dao_map_all<user>(res, dst, [](auto *res, auto tuple) { return dao_map_user(res, tuple); });
	PQclear(res);
	return true;
}

[[nodiscard]] inline bool user_save(Database &db, std::string /*PK*/ id, std::string nickname, std::string passwd_hash)
{
	user tmp;
	if (!get_one_user(db, &tmp, id))
		return user_insert(db, id, nickname, passwd_hash);
	return user_update(db, id, nickname, passwd_hash);
}

#define SPREAD_USER(user_struct) user_struct.id, user_struct.nickname, user_struct.passwd_hash
#define SPREAD_USER_PTR(user_struct) user_struct->id, user_struct->nickname, user_struct->passwd_hash

struct session {
	std::string session_token;
	std::string user_id;
	std::optional<bool> invalidated;
};

[[nodiscard]] inline session dao_map_session(PGresult *result, int tuple) {
	return session {
		.session_token = std::string(PQgetvalue(result, tuple,0)),
		.user_id = std::string(PQgetvalue(result, tuple,1)),
		.invalidated = IFNNULL(2, cast_bool(PQgetvalue(result, tuple,2))),
	};
}

[[nodiscard]] inline bool session_insert(Database &db, std::string /*PK*/ session_token, std::string user_id, std::optional<bool> invalidated) {
	std::string query = "INSERT INTO \"session\" (session_token, user_id, invalidated) VALUES (" + std::string("'") + xto_string(session_token) + std::string("'") + ", " + std::string("'") + xto_string(user_id) + std::string("'") + ", " + (invalidated ? (xto_string(*invalidated)) : "null")+ ")";
	return finalize_op(dao_query(db, query, PGRES_COMMAND_OK));
}

[[nodiscard]] inline bool session_update(Database &db, std::string /*PK*/ session_token, std::string user_id, std::optional<bool> invalidated)
{
	std::string query = "UPDATE \"session\" SET user_id = " + std::string("'") + xto_string(user_id) + std::string("'") + ", invalidated = " + (invalidated ? (xto_string(*invalidated)) : "null") + " WHERE session_token = " + std::string("'") + xto_string(session_token) + std::string("'") + ";";
	return finalize_op(dao_query(db, query, PGRES_COMMAND_OK));
}

[[nodiscard]] inline bool get_one_session(Database &db, session *dst, std::string session_token)
{
	std::string query = "SELECT * FROM \"session\" WHERE session_token = '" + xto_string(session_token)+ "'";
	PGresult *res = dao_query(db, query, PGRES_TUPLES_OK);
	if (!res) return false;
	if (PQntuples(res) != 1) {
		PQclear(res);
		return false;
	}
	*dst = dao_map_session(res, 0);
	PQclear(res);
	return true;
}

[[nodiscard]] inline bool get_all_session(Database &db, std::vector<session> &dst)
{
	std::string query = "SELECT * from \"session\"";
	PGresult *res = dao_query(db, query, PGRES_TUPLES_OK);
	if (!res) return false;
	dao_map_all<session>(res, dst, [](auto *res, auto tuple) { return dao_map_session(res, tuple); });
	PQclear(res);
	return true;
}

[[nodiscard]] inline bool session_save(Database &db, std::string /*PK*/ session_token, std::string user_id, std::optional<bool> invalidated)
{
	session tmp;
	if (!get_one_session(db, &tmp, session_token))
		return session_insert(db, session_token, user_id, invalidated);
	return session_update(db, session_token, user_id, invalidated);
}

#define SPREAD_SESSION(session_struct) session_struct.session_token, session_struct.user_id, session_struct.invalidated
#define SPREAD_SESSION_PTR(session_struct) session_struct->session_token, session_struct->user_id, session_struct->invalidated

struct scene {
	std::string id;
	std::string author;
	std::string scene_name;
	std::string description;
};

[[nodiscard]] inline scene dao_map_scene(PGresult *result, int tuple) {
	return scene {
		.id = std::string(PQgetvalue(result, tuple,0)),
		.author = std::string(PQgetvalue(result, tuple,1)),
		.scene_name = std::string(PQgetvalue(result, tuple,2)),
		.description = std::string(PQgetvalue(result, tuple,3)),
	};
}

[[nodiscard]] inline bool scene_insert(Database &db, std::string /*PK*/ id, std::string author, std::string scene_name, std::string description) {
	std::string query = "INSERT INTO \"scene\" (id, author, scene_name, description) VALUES (" + std::string("'") + xto_string(id) + std::string("'") + ", " + std::string("'") + xto_string(author) + std::string("'") + ", " + std::string("'") + xto_string(scene_name) + std::string("'") + ", " + std::string("'") + xto_string(description) + std::string("'") + ")";
	return finalize_op(dao_query(db, query, PGRES_COMMAND_OK));
}

[[nodiscard]] inline bool scene_update(Database &db, std::string /*PK*/ id, std::string author, std::string scene_name, std::string description)
{
	std::string query = "UPDATE \"scene\" SET author = " + std::string("'") + xto_string(author) + std::string("'") + ", scene_name = " + std::string("'") + xto_string(scene_name) + std::string("'") + ", description = " + std::string("'") + xto_string(description) + std::string("'") + " WHERE id = " + std::string("'") + xto_string(id) + std::string("'") + ";";
	return finalize_op(dao_query(db, query, PGRES_COMMAND_OK));
}

[[nodiscard]] inline bool get_one_scene(Database &db, scene *dst, std::string id)
{
	std::string query = "SELECT * FROM \"scene\" WHERE id = '" + xto_string(id)+ "'";
	PGresult *res = dao_query(db, query, PGRES_TUPLES_OK);
	if (!res) return false;
	if (PQntuples(res) != 1) {
		PQclear(res);
		return false;
	}
	*dst = dao_map_scene(res, 0);
	PQclear(res);
	return true;
}

[[nodiscard]] inline bool get_all_scene(Database &db, std::vector<scene> &dst)
{
	std::string query = "SELECT * from \"scene\"";
	PGresult *res = dao_query(db, query, PGRES_TUPLES_OK);
	if (!res) return false;
	dao_map_all<scene>(res, dst, [](auto *res, auto tuple) { return dao_map_scene(res, tuple); });
	PQclear(res);
	return true;
}

[[nodiscard]] inline bool scene_save(Database &db, std::string /*PK*/ id, std::string author, std::string scene_name, std::string description)
{
	scene tmp;
	if (!get_one_scene(db, &tmp, id))
		return scene_insert(db, id, author, scene_name, description);
	return scene_update(db, id, author, scene_name, description);
}

#define SPREAD_SCENE(scene_struct) scene_struct.id, scene_struct.author, scene_struct.scene_name, scene_struct.description
#define SPREAD_SCENE_PTR(scene_struct) scene_struct->id, scene_struct->author, scene_struct->scene_name, scene_struct->description

struct circuit {
	std::string id;
	std::string parent_scene;
	int location_x;
	int location_y;
	std::optional<std::string> parent_circuit;
	std::string gate_type;
};

[[nodiscard]] inline circuit dao_map_circuit(PGresult *result, int tuple) {
	return circuit {
		.id = std::string(PQgetvalue(result, tuple,0)),
		.parent_scene = std::string(PQgetvalue(result, tuple,1)),
		.location_x = std::stoi(PQgetvalue(result, tuple,2)),
		.location_y = std::stoi(PQgetvalue(result, tuple,3)),
		.parent_circuit = IFNNULL(4, std::string(PQgetvalue(result, tuple,4))),
		.gate_type = std::string(PQgetvalue(result, tuple,5)),
	};
}

[[nodiscard]] inline bool circuit_insert(Database &db, std::string /*PK*/ id, std::string parent_scene, int location_x, int location_y, std::optional<std::string> parent_circuit, std::string gate_type) {
	std::string query = "INSERT INTO \"circuit\" (id, parent_scene, location_x, location_y, parent_circuit, gate_type) VALUES (" + std::string("'") + xto_string(id) + std::string("'") + ", " + std::string("'") + xto_string(parent_scene) + std::string("'") + ", " + xto_string(location_x) + ", " + xto_string(location_y) + ", " + (parent_circuit ? (std::string("'") + xto_string(*parent_circuit) + std::string("'")) : "null") + ", " + std::string("'") + xto_string(gate_type) + std::string("'") + ")";
	return finalize_op(dao_query(db, query, PGRES_COMMAND_OK));
}

[[nodiscard]] inline bool circuit_update(Database &db, std::string /*PK*/ id, std::string parent_scene, int location_x, int location_y, std::optional<std::string> parent_circuit, std::string gate_type)
{
	std::string query = "UPDATE \"circuit\" SET parent_scene = " + std::string("'") + xto_string(parent_scene) + std::string("'") + ", location_x = " + xto_string(location_x) + ", location_y = " + xto_string(location_y) + ", parent_circuit = " + (parent_circuit ? (std::string("'") + xto_string(*parent_circuit) + std::string("'")) : "null") + ", gate_type = " + std::string("'") + xto_string(gate_type) + std::string("'") + " WHERE id = " + std::string("'") + xto_string(id) + std::string("'") + ";";
	return finalize_op(dao_query(db, query, PGRES_COMMAND_OK));
}

[[nodiscard]] inline bool get_one_circuit(Database &db, circuit *dst, std::string id)
{
	std::string query = "SELECT * FROM \"circuit\" WHERE id = '" + xto_string(id)+ "'";
	PGresult *res = dao_query(db, query, PGRES_TUPLES_OK);
	if (!res) return false;
	if (PQntuples(res) != 1) {
		PQclear(res);
		return false;
	}
	*dst = dao_map_circuit(res, 0);
	PQclear(res);
	return true;
}

[[nodiscard]] inline bool get_all_circuit(Database &db, std::vector<circuit> &dst)
{
	std::string query = "SELECT * from \"circuit\"";
	PGresult *res = dao_query(db, query, PGRES_TUPLES_OK);
	if (!res) return false;
	dao_map_all<circuit>(res, dst, [](auto *res, auto tuple) { return dao_map_circuit(res, tuple); });
	PQclear(res);
	return true;
}

[[nodiscard]] inline bool circuit_save(Database &db, std::string /*PK*/ id, std::string parent_scene, int location_x, int location_y, std::optional<std::string> parent_circuit, std::string gate_type)
{
	circuit tmp;
	if (!get_one_circuit(db, &tmp, id))
		return circuit_insert(db, id, parent_scene, location_x, location_y, parent_circuit, gate_type);
	return circuit_update(db, id, parent_scene, location_x, location_y, parent_circuit, gate_type);
}

#define SPREAD_CIRCUIT(circuit_struct) circuit_struct.id, circuit_struct.parent_scene, circuit_struct.location_x, circuit_struct.location_y, circuit_struct.parent_circuit, circuit_struct.gate_type
#define SPREAD_CIRCUIT_PTR(circuit_struct) circuit_struct->id, circuit_struct->parent_scene, circuit_struct->location_x, circuit_struct->location_y, circuit_struct->parent_circuit, circuit_struct->gate_type

struct wire {
	std::string id;
	std::string source_circuit;
	std::string target_circuit;
	bool init_signal;
	int amount_input;
	int amount_output;
	std::string location;
};

[[nodiscard]] inline wire dao_map_wire(PGresult *result, int tuple) {
	return wire {
		.id = std::string(PQgetvalue(result, tuple,0)),
		.source_circuit = std::string(PQgetvalue(result, tuple,1)),
		.target_circuit = std::string(PQgetvalue(result, tuple,2)),
		.init_signal = cast_bool(PQgetvalue(result, tuple,3)),
		.amount_input = std::stoi(PQgetvalue(result, tuple,4)),
		.amount_output = std::stoi(PQgetvalue(result, tuple,5)),
		.location = std::string(PQgetvalue(result, tuple,6)),
	};
}

[[nodiscard]] inline bool wire_insert(Database &db, std::string /*PK*/ id, std::string source_circuit, std::string target_circuit, bool init_signal, int amount_input, int amount_output, std::string location) {
	std::string query = "INSERT INTO \"wire\" (id, source_circuit, target_circuit, init_signal, amount_input, amount_output, location) VALUES (" + std::string("'") + xto_string(id) + std::string("'") + ", " + std::string("'") + xto_string(source_circuit) + std::string("'") + ", " + std::string("'") + xto_string(target_circuit) + std::string("'") + ", " + xto_string(init_signal) + ", " + xto_string(amount_input) + ", " + xto_string(amount_output) + ", " + std::string("'") + xto_string(location) + std::string("'") + ")";
	return finalize_op(dao_query(db, query, PGRES_COMMAND_OK));
}

[[nodiscard]] inline bool wire_update(Database &db, std::string /*PK*/ id, std::string source_circuit, std::string target_circuit, bool init_signal, int amount_input, int amount_output, std::string location)
{
	std::string query = "UPDATE \"wire\" SET source_circuit = " + std::string("'") + xto_string(source_circuit) + std::string("'") + ", target_circuit = " + std::string("'") + xto_string(target_circuit) + std::string("'") + ", init_signal = " + xto_string(init_signal) + ", amount_input = " + xto_string(amount_input) + ", amount_output = " + xto_string(amount_output) + ", location = " + std::string("'") + xto_string(location) + std::string("'") + " WHERE id = " + std::string("'") + xto_string(id) + std::string("'") + ";";
	return finalize_op(dao_query(db, query, PGRES_COMMAND_OK));
}

[[nodiscard]] inline bool get_one_wire(Database &db, wire *dst, std::string id)
{
	std::string query = "SELECT * FROM \"wire\" WHERE id = '" + xto_string(id)+ "'";
	PGresult *res = dao_query(db, query, PGRES_TUPLES_OK);
	if (!res) return false;
	if (PQntuples(res) != 1) {
		PQclear(res);
		return false;
	}
	*dst = dao_map_wire(res, 0);
	PQclear(res);
	return true;
}

[[nodiscard]] inline bool get_all_wire(Database &db, std::vector<wire> &dst)
{
	std::string query = "SELECT * from \"wire\"";
	PGresult *res = dao_query(db, query, PGRES_TUPLES_OK);
	if (!res) return false;
	dao_map_all<wire>(res, dst, [](auto *res, auto tuple) { return dao_map_wire(res, tuple); });
	PQclear(res);
	return true;
}

[[nodiscard]] inline bool wire_save(Database &db, std::string /*PK*/ id, std::string source_circuit, std::string target_circuit, bool init_signal, int amount_input, int amount_output, std::string location)
{
	wire tmp;
	if (!get_one_wire(db, &tmp, id))
		return wire_insert(db, id, source_circuit, target_circuit, init_signal, amount_input, amount_output, location);
	return wire_update(db, id, source_circuit, target_circuit, init_signal, amount_input, amount_output, location);
}

#define SPREAD_WIRE(wire_struct) wire_struct.id, wire_struct.source_circuit, wire_struct.target_circuit, wire_struct.init_signal, wire_struct.amount_input, wire_struct.amount_output, wire_struct.location
#define SPREAD_WIRE_PTR(wire_struct) wire_struct->id, wire_struct->source_circuit, wire_struct->target_circuit, wire_struct->init_signal, wire_struct->amount_input, wire_struct->amount_output, wire_struct->location

//
// Custom Functions
//

[[nodiscard]] inline bool delete_wire(Database &db, std::string id)
{
	std::string query = "DELETE from wire w where w.id = '" + id + "';";
	PGresult *res = dao_query(db, query, PGRES_COMMAND_OK);
	if (!res) return false;
	PQclear(res);
	return true;
}

[[nodiscard]] inline bool get_session_user(Database &db, user *dst, std::string user_id)
{
	std::string query = "SELECT * FROM \"user\" u INNER JOIN session s on s.user_id = u.id WHERE s.session_token = '" + user_id + "';";
	PGresult *res = dao_query(db, query, PGRES_TUPLES_OK);
	if (!res) return false;
	if (PQntuples(res) != 1) {
		PQclear(res);
		return false;
	}
	*dst = dao_map_user(res, 0);
	PQclear(res);
	return true;
}

[[nodiscard]] inline bool get_user_by_username(Database &db, user *dst, std::string username)
{
	std::string query = "SELECT * FROM \"user\" u WHERE u.nickname = '" + username + "';";
	PGresult *res = dao_query(db, query, PGRES_TUPLES_OK);
	if (!res) return false;
	if (PQntuples(res) != 1) {
		PQclear(res);
		return false;
	}
	*dst = dao_map_user(res, 0);
	PQclear(res);
	return true;
}

[[nodiscard]] inline bool get_scenes_of_user(Database &db, std::vector<scene> &dst, std::string user_id)
{
	std::string query = "SELECT * FROM scene s WHERE s.author = '" + user_id + "';";
	PGresult *res = dao_query(db, query, PGRES_TUPLES_OK);
	if (!res) return false;
	dao_map_all<scene>(res, dst, [](auto *res, auto tuple) { return dao_map_scene(res, tuple); });
	PQclear(res);
	return true;
}

[[nodiscard]] inline bool get_sessions_of_user(Database &db, std::vector<session> &dst, std::string user_id)
{
	std::string query = "SELECT * FROM session s WHERE s.user_id = '" + user_id + "';";
	PGresult *res = dao_query(db, query, PGRES_TUPLES_OK);
	if (!res) return false;
	dao_map_all<session>(res, dst, [](auto *res, auto tuple) { return dao_map_session(res, tuple); });
	PQclear(res);
	return true;
}

[[nodiscard]] inline bool invalidate_session(Database &db, std::string session_token)
{
	std::string query = "UPDATE session s SET s.invalidated = true WHERE s.session_token = '" + session_token + "';";
	PGresult *res = dao_query(db, query, PGRES_COMMAND_OK);
	if (!res) return false;
	PQclear(res);
	return true;
}

// END GENERATED CODE
