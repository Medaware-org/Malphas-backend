/**
 * This file was created by the Malphas DAO generator
 */

#pragma once

#include <string>
#include <any>

struct user {
	std::string id;
	std::string nickname;
	std::string passwd_hash;
};

struct session {
	std::string user_id;
	std::string session_token;
};

