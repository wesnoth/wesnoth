/*
	Copyright (C) 2017 - 2022
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

#include <boost/algorithm/string.hpp>

#include <algorithm>
#include <memory>

#ifndef __APPLE__
#include <openssl/evp.h>
#else
#include <CommonCrypto/CommonCryptor.h>
#endif

#ifdef _WIN32
#include <boost/range/iterator_range.hpp>
#include <windows.h>
#endif

static lg::log_domain log_config("config");
#define DBG_CFG LOG_STREAM(debug , log_config)
#define ERR_CFG LOG_STREAM(err , log_config)

class secure_buffer : public std::vector<unsigned char>
{
public:
	template<typename... T> secure_buffer(T&&... args)
		: vector<unsigned char>(std::forward<T>(args)...)
	{}
	~secure_buffer()
	{
		std::fill(begin(), end(), '\0');
	}
};

struct login_info
{
	std::string username, server;
	secure_buffer key;
	login_info(const std::string& username, const std::string& server, const secure_buffer& key)
		: username(username), server(server), key(key)
	{}
	login_info(const std::string& username, const std::string& server)
		: username(username), server(server), key()
	{}
	std::size_t size() const
	{
		return 3 + username.size() + server.size() + key.size();
	}
};

static std::vector<login_info> credentials;

// Separate password entries with formfeed
static const unsigned char CREDENTIAL_SEPARATOR = '\f';

static secure_buffer encrypt(const secure_buffer& text, const secure_buffer& key);
static secure_buffer decrypt(const secure_buffer& text, const secure_buffer& key);
static secure_buffer build_key(const std::string& server, const std::string& login);
static secure_buffer escape(const secure_buffer& text);
static secure_buffer unescape(const secure_buffer& text);

static std::string get_system_username()
{
	std::string res;
#ifdef _WIN32
	wchar_t buffer[300];
	DWORD size = 300;
	if(GetUserNameW(buffer, &size)) {
		//size includes a terminating null character.
		assert(size > 0);
		res = unicode_cast<std::string>(boost::iterator_range<wchar_t*>(buffer, buffer + size - 1));
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
		} else if(name.size() > 2 && name.front() == '@' && name.back() == '@') {
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
		auto login_clean = login;
		boost::trim(login_clean);

		preferences::set("login", '@' + login_clean + '@');
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
		DBG_CFG << "Retrieving password for server: '" << server << "', login: '" << login << "'\n";
		auto login_clean = login;
		boost::trim(login_clean);

		if(!remember_password()) {
			if(!credentials.empty() && credentials[0].username == login_clean && credentials[0].server == server) {
				auto temp = decrypt(credentials[0].key, build_key(server, login_clean));
				return std::string(temp.begin(), temp.end());
			} else {
				return "";
			}
		}
		auto cred = std::find_if(credentials.begin(), credentials.end(), [&](const login_info& cred) {
			return cred.server == server && cred.username == login_clean;
		});
		if(cred == credentials.end()) {
			return "";
		}
		auto temp = decrypt(cred->key, build_key(server, login_clean));
		return std::string(temp.begin(), temp.end());
	}

	void set_password(const std::string& server, const std::string& login, const std::string& key)
	{
		DBG_CFG << "Setting password for server: '" << server << "', login: '" << login << "'\n";
		auto login_clean = login;
		boost::trim(login_clean);

		secure_buffer temp(key.begin(), key.end());
		if(!remember_password()) {
			clear_credentials();
			credentials.emplace_back(login_clean, server, encrypt(temp, build_key(server, login_clean)));
			return;
		}
		auto cred = std::find_if(credentials.begin(), credentials.end(), [&](const login_info& cred) {
			return cred.server == server && cred.username == login_clean;
		});
		if(cred == credentials.end()) {
			// This is equivalent to emplace_back, but also returns the iterator to the new element
			cred = credentials.emplace(credentials.end(), login_clean, server);
		}
		cred->key = encrypt(temp, build_key(server, login_clean));
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
		secure_buffer data((std::istreambuf_iterator<char>(*stream)), (std::istreambuf_iterator<char>()));
		data = decrypt(data, build_key("global", get_system_username()));
		if(data.empty() || data[0] != CREDENTIAL_SEPARATOR) {
			ERR_CFG << "Invalid data in credentials file\n";
			return;
		}
		for(const std::string& elem : utils::split(std::string(data.begin(), data.end()), CREDENTIAL_SEPARATOR, utils::REMOVE_EMPTY)) {
			std::size_t at = elem.find_last_of('@');
			std::size_t eq = elem.find_first_of('=', at + 1);
			if(at != std::string::npos && eq != std::string::npos) {
				secure_buffer key(elem.begin() + eq + 1, elem.end());
				credentials.emplace_back(elem.substr(0, at), elem.substr(at + 1, eq - at - 1), unescape(key));
			}
		}
	}

	void save_credentials()
	{
		if(!remember_password()) {
			filesystem::delete_file(filesystem::get_credentials_file());
			return;
		}
		secure_buffer credentials_data;
		for(const auto& cred : credentials) {
			credentials_data.push_back(CREDENTIAL_SEPARATOR);
			credentials_data.insert(credentials_data.end(), cred.username.begin(), cred.username.end());
			credentials_data.push_back('@');
			credentials_data.insert(credentials_data.end(), cred.server.begin(), cred.server.end());
			credentials_data.push_back('=');
			secure_buffer key_escaped = escape(cred.key);
			credentials_data.insert(credentials_data.end(), key_escaped.begin(), key_escaped.end());
		}
		try {
			filesystem::scoped_ostream credentials_file = filesystem::ostream_file(filesystem::get_credentials_file());
			secure_buffer encrypted = encrypt(credentials_data, build_key("global", get_system_username()));
			credentials_file->write(reinterpret_cast<const char*>(encrypted.data()), encrypted.size());
		} catch(const filesystem::io_exception&) {
			ERR_CFG << "error writing to credentials file '" << filesystem::get_credentials_file() << "'" << std::endl;
		}
	}
}

// TODO: Key-stretching (bcrypt was recommended)
secure_buffer build_key(const std::string& server, const std::string& login)
{
	std::string sysname = get_system_username();
	secure_buffer result(std::max<std::size_t>(server.size() + login.size() + sysname.size(), 32));
	unsigned char i = 0;
	std::generate(result.begin(), result.end(), [&i]() {return 'x' ^ i++;});
	std::copy(login.begin(), login.end(), result.begin());
	std::copy(sysname.begin(), sysname.end(), result.begin() + login.size());
	std::copy(server.begin(), server.end(), result.begin() + login.size() + sysname.size());
	return result;
}

static secure_buffer rc4_crypt(const secure_buffer& text, const secure_buffer& key)
{
	secure_buffer result(text.size(), '\0');
#ifndef __APPLE__
	int outlen;
	int tmplen;

	EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();

	// TODO: use EVP_EncryptInit_ex2 once openssl 3.0 is more widespread
	EVP_EncryptInit_ex(ctx, EVP_rc4(), NULL, key.data(), NULL);
	EVP_EncryptUpdate(ctx, result.data(), &outlen, text.data(), text.size());
	EVP_EncryptFinal_ex(ctx, result.data() + outlen, &tmplen);

	EVP_CIPHER_CTX_free(ctx);
#else
	size_t outWritten = 0;
	CCCryptorStatus ccStatus = CCCrypt(kCCDecrypt,
		kCCAlgorithmRC4,
		kCCOptionPKCS7Padding,
		key.data(),
		key.size(),
		nullptr,
		text.data(),
		text.size(),
		result.data(),
		result.size(),
		&outWritten);

	assert(ccStatus == kCCSuccess);
	assert(outWritten == text.size());
#endif
	return result;
}

secure_buffer encrypt(const secure_buffer& text, const secure_buffer& key)
{
	return rc4_crypt(text, key);
}

secure_buffer decrypt(const secure_buffer& text, const secure_buffer& key)
{
	auto buf = rc4_crypt(text, key);
	while(!buf.empty() && buf.back() == 0) {
		buf.pop_back();
	}
	return buf;
}

secure_buffer unescape(const secure_buffer& text)
{
	secure_buffer unescaped;
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

secure_buffer escape(const secure_buffer& text)
{
	secure_buffer escaped;
	escaped.reserve(text.size());
	for(char c : text) {
		if(c == '\x1') {
			escaped.push_back('\x1');
			escaped.push_back('\x1');
		} else if(c == '\xc') {
			escaped.push_back('\x1');
			escaped.push_back('\xa');
		} else if(c == '@') {
			escaped.push_back('\x1');
			escaped.push_back('.');
		} else {
			escaped.push_back(c);
		}
	}
	return escaped;
}
