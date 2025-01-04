/**
 * This file was automatically generated by the Malphas DAO generator through database introspection
 */

#pragma once

#include <string>
#include <any>
#include <libpq-fe.h>
#include <Database.hpp>

#define NO_CAST(x) (x)
[[nodiscard]] inline bool cast_bool(std::string &&str) { return (str == "true"); }

template<typename T> [[nodiscard]] inline typename std::enable_if<std::is_arithmetic<T>::value, std::string>::type xto_string(T arg) { return std::to_string(arg); }
template<typename T> [[nodiscard]] inline typename std::enable_if<std::is_same<T, std::string>::value, std::string>::type xto_string(T arg) { return arg; }

inline bool finalize_op(PGresult *res) {
        if (!res)
                return false;
        PQclear(res);
        return true;
}

inline PGresult *dao_query(Database &db, std::string query, ExecStatusType assert_status)
{
        ExecStatusType status;
        PGresult *res = db.query(query, &status);
        if (status != assert_status) {
                PQclear(res);
                return NULL;
        }
        return res;
}

template<typename T>
inline void dao_map_all(PGresult *res, std::vector<T> &dst, std::function<T(PGresult *res, int tuple)> mapper)
{
       for (int i = 0; i < PQntuples(res); i++)
               dst.push_back(mapper(res, i));
}

// BEGIN GENERATED CODE

struct user {
	std::string id;
	std::string nickname;
	std::string passwd_hash;
};

[[nodiscard]] inline user dao_map_user(PGresult *result, int tuple) {
	return user {
		.id = NO_CAST(PQgetvalue(result, tuple,0)),
		.nickname = NO_CAST(PQgetvalue(result, tuple,1)),
		.passwd_hash = NO_CAST(PQgetvalue(result, tuple,2)),
	};
}

[[nodiscard]] inline bool user_insert(Database &db, std::string /*PK*/ id, std::string nickname, std::string passwd_hash) {
	std::string query = "INSERT INTO \"user\" (id, nickname, passwd_hash) VALUES ('" + xto_string(id) + "', '" + xto_string(nickname) + "', '" + xto_string(passwd_hash) + "')";
	return finalize_op(dao_query(db, query, PGRES_COMMAND_OK));
}

[[nodiscard]] inline bool user_update(Database &db, std::string /*PK*/ id, std::string nickname, std::string passwd_hash)
{
	std::string query = "UPDATE \"user\" SET nickname = '" + xto_string(nickname) + "', passwd_hash = '" + xto_string(passwd_hash) + "' WHERE id = '" + xto_string(id) + "';";
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
};

[[nodiscard]] inline session dao_map_session(PGresult *result, int tuple) {
	return session {
		.session_token = NO_CAST(PQgetvalue(result, tuple,0)),
		.user_id = NO_CAST(PQgetvalue(result, tuple,1)),
	};
}

[[nodiscard]] inline bool session_insert(Database &db, std::string /*PK*/ session_token, std::string user_id) {
	std::string query = "INSERT INTO \"session\" (session_token, user_id) VALUES ('" + xto_string(session_token) + "', '" + xto_string(user_id) + "')";
	return finalize_op(dao_query(db, query, PGRES_COMMAND_OK));
}

