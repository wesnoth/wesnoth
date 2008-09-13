#ifdef HAVE_MYSQLPP

#include "forum_user_handler.hpp"

#include <stdlib.h> 
#include <sstream>

fuh::fuh(config c) {
	db_name_ = c["db_name"];
	db_host_ = c["db_host"];
	db_user_ = c["db_user"];
	db_password_ = c["db_password"];
	db_users_table_ = c["db_users_table"];

	// Connect to the database
	try {
		db_interface_.connect(db_name_.c_str(), db_host_.c_str(), db_user_.c_str(), db_password_.c_str());
	} catch(...) {
		 ERR_UH << "Could not connect to database: " << db_interface_.error() << std::endl;
	}
}

std::string fuh::get_detail_for_user(const std::string& name, const std::string& detail) {
	return std::string("SELECT " + detail + " FROM " + db_users_table_ + " WHERE username='" + name + "'");
}

std::string fuh::set_detail_for_user(const std::string& name, const std::string& detail, const std::string& value) {
	return std::string("UPDATE " + db_users_table_ + " SET " + detail + "='" + value + "' WHERE username='" + name + "'");
}

void fuh::add_user(const std::string& name, const std::string& mail, const std::string& password) {
	throw error("For now please register at http://forum.wesnoth.org");
}

void fuh::remove_user(const std::string& name) {
	throw error("'Dropping your nick is currently impossible");
}

void fuh::set_user_detail(const std::string& user, const std::string& detail, const std::string& value) {
	throw error("For now this is a 'read-only' user_handler");
}

std::string fuh::get_valid_details() {
	return "For now this is a 'read-only' user_handler";
}

mysqlpp::Result fuh::db_query(const std::string& sql) {

	//Check if we are connected
	if(!(db_interface_.connected())) {
		WRN_UH << "not connected to database, reconnecting..." << std::endl;
		//Try to reconnect
		try {
			db_interface_.connect(db_name_.c_str(), db_host_.c_str(), db_user_.c_str(), db_password_.c_str());
		} catch(...) {
			 ERR_UH << "Could not connect to database: " << db_interface_.error() << std::endl;
		}
	}

	mysqlpp::Query query = db_interface_.query();
	query << sql;
	return query.store();
}

std::string fuh::db_query_to_string(const std::string& query) {
	return std::string(db_query(query).at(0).at(0));
}

