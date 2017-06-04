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
#include <memory>

#if defined(__APPLE__) && defined(__MACH__) && defined(__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__)
#define __IPHONEOS__
#endif

#ifndef __IPHONEOS__
#include <openssl/rc4.h>
#else
#include <CommonCrypto/CommonCryptor.h>
#endif

#ifdef _WIN32
#include <boost/range/iterator_range.hpp>
#include <windows.h>
#endif

static lg::log_domain log_config("config");
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
	size_t size() const
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
				auto temp = decrypt(credentials[0].key, build_key(server, login));
				return std::string(temp.begin(), temp.end());
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
		auto temp = decrypt(cred->key, build_key(server, login));
		return std::string(temp.begin(), temp.end());
	}

	void set_password(const std::string& server, const std::string& login, const std::string& key)
	{
		secure_buffer temp(key.begin(), key.end());
		if(!remember_password()) {
			clear_credentials();
			credentials.emplace_back(login, server, encrypt(temp, build_key(server, login)));
			return;
		}
		auto cred = std::find_if(credentials.begin(), credentials.end(), [&](const login_info& cred) {
			return cred.server == server && cred.username == login;
		});
		if(cred == credentials.end()) {
			// This is equivalent to emplace_back, but also returns the iterator to the new element
			cred = credentials.emplace(credentials.end(), login, server);
		}
		cred->key = encrypt(temp, build_key(server, login));
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
			size_t at = elem.find_last_of('@');
			size_t eq = elem.find_first_of('=', at + 1);
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
		secure_buffer credentials_data(1, CREDENTIAL_SEPARATOR);
		size_t offset = 1;
		for(const auto& cred : credentials) {
			credentials_data.resize(credentials_data.size() + cred.size(), CREDENTIAL_SEPARATOR);
			std::copy(cred.username.begin(), cred.username.end(), credentials_data.begin() + offset);
			offset += cred.username.size();
			credentials_data[offset++] = '@';
			std::copy(cred.server.begin(), cred.server.end(), credentials_data.begin() + offset);
			offset += cred.server.size();
			credentials_data[offset++] = '=';
			secure_buffer key_escaped = escape(cred.key);
			// Escaping may increase the length, so resize again if so
			credentials_data.resize(credentials_data.size() + key_escaped.size() - cred.key.size());
			std::copy(key_escaped.begin(), key_escaped.end(), credentials_data.begin() + offset);
			offset += key_escaped.size() + 1;
		}
		try {
			filesystem::scoped_ostream credentials_file = filesystem::ostream_file(filesystem::get_credentials_file());
			secure_buffer encrypted = encrypt(credentials_data, build_key("global", get_system_username()));
			credentials_file->write(reinterpret_cast<const char*>(encrypted.data()), encrypted.size());
		} catch(filesystem::io_exception&) {
			ERR_CFG << "error writing to credentials file '" << filesystem::get_credentials_file() << "'" << std::endl;
		}
	}
}

// TODO: Key-stretching (bcrypt was recommended)
secure_buffer build_key(const std::string& server, const std::string& login)
{
	std::string sysname = get_system_username();
	secure_buffer result(std::max<size_t>(server.size() + login.size() + sysname.size(), 32));
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
#ifndef __IPHONEOS__
	RC4_KEY cipher_key;
	RC4_set_key(&cipher_key, key.size(), key.data());
	const size_t block_size = key.size();
	const size_t blocks = text.size() / block_size;
	const size_t extra = text.size() % block_size;
	for(size_t i = 0; i < blocks * block_size; i += block_size) {
		RC4(&cipher_key, block_size, text.data() + i, result.data() + i);
	}
	if(extra) {
		size_t i = blocks * block_size;
		RC4(&cipher_key, extra, text.data() + i, result.data() + i);
	}
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
	return rc4_crypt(text, key);
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