[[nodiscard]] inline bool session_update(Database &db, std::string /*PK*/ session_token, std::string user_id)
{
	std::string query = "UPDATE \"session\" SET user_id = '" + xto_string(user_id) + "' WHERE session_token = '" + xto_string(session_token) + "';";
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

[[nodiscard]] inline bool session_save(Database &db, std::string /*PK*/ session_token, std::string user_id)
{
	session tmp;
	if (!get_one_session(db, &tmp, session_token))
		return session_insert(db, session_token, user_id);
	return session_update(db, session_token, user_id);
}

#define SPREAD_SESSION(session_struct) session_struct.session_token, session_struct.user_id
#define SPREAD_SESSION_PTR(session_struct) session_struct->session_token, session_struct->user_id

struct scene {
	std::string author;
	std::string description;
	std::string id;
	std::string scene_name;
};

[[nodiscard]] inline scene dao_map_scene(PGresult *result, int tuple) {
	return scene {
		.author = NO_CAST(PQgetvalue(result, tuple,0)),
		.description = NO_CAST(PQgetvalue(result, tuple,1)),
		.id = NO_CAST(PQgetvalue(result, tuple,2)),
		.scene_name = NO_CAST(PQgetvalue(result, tuple,3)),
	};
}

[[nodiscard]] inline bool scene_insert(Database &db, std::string author, std::string description, std::string /*PK*/ id, std::string scene_name) {
	std::string query = "INSERT INTO \"scene\" (author, description, id, scene_name) VALUES ('" + xto_string(author) + "', '" + xto_string(description) + "', '" + xto_string(id) + "', '" + xto_string(scene_name) + "')";
	return finalize_op(dao_query(db, query, PGRES_COMMAND_OK));
}

[[nodiscard]] inline bool scene_update(Database &db, std::string author, std::string description, std::string /*PK*/ id, std::string scene_name)
{
	std::string query = "UPDATE \"scene\" SET author = '" + xto_string(author) + "', description = '" + xto_string(description) + "', scene_name = '" + xto_string(scene_name) + "' WHERE id = '" + xto_string(id) + "';";
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

[[nodiscard]] inline bool scene_save(Database &db, std::string author, std::string description, std::string /*PK*/ id, std::string scene_name)
{
	scene tmp;
	if (!get_one_scene(db, &tmp, id))
		return scene_insert(db, author, description, id, scene_name);
	return scene_update(db, author, description, id, scene_name);
}

#define SPREAD_SCENE(scene_struct) scene_struct.author, scene_struct.description, scene_struct.id, scene_struct.scene_name
#define SPREAD_SCENE_PTR(scene_struct) scene_struct->author, scene_struct->description, scene_struct->id, scene_struct->scene_name

struct circuit {
	std::string gate_type;
	std::string id;
	int location_x;
	int location_y;
	std::string parent_circuit;
	std::string parent_scene;
};

[[nodiscard]] inline circuit dao_map_circuit(PGresult *result, int tuple) {
	return circuit {
		.gate_type = NO_CAST(PQgetvalue(result, tuple,0)),
		.id = NO_CAST(PQgetvalue(result, tuple,1)),
		.location_x = std::stoi(PQgetvalue(result, tuple,2)),
		.location_y = std::stoi(PQgetvalue(result, tuple,3)),
		.parent_circuit = NO_CAST(PQgetvalue(result, tuple,4)),
		.parent_scene = NO_CAST(PQgetvalue(result, tuple,5)),
	};
}

[[nodiscard]] inline bool circuit_insert(Database &db, std::string gate_type, std::string /*PK*/ id, int location_x, int location_y, std::string parent_circuit, std::string parent_scene) {
	std::string query = "INSERT INTO \"circuit\" (gate_type, id, location_x, location_y, parent_circuit, parent_scene) VALUES ('" + xto_string(gate_type) + "', '" + xto_string(id) + "', " + xto_string(location_x) + ", " + xto_string(location_y) + ", '" + xto_string(parent_circuit) + "', '" + xto_string(parent_scene) + "')";
	return finalize_op(dao_query(db, query, PGRES_COMMAND_OK));
}

[[nodiscard]] inline bool circuit_update(Database &db, std::string gate_type, std::string /*PK*/ id, int location_x, int location_y, std::string parent_circuit, std::string parent_scene)
{
	std::string query = "UPDATE \"circuit\" SET gate_type = '" + xto_string(gate_type) + "', location_x = " + xto_string(location_x) + ", location_y = " + xto_string(location_y) + ", parent_circuit = '" + xto_string(parent_circuit) + "', parent_scene = '" + xto_string(parent_scene) + "' WHERE id = '" + xto_string(id) + "';";
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

[[nodiscard]] inline bool circuit_save(Database &db, std::string gate_type, std::string /*PK*/ id, int location_x, int location_y, std::string parent_circuit, std::string parent_scene)
{
	circuit tmp;
	if (!get_one_circuit(db, &tmp, id))
		return circuit_insert(db, gate_type, id, location_x, location_y, parent_circuit, parent_scene);
	return circuit_update(db, gate_type, id, location_x, location_y, parent_circuit, parent_scene);
}

#define SPREAD_CIRCUIT(circuit_struct) circuit_struct.gate_type, circuit_struct.id, circuit_struct.location_x, circuit_struct.location_y, circuit_struct.parent_circuit, circuit_struct.parent_scene
#define SPREAD_CIRCUIT_PTR(circuit_struct) circuit_struct->gate_type, circuit_struct->id, circuit_struct->location_x, circuit_struct->location_y, circuit_struct->parent_circuit, circuit_struct->parent_scene

struct wire {
	int amount_input;
	int amount_output;
	std::string id;
	bool init_signal;
	std::string location;
	std::string source_circuit;
	std::string target_circuit;
};

[[nodiscard]] inline wire dao_map_wire(PGresult *result, int tuple) {
	return wire {
		.amount_input = std::stoi(PQgetvalue(result, tuple,0)),
		.amount_output = std::stoi(PQgetvalue(result, tuple,1)),
		.id = NO_CAST(PQgetvalue(result, tuple,2)),
		.init_signal = cast_bool(PQgetvalue(result, tuple,3)),
		.location = NO_CAST(PQgetvalue(result, tuple,4)),
		.source_circuit = NO_CAST(PQgetvalue(result, tuple,5)),
		.target_circuit = NO_CAST(PQgetvalue(result, tuple,6)),
	};
}

[[nodiscard]] inline bool wire_insert(Database &db, int amount_input, int amount_output, std::string /*PK*/ id, bool init_signal, std::string location, std::string source_circuit, std::string target_circuit) {
	std::string query = "INSERT INTO \"wire\" (amount_input, amount_output, id, init_signal, location, source_circuit, target_circuit) VALUES (" + xto_string(amount_input) + ", " + xto_string(amount_output) + ", '" + xto_string(id) + "', " + xto_string(init_signal) + ", '" + xto_string(location) + "', '" + xto_string(source_circuit) + "', '" + xto_string(target_circuit) + "')";
	return finalize_op(dao_query(db, query, PGRES_COMMAND_OK));
}

[[nodiscard]] inline bool wire_update(Database &db, int amount_input, int amount_output, std::string /*PK*/ id, bool init_signal, std::string location, std::string source_circuit, std::string target_circuit)
{
	std::string query = "UPDATE \"wire\" SET amount_input = " + xto_string(amount_input) + ", amount_output = " + xto_string(amount_output) + ", init_signal = " + xto_string(init_signal) + ", location = '" + xto_string(location) + "', source_circuit = '" + xto_string(source_circuit) + "', target_circuit = '" + xto_string(target_circuit) + "' WHERE id = '" + xto_string(id) + "';";
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

[[nodiscard]] inline bool wire_save(Database &db, int amount_input, int amount_output, std::string /*PK*/ id, bool init_signal, std::string location, std::string source_circuit, std::string target_circuit)
{
	wire tmp;
	if (!get_one_wire(db, &tmp, id))
		return wire_insert(db, amount_input, amount_output, id, init_signal, location, source_circuit, target_circuit);
	return wire_update(db, amount_input, amount_output, id, init_signal, location, source_circuit, target_circuit);
}

#define SPREAD_WIRE(wire_struct) wire_struct.amount_input, wire_struct.amount_output, wire_struct.id, wire_struct.init_signal, wire_struct.location, wire_struct.source_circuit, wire_struct.target_circuit
#define SPREAD_WIRE_PTR(wire_struct) wire_struct->amount_input, wire_struct->amount_output, wire_struct->id, wire_struct->init_signal, wire_struct->location, wire_struct->source_circuit, wire_struct->target_circuit

//
// Custom Functions
//

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

// END GENERATED CODE
