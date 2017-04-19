/*
Copyright (C) 2017 by the Battle for Wesnoth Project http://www.wesnoth.org/

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY.

See the COPYING file for more details.
*/

#include "credentials.hpp"

#include "preferences/general.hpp"
#include "serialization/unicode.hpp"
#include "filesystem.hpp"
#include "log.hpp"
#include "serialization/string_utils.hpp"

#include <algorithm>
#include <sstream>

#ifdef _WIN32
#include <boost/range/iterator_range.hpp>
#include <windows.h>
#endif

static lg::log_domain log_config("config");
#define ERR_CFG LOG_STREAM(err , log_config)

struct login_info
{
	std::string username, server, key;
	login_info(const std::string& username, const std::string& server, const std::string& key)
		: username(username), server(server), key(key)
	{}
};

static std::vector<login_info> credentials;

// Separate password entries with formfeed
static const char CREDENTIAL_SEPARATOR = '\f';

static std::string encrypt(const std::string& text, const std::string& key);
static std::string decrypt(const std::string& text, const std::string& key);
static std::string build_key(const std::string& server, const std::string& login);
static std::string escape(const std::string& text);
static std::string unescape(const std::string& text);

static std::string get_system_username()
{
	std::string res;
#ifdef _WIN32
	wchar_t buffer[300];
	DWORD size = 300;
	if(GetUserNameW(buffer, &size)) {
		//size includes a terminating null character.
		assert(size > 0);
		res = unicode_cast<utf8::string>(boost::iterator_range<wchar_t*>(buffer, buffer + size - 1));
	}
#else
	if(char* const login = getenv("USER")) {
		res = login;
	}
#endif
	return res;
}

static void clear_credentials()
{
	// Zero them before clearing.
	// Probably overly paranoid, but doesn't hurt?
	for(auto& cred : credentials) {
		std::fill(cred.username.begin(), cred.username.end(), '\0');
		std::fill(cred.server.begin(), cred.server.end(), '\0');
		std::fill(cred.key.begin(), cred.key.end(), '\0');
	}
	credentials.clear();
}

static const std::string EMPTY_LOGIN = "@@";

namespace preferences
{
	std::string login()
	{
		std::string name = preferences::get("login", EMPTY_LOGIN);
		if(name == EMPTY_LOGIN) {
			name = get_system_username();
		} else if(name.size() > 2 && name[0] == '@' && name[name.size() - 1] == '@') {
			name = name.substr(1, name.size() - 2);
		} else {
			ERR_CFG << "malformed user credentials (did you manually edit the preferences file?)" << std::endl;
		}
		if(name.empty()) {
			return "player";
		}
		return name;
	}

	void set_login(const std::string& login)
	{
		preferences::set("login", '@' + login + '@');
	}

	bool remember_password()
	{
		return preferences::get("remember_password", false);
	}

	void set_remember_password(bool remember)
	{
		preferences::set("remember_password", remember);

		if(remember) {
			load_credentials();
		} else {
			clear_credentials();
		}
	}

	std::string password(const std::string& server, const std::string& login)
	{
		if(!remember_password()) {
			if(!credentials.empty() && credentials[0].username == login && credentials[0].server == server) {
				return decrypt(credentials[0].key, build_key(server, login));
			} else {
				return "";
			}
		}
		auto cred = std::find_if(credentials.begin(), credentials.end(), [&](const login_info& cred) {
			return cred.server == server && cred.username == login;
		});
		if(cred == credentials.end()) {
			return "";
		}
		return decrypt(unescape(cred->key), build_key(server, login));
	}

	void set_password(const std::string& server, const std::string& login, const std::string& key)
	{
		if(!remember_password()) {
			clear_credentials();
			credentials.emplace_back(login, server, encrypt(key, build_key(server, login)));
			return;
		}
		auto cred = std::find_if(credentials.begin(), credentials.end(), [&](const login_info& cred) {
			return cred.server == server && cred.username == login;
		});
		if(cred == credentials.end()) {
			// This is equivalent to emplace_back, but also returns the iterator to the new element
			cred = credentials.emplace(credentials.end(), login, server, "");
		}
		cred->key = escape(encrypt(key, build_key(server, login)));
	}