// The hashing code is basically taken from forum_auth.cpp
bool fuh::login(const std::string& name, const std::string& password, const std::string& seed) {

	// Set an alphabet-like string for use in encrytpion algorithm	 
	std::string itoa64("./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

	// Retrieve users' password as hash

	std::string hash;

	try {
		hash = get_hash(name);
	} catch (error e) {
		ERR_UH << "Could not retrieve hash for user '" << name << "' :" << e.message << std::endl;
		return false;
	}

	// Check hash prefix, if different than $H$ hash is invalid
	if(hash.substr(0,3) != "$H$") {
		ERR_UH << "Invalid hash prefix for user '" << name << "'" << std::endl;
		return false;
	}

	std::string valid_hash = hash.substr(12,34) + seed;
	MD5 md5_worker;
	md5_worker.update((unsigned char *)valid_hash.c_str(), valid_hash.size());
	md5_worker.finalize();
	valid_hash = std::string(md5_worker.hex_digest());

	if(password == valid_hash) return true;

	return false;
}

std::string fuh::create_pepper(const std::string& name, int index) {

	// Some doulbe security, this should never be neeeded
	if(!(user_exists(name))) {
		return "";
	}

	// Set an alphabet-like string for use in encrytpion algorithm	 
	std::string itoa64("./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

	std::string hash;

	try {
		hash = get_hash(name);
	} catch (error e) {
		ERR_UH << "Could not retrieve hash for user '" << name << "' :" << e.message << std::endl;
		return "";
	}

	// Check hash prefix, if different than $H$ hash is invalid
	if(hash.substr(0,3) != "$H$")
		return "";

	if(index == 0) {
		// Start of the encryption, get the position of first nonidentifier character in extended alphabet
		int hash_seed = itoa64.find_first_of(hash[3]);

		// If position is lower than 8 or higher than 32 hash is also invalid
		if(hash_seed < 7 || hash_seed > 30)
			return "";

		// Set the number of encryption passes as 2^position
		hash_seed = 1 << hash_seed;

		std::stringstream ss;
		ss << hash_seed;
		return ss.str();

	} else if (index == 1) {
		// Create salt for mixing with the hash
		return hash.substr(4,8);

	} else {
		return "";
	}

}

bool fuh::user_exists(const std::string& name) {

	// Make a test query for this username
	try {
		return db_query(get_detail_for_user(name, "username")).num_rows() > 0;
	} catch (error e) {
		ERR_UH << "Could not execute test query for user '" << e.message << std::endl;
		// If the database is down just let all usernames log in
		return false;
	}
}

void fuh::user_logged_in(const std::string& name) {
	set_lastlogin(name, time(NULL));
}


void fuh::clean_up() {

}

void fuh::set_lastlogin(const std::string& user, const time_t& lastlogin) {

	std::stringstream ss;
	ss << lastlogin;

	try {
	db_query(set_detail_for_user(user, "user_lastvisit", ss.str()));
	} catch (error e) {
		ERR_UH << "Could not set last visit for user '" << e.message << std::endl;
	}
}

std::string fuh::get_hash(const std::string& user) {
	try {
		return db_query_to_string(get_detail_for_user(user, "user_password"));
	} catch (error e) {
		ERR_UH << "Could not retrieve password for user '" << e.message << std::endl;
		return time_t(0);
	}
}

std::string fuh::get_mail(const std::string& user) {
	try {
		return db_query_to_string(get_detail_for_user(user, "user_email"));
	} catch (error e) {
		ERR_UH << "Could not retrieve email for user '" << e.message << std::endl;
		return time_t(0);
	}
}

/*
std::vector<std::string> fuh::get_friends(const std::string& user) {
	std::string sql("SELECT user_id FROM phpbb_users WHERE username='");
	sql.append(user);
	sql.append("'");

	std::string id = db_query_to_string(sql);

	sql = "SELECT zebra_id, friend FROM phpbb_zebra WHERE user_id='";
	sql.append(id);
	sql.append("'");
	
	mysqlpp::StoreQueryResult sqr = db_query(sql);

	std::vector<std::string> friends;

	for(int i = 0; i < sqr.num_rows(); i++) {
		if(std::string(sqr[i][1]) == "1") {
			sql = "SELECT username FROM phpbb_users WHERE user_id='";
			sql.append(std::string(sqr[i][0]));
			sql.append("'");
			friends.push_back(db_query_to_string(sql));
		}
	}

	return friends;
}

std::vector<std::string> fuh::get_ignores(const std::string& user) {
	std::string sql("SELECT user_id FROM phpbb_users WHERE username='");
	sql.append(user);
	sql.append("'");

	std::string id = db_query_to_string(sql);

	sql = "SELECT zebra_id, friend FROM phpbb_zebra WHERE user_id='";
	sql.append(id);
	sql.append("'");
	
	mysqlpp::StoreQueryResult sqr = db_query(sql);

	std::vector<std::string> ignores;

	for(int i = 0; i < sqr.num_rows(); i++) {
		if(std::string(sqr[i][1]) == "0") {
			sql = "SELECT username FROM phpbb_users WHERE user_id='";
			sql.append(std::string(sqr[i][0]));
			sql.append("'");
			ignores.push_back(db_query_to_string(sql));
		}
	}

	return ignores;
}
*/

time_t fuh::get_lastlogin(const std::string& user) {
	try {
		int time_int = atoi(db_query_to_string(get_detail_for_user(user, "user_lastvisit")).c_str());
		return time_t(time_int);
	} catch (error e) {
		ERR_UH << "Could not retrieve last visit for user '" << e.message << std::endl;
		return time_t(0);
	}
}

time_t fuh::get_registrationdate(const std::string& user) {
	try {
		int time_int = atoi(db_query_to_string(get_detail_for_user(user, "user_regdate")).c_str());
		return time_t(time_int);
	} catch (error e) {
		ERR_UH << "Could not retrieve registration date for user '" << e.message << std::endl;
		return time_t(0);
	}
}

void fuh::password_reminder(const std::string& name) {
	throw error("For now please use the password recovery"
		"function provided at http://forum.wesnoth.org");
}

std::string fuh::user_info(const std::string& name) {
	if(!user_exists(name)) {
		throw error("No user with the name '" + name + "' exists.");
	}

	time_t reg_date = get_registrationdate(name);
	time_t ll_date = get_lastlogin(name);

	std::string reg_string = ctime(&reg_date);
	std::string ll_string = ctime(&ll_date);

	std::stringstream info;
	info << "Name: " << name << "\n"
		 << "Registered: " << reg_string
		 << "Last login: " << ll_string;

	return info.str();
}

#endif //HAVE_MYSQLPP
