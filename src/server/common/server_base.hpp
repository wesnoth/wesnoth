/*
   Copyright (C) 2016 - 2018 by Sergey Popov <dave@whitevine.net>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License 2
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Base class for servers using Wesnoth's WML over TCP protocol.
 */

#pragma once

#include "exceptions.hpp"
#include "server/common/simple_wml.hpp"

#include "utils/variant.hpp"
#include "utils/general.hpp"

#ifdef _WIN32
#include "serialization/unicode_cast.hpp"
#endif

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#ifndef _WIN32
#include <boost/asio/posix/stream_descriptor.hpp>
#endif
#include <boost/asio/signal_set.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/shared_array.hpp>

#include <map>

extern bool dump_wml;

class config;

typedef std::shared_ptr<boost::asio::ip::tcp::socket> socket_ptr;
typedef std::shared_ptr<boost::asio::ssl::stream<socket_ptr::element_type>> tls_socket_ptr;
typedef utils::variant<socket_ptr, tls_socket_ptr> any_socket_ptr;

struct server_shutdown : public game::error
{
	server_shutdown(const std::string& msg) : game::error(msg) {}
};

/**
 * Base class for implementing servers that use gzipped-WML network protocol
 *
 * The protocol is based on TCP connection between client and server.
 * Before WML payloads can be sent a handshake is required. Handshake process is as follows:
 * - client establishes a TCP connection to server.
 * - client sends 32-bit integer(network byte order) representing protocol version requested.
 * - server receives 32-bit integer. Depending on number received server does the following:
 *   0: unencrypted protocol, send unspecified 32-bit integer for compatibility(current implementation sends 42)
 *   1: depending on whether TLS is enabled on server
 *     if TLS enabled: send 32-bit integer 0 and immediately start TLS, client is expected to start TLS on receiving this 0
 *     if TLS disabled: send 32-bit integer 0xFFFFFFFF, on receiving this client should proceed as with unencrypted connection or immediately close
 *   any other number: server closes connection immediately
 * - at this point handshake is completed and client and server can exchange WML messages
 *
 * Message format is as follows:
 * - 32-bit unsigned integer(network byte order), this is size of the following payload
 * - payload: gzipped WML data, which is WML text fed through gzip utility or the equivalent library function.
 */
class server_base
{
public:
	server_base(unsigned short port, bool keep_alive);
	virtual ~server_base() {}
	void run();

	/**
	 * Send a WML document from within a coroutine
	 * @param socket
	 * @param doc
	 * @param yield The function will suspend on write operation using this yield context
	 */
	template<class SocketPtr> void coro_send_doc(SocketPtr socket, simple_wml::document& doc, boost::asio::yield_context yield);
	/**
	 * Send contents of entire file directly to socket from within a coroutine
	 * @param socket
	 * @param filename
	 * @param yield The function will suspend on write operations using this yield context
	 */
	void coro_send_file(socket_ptr socket, const std::string& filename, boost::asio::yield_context yield);
	void coro_send_file(tls_socket_ptr socket, const std::string& filename, boost::asio::yield_context yield);
	/**
	 * Receive WML document from a coroutine
	 * @param socket
	 * @param yield The function will suspend on read operation using this yield context
	 */
	template<class SocketPtr> std::unique_ptr<simple_wml::document> coro_receive_doc(SocketPtr socket, boost::asio::yield_context yield);

	/**
	 * High level wrapper for sending a WML document
	 *
	 * This function returns before send is finished. This function can be called again on same socket before previous send was finished.
	 * WML documents are kept in internal queue and sent in FIFO order.
	 * @param socket
	 * @param doc Document to send. A copy of it will be made so there is no need to keep the reference live after the function returns.
	 */
	template<class SocketPtr> void async_send_doc_queued(SocketPtr socket, simple_wml::document& doc);

	typedef std::map<std::string, std::string> info_table;
	template<class SocketPtr> void async_send_error(SocketPtr socket, const std::string& msg, const char* error_code = "", const info_table& info = {});
	template<class SocketPtr> void async_send_warning(SocketPtr socket, const std::string& msg, const char* warning_code = "", const info_table& info = {});

	/**
	 * Create the poor security nonce for use with passwords still hashed with MD5.
	 * Uses 8 random integer digits, 29.8 bits entropy.
	 *
	 * @param length How many random numbers to generate.
	 * @return The nonce to use.
	 */
	std::string create_unsecure_nonce(int length = 8);
	/**
	 * Create a good security nonce for use with bcrypt/crypt_blowfish hashing.
	 * Uses 32 random Base64 characters, cryptographic-strength, 192 bits entropy
	 *
	 * @return The nonce to use.
	 */
	std::string create_secure_nonce();
	/**
	 * Handles hashing the password provided by the player before comparing it to the hashed password in the forum database.
	 *
	 * @param pw The plaintext password.
	 * @param salt The salt as retrieved from the forum database.
	 * @param username The player attempting to log in.
	 * @return The hashed password, or empty if the password couldn't be hashed.
	 */
	std::string hash_password(const std::string& pw, const std::string& salt, const std::string& username);

protected:
	unsigned short port_;
	bool keep_alive_;
	boost::asio::io_service io_service_;
	boost::asio::ssl::context tls_context_ { boost::asio::ssl::context::sslv23 };
	bool tls_enabled_ { false };
	boost::asio::ip::tcp::acceptor acceptor_v6_;
	boost::asio::ip::tcp::acceptor acceptor_v4_;

	void load_tls_config(const config& cfg);

	void start_server();
	void serve(boost::asio::yield_context yield, boost::asio::ip::tcp::acceptor& acceptor, boost::asio::ip::tcp::endpoint endpoint);

	uint32_t handshake_response_;

	virtual void handle_new_client(socket_ptr socket) = 0;
	virtual void handle_new_client(tls_socket_ptr socket) = 0;

	virtual bool accepting_connections() const { return true; }
	virtual std::string is_ip_banned(const std::string&) { return std::string(); }
	virtual bool ip_exceeds_connection_limit(const std::string&) const { return false; }

#ifndef _WIN32
	boost::asio::posix::stream_descriptor input_;
	std::string fifo_path_;
	void read_from_fifo();
	virtual void handle_read_from_fifo(const boost::system::error_code& error, std::size_t bytes_transferred) = 0;
	boost::asio::streambuf admin_cmd_;

	boost::asio::signal_set sighup_;
	virtual void handle_sighup(const boost::system::error_code& error, int signal_number) = 0;
#endif
	boost::asio::signal_set sigs_;
	void handle_termination(const boost::system::error_code& error, int signal_number);
};

template<class SocketPtr> std::string client_address(SocketPtr socket);
template<class SocketPtr> std::string log_address(SocketPtr socket) { return (utils::decayed_is_same<tls_socket_ptr, decltype(socket)> ? "+" : "") + client_address(socket); }
template<class SocketPtr> bool check_error(const boost::system::error_code& error, SocketPtr socket);