	void load_credentials()
	{
		if(!remember_password()) {
			return;
		}
		clear_credentials();
		std::string cred_file = filesystem::get_credentials_file();
		if(!filesystem::file_exists(cred_file)) {
			return;
		}
		filesystem::scoped_istream stream = filesystem::istream_file(cred_file, false);
		// Credentials file is a binary blob, so use streambuf iterator
		std::string data((std::istreambuf_iterator<char>(*stream)), (std::istreambuf_iterator<char>()));
		data = decrypt(data, build_key("global", get_system_username()));
		if(data.empty() || data[0] != CREDENTIAL_SEPARATOR) {
			ERR_CFG << "Invalid data in credentials file\n";
			std::fill(data.begin(), data.end(), '\0');
			return;
		}
		for(const std::string elem : utils::split(data, CREDENTIAL_SEPARATOR, utils::REMOVE_EMPTY)) {
			size_t at = elem.find_last_of('@');
			size_t eq = elem.find_first_of('=', at + 1);
			if(at != std::string::npos && eq != std::string::npos) {
				credentials.emplace_back(elem.substr(0, at), elem.substr(at + 1, eq - at - 1), elem.substr(eq + 1));
			}
		}
		std::fill(data.begin(), data.end(), '\0');
	}

	void save_credentials()
	{
		if(!remember_password()) {
			filesystem::delete_file(filesystem::get_credentials_file());
			return;
		}
		std::ostringstream credentials_data;
		for(const auto& cred : credentials) {
			credentials_data.put(CREDENTIAL_SEPARATOR);
			std::copy(cred.username.begin(), cred.username.end(), std::ostreambuf_iterator<char>(credentials_data));
			credentials_data.put('@');
			std::copy(cred.server.begin(), cred.server.end(), std::ostreambuf_iterator<char>(credentials_data));
			credentials_data.put('=');
			std::copy(cred.key.begin(), cred.key.end(), std::ostreambuf_iterator<char>(credentials_data));
		}
		credentials_data.put(CREDENTIAL_SEPARATOR);
		try {
			filesystem::scoped_ostream credentials_file = filesystem::ostream_file(filesystem::get_credentials_file());
			std::string encrypted = encrypt(credentials_data.str(), build_key("global", get_system_username()));
			credentials_file->write(encrypted.c_str(), encrypted.size());
		} catch(filesystem::io_exception&) {
			ERR_CFG << "error writing to credentials file '" << filesystem::get_credentials_file() << "'" << std::endl;
		}
		size_t n = credentials_data.tellp();
		credentials_data.seekp(0, std::ios::beg);
		std::fill_n(std::ostreambuf_iterator<char>(credentials_data), n, '\0');
	}
}

// TODO: Key-stretching (bcrypt was recommended)
std::string build_key(const std::string& server, const std::string& login)
{
	std::ostringstream out;
	out << login << get_system_username() << server;
	return out.str();
}

// FIXME: XOR encryption is a really terrible choice - swap it out for something better!
// TODO: Maybe use cryptopp or something for AES encryption?
static std::string xor_crypt(std::string text, const std::string& key)
{
	const size_t m = key.size();
	for(size_t i = 0; i < text.size(); i++) {
		text[i] ^= key[i % m];
	}
	return text;
}

std::string encrypt(const std::string& text, const std::string& key)
{
	return xor_crypt(text, key);
}

std::string decrypt(const std::string& text, const std::string& key)
{
	return xor_crypt(text, key);
}

std::string unescape(const std::string& text)
{
	std::string unescaped;
	unescaped.reserve(text.size());
	bool escaping = false;
	for(char c : text) {
		if(escaping) {
			if(c == '\xa') {
				unescaped.push_back('\xc');
			} else if(c == '.') {
				unescaped.push_back('@');
			} else {
				unescaped.push_back(c);
			}
			escaping = false;
		} else if(c == '\x1') {
			escaping = true;
		} else {
			unescaped.push_back(c);
		}
	}
	assert(!escaping);
	return unescaped;
}

std::string escape(const std::string& text)
{
	std::string escaped;
	escaped.reserve(text.size());
	for(char c : text) {
		if(c == '\x1') {
			escaped += "\x1\x1";
		} else if(c == '\xc') {
			escaped += "\x1\xa";
		} else if(c == '@') {
			escaped += "\x1.";
		} else {
			escaped.push_back(c);
		}
	}
	return escaped;
}
