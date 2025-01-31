/*
	Copyright (C) 2016 - 2024
	by Sergey Popov <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "server/common/server_base.hpp"

#include "config.hpp"
#include "hash.hpp"
#include "log.hpp"
#include "filesystem.hpp"
#include "utils/scope_exit.hpp"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_SENDFILE
#include <sys/sendfile.h>
#endif

#ifdef _WIN32
#include <windows.h>
#include <boost/scope_exit.hpp>
#endif

#include <boost/asio/ip/v6_only.hpp>
#include <boost/asio/read.hpp>
#ifndef _WIN32
#include <boost/asio/read_until.hpp>
#endif
#ifndef BOOST_NO_EXCEPTIONS
#include <boost/exception/diagnostic_information.hpp>
#endif

#include <iostream>
#include <queue>
#include <string>
#include <utility>


static lg::log_domain log_server("server");
#define ERR_SERVER LOG_STREAM(err, log_server)
#define WRN_SERVER LOG_STREAM(warn, log_server)
#define LOG_SERVER LOG_STREAM(info, log_server)
#define DBG_SERVER LOG_STREAM(debug, log_server)

static lg::log_domain log_config("config");
#define ERR_CONFIG LOG_STREAM(err, log_config)
#define WRN_CONFIG LOG_STREAM(warn, log_config)

bool dump_wml = false;

server_base::server_base(unsigned short port, bool keep_alive)
	: port_(port)
	, keep_alive_(keep_alive)
	, io_service_()
	, acceptor_v6_(io_service_)
	, acceptor_v4_(io_service_)
	, handshake_response_()
#ifndef _WIN32
	, input_(io_service_)
	, sighup_(io_service_, SIGHUP)
#endif
{
}

void server_base::start_server()
{
	boost::asio::ip::tcp::endpoint endpoint_v6(boost::asio::ip::tcp::v6(), port_);
	boost::asio::spawn(io_service_, [this, endpoint_v6](const boost::asio::yield_context& yield) { serve(yield, acceptor_v6_, endpoint_v6); }
#if BOOST_VERSION >= 108000
		, [](const std::exception_ptr& e) { if (e) std::rethrow_exception(e); }
#endif
	);

	boost::asio::ip::tcp::endpoint endpoint_v4(boost::asio::ip::tcp::v4(), port_);
	boost::asio::spawn(io_service_, [this, endpoint_v4](const boost::asio::yield_context& yield) { serve(yield, acceptor_v4_, endpoint_v4); }
#if BOOST_VERSION >= 108000
		, [](const std::exception_ptr& e) { if (e) std::rethrow_exception(e); }
#endif
	);

	handshake_response_ = htonl(42);

#ifndef _WIN32
	sighup_.async_wait(
		[this](const boost::system::error_code& error, int sig)
			{ this->handle_sighup(error, sig); });
#endif
}

void server_base::serve(const boost::asio::yield_context& yield, boost::asio::ip::tcp::acceptor& acceptor, const boost::asio::ip::tcp::endpoint& endpoint)
{
	try {
		if(!acceptor.is_open()) {
			acceptor.open(endpoint.protocol());
			acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
			acceptor.set_option(boost::asio::ip::tcp::acceptor::keep_alive(keep_alive_));
			if(endpoint.protocol() == boost::asio::ip::tcp::v6())
				acceptor.set_option(boost::asio::ip::v6_only(true));
			acceptor.bind(endpoint);
			acceptor.listen();
		}
	} catch(const boost::system::system_error& e) {
		ERR_SERVER << "Exception when trying to bind port: " << e.code().message();
		BOOST_THROW_EXCEPTION(server_shutdown("Port binding failed", e.code()));
	}

	socket_ptr socket = std::make_shared<socket_ptr::element_type>(io_service_);

	boost::system::error_code error;
	acceptor.async_accept(socket->lowest_layer(), yield[error]);
	if(error && accepting_connections()) {
		ERR_SERVER << "Accept failed: " << error.message();
		BOOST_THROW_EXCEPTION(server_shutdown("Accept failed", error));
	}

	if(accepting_connections()) {
		boost::asio::spawn(io_service_, [this, &acceptor, endpoint](const boost::asio::yield_context& yield) { serve(yield, acceptor, endpoint); }
#if BOOST_VERSION >= 108000
			, [](const std::exception_ptr& e) { if (e) std::rethrow_exception(e); }
#endif
		);
	} else {
		return;
	}

	if(keep_alive_) {
		int timeout = 30;
#ifdef __linux__
		int cnt = 10;
		int interval = 30;
		setsockopt(socket->native_handle(), SOL_TCP, TCP_KEEPIDLE, &timeout, sizeof(timeout));
		setsockopt(socket->native_handle(), SOL_TCP, TCP_KEEPCNT, &cnt, sizeof(cnt));
		setsockopt(socket->native_handle(), SOL_TCP, TCP_KEEPINTVL, &interval, sizeof(interval));
#elif defined(__APPLE__) && defined(__MACH__)
		setsockopt(socket->native_handle(), IPPROTO_TCP, TCP_KEEPALIVE, &timeout, sizeof(timeout));
#elif defined(_WIN32)
		// these are in milliseconds for windows
		DWORD timeout_ms = timeout * 1000;
		setsockopt(socket->native_handle(), SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout_ms), sizeof(timeout_ms));
		setsockopt(socket->native_handle(), SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&timeout_ms), sizeof(timeout_ms));
#endif
	}

#ifdef __linux__
	fcntl(socket->native_handle(), F_SETFD, FD_CLOEXEC);
#endif

	DBG_SERVER << client_address(socket) << "\tnew connection tentatively accepted";

	uint32_t protocol_version;
	uint32_t handshake_response;

	any_socket_ptr final_socket;

	async_read(*socket, boost::asio::buffer(reinterpret_cast<std::byte*>(&protocol_version), 4), yield[error]);
	if(check_error(error, socket))
		return;

	switch(ntohl(protocol_version)) {
		case 0:
			async_write(*socket, boost::asio::buffer(reinterpret_cast<std::byte*>(&handshake_response_), 4), yield[error]);
			if(check_error(error, socket)) return;
			final_socket = socket;
			break;
		case 1:
			if(!tls_enabled_) {
				ERR_SERVER << client_address(socket) << "\tTLS requested by client but not enabled on server";
				handshake_response = 0xFFFFFFFFU;
			} else {
				handshake_response = 0x00000000;
			}

			async_write(*socket, boost::asio::buffer(reinterpret_cast<const std::byte*>(&handshake_response), 4), yield[error]);
			if(check_error(error, socket)) return;
			if(!tls_enabled_) { // continue with unencrypted connection if TLS disabled
				final_socket = socket;
				break;
			}

			final_socket = tls_socket_ptr { new tls_socket_ptr::element_type(std::move(*socket), tls_context_) };
			utils::get<tls_socket_ptr>(final_socket)->async_handshake(boost::asio::ssl::stream_base::server, yield[error]);
			if(error) {
				ERR_SERVER << "TLS handshake failed: " << error.message();
				return;
			}

			break;
		default:
			ERR_SERVER << client_address(socket) << "\tincorrect handshake";
			return;
	}

	utils::visit(
		[this](auto&& socket) {
			const std::string ip = client_address(socket);

			if(const auto ban_info = is_ip_banned(ip)) {
				const auto& [error_code, reason, time_remaining] = *ban_info;
				LOG_SERVER << log_address(socket) << "\trejected banned user. Reason: " << reason;

				if(time_remaining) {
					// Temporary ban
					async_send_error(socket, "You are banned from this server: " + reason, error_code,
						{{ "duration", std::to_string(time_remaining->count()) }});
				} else {
					// Permanent ban
					async_send_error(socket, "You are banned from this server: " + reason, error_code);
				}

				return;
			}

			if(ip_exceeds_connection_limit(ip)) {
				LOG_SERVER << log_address(socket) << "\trejected ip due to excessive connections";
				async_send_error(socket, "Too many connections from your IP.");
				return;
			}

			if constexpr(utils::decayed_is_same<tls_socket_ptr, decltype(socket)>) {
				DBG_SERVER << ip << "\tnew encrypted connection fully accepted";
			} else {
				DBG_SERVER << ip << "\tnew connection fully accepted";
			}
			this->handle_new_client(socket);
		},
		final_socket);
}

#ifndef _WIN32
void server_base::read_from_fifo() {
	async_read_until(input_,
					 admin_cmd_, '\n',
					 [this](const boost::system::error_code& error, std::size_t bytes_transferred)
						{ this->handle_read_from_fifo(error, bytes_transferred); }
	);
}
#endif

int server_base::run() {
	for(;;) {
		try {
			io_service_.run();
			LOG_SERVER << "Server has shut down because event loop is out of work";
			return 1;
		} catch(const server_shutdown& e) {
			LOG_SERVER << "Server has been shut down: " << e.what();
			return e.ec.value();
		} catch(const boost::system::system_error& e) {
			ERR_SERVER << "Caught system error exception from handler: " << e.code().message();
		} catch(const std::exception& e) {
			ERR_SERVER << "Caught exception from handler: " << e.what() << "\n" << boost::current_exception_diagnostic_information();
		}
	}
}

template<class SocketPtr> std::string client_address(SocketPtr socket)
{
	boost::system::error_code error;
	std::string result = socket->lowest_layer().remote_endpoint(error).address().to_string();
	if(error)
		return "<unknown address>";
	else
		return result;
}

template<class SocketPtr> bool check_error(const boost::system::error_code& error, SocketPtr socket)
{
	if(error) {
		if(error == boost::asio::error::eof)
			LOG_SERVER << log_address(socket) << "\tconnection closed";
		else
			ERR_SERVER << log_address(socket) << "\t" << error.message();
		return true;
	}
	return false;
}
template bool check_error<tls_socket_ptr>(const boost::system::error_code& error, tls_socket_ptr socket);

namespace {

void info_table_into_simple_wml(simple_wml::document& doc, const std::string& parent_name, const server_base::info_table& info)
{
	if(info.empty()) {
		return;
	}

	auto& node = doc.child(parent_name.c_str())->add_child("data");
	for(const auto& kv : info) {
		node.set_attr_dup(kv.first.c_str(), kv.second.c_str());
	}
}

}

/**
 * Send a WML document from within a coroutine
 * @param socket
 * @param doc
 * @param yield The function will suspend on write operation using this yield context
 */
template<class SocketPtr> void server_base::coro_send_doc(SocketPtr socket, simple_wml::document& doc, const boost::asio::yield_context& yield)
{
	if(dump_wml) {
		std::cout << "Sending WML to " << log_address(socket) << ": \n" << doc.output() << std::endl;
	}

	try {
		simple_wml::string_span s = doc.output_compressed();

		union DataSize
		{
			uint32_t size;
			char buf[4];
		} data_size {};
		data_size.size = htonl(s.size());

		std::vector<boost::asio::const_buffer> buffers {
			{ data_size.buf, 4 },
			{ s.begin(), std::size_t(s.size()) }
		};

		boost::system::error_code ec;
		async_write(*socket, buffers, yield[ec]);
		if(check_error(ec, socket)) {
			socket->lowest_layer().close();
			return;
		}
	} catch (simple_wml::error& e) {
		WRN_CONFIG << __func__ << ": simple_wml error: " << e.message;
		throw;
	}
}
template void server_base::coro_send_doc<socket_ptr>(socket_ptr socket, simple_wml::document& doc, const boost::asio::yield_context& yield);
template void server_base::coro_send_doc<tls_socket_ptr>(tls_socket_ptr socket, simple_wml::document& doc, const boost::asio::yield_context& yield);

template<class SocketPtr> void coro_send_file_userspace(SocketPtr socket, const std::string& filename, const boost::asio::yield_context& yield)
{
	std::size_t filesize { std::size_t(filesystem::file_size(filename)) };
	union DataSize
	{
		uint32_t size;
		char buf[4];
	} data_size {};
	data_size.size = htonl(filesize);

	boost::system::error_code ec;
	async_write(*socket, boost::asio::buffer(data_size.buf), yield[ec]);
	if(check_error(ec, socket)) {
		socket->lowest_layer().close();
		return;
	}

	auto ifs { filesystem::istream_file(filename) };
	ifs->seekg(0);
	while(ifs->good()) {
		char buf[16384];
		ifs->read(buf, sizeof(buf));
		async_write(*socket, boost::asio::buffer(buf, ifs->gcount()), yield[ec]);
		if(check_error(ec, socket)) {
			socket->lowest_layer().close();
			return;
		}
	}
}

#ifdef HAVE_SENDFILE

void server_base::coro_send_file(tls_socket_ptr socket, const std::string& filename, const boost::asio::yield_context& yield)
{
	// We fallback to userspace if using TLS socket because sendfile is not aware of TLS state
	// TODO: keep in mind possibility of using KTLS instead. This seem to be available only in openssl3 branch for now
	coro_send_file_userspace(std::move(socket), filename, yield);
}

void server_base::coro_send_file(const socket_ptr& socket, const std::string& filename, const boost::asio::yield_context& yield)
{
	std::size_t filesize { std::size_t(filesystem::file_size(filename)) };
	int in_file { open(filename.c_str(), O_RDONLY) };
	ON_SCOPE_EXIT(in_file) { close(in_file); };
	off_t offset { 0 };
	//std::size_t total_bytes_transferred { 0 };

	union DataSize
	{
		uint32_t size;
		char buf[4];
	} data_size {};
	data_size.size = htonl(filesize);

	boost::system::error_code ec;
	async_write(*socket, boost::asio::buffer(data_size.buf), yield[ec]);
	if(check_error(ec, socket)) return;

	// Put the underlying socket into non-blocking mode.
	if(!socket->native_non_blocking()) {
		socket->native_non_blocking(true, ec);
		if(check_error(ec, socket)) return;
	}

	for(;;)
	{
		// Try the system call.
		errno = 0;
		int n = ::sendfile(socket->native_handle(), in_file, &offset, 65536);
		ec = boost::system::error_code(n < 0 ? errno : 0,
									   boost::asio::error::get_system_category());
		//total_bytes_transferred += *(yield.ec_) ? 0 : n;

		// Retry operation immediately if interrupted by signal.
		if(ec == boost::asio::error::interrupted)
			continue;

		// Check if we need to run the operation again.
		if (ec == boost::asio::error::would_block
				|| ec == boost::asio::error::try_again)
		{
			// We have to wait for the socket to become ready again.
			socket->async_write_some(boost::asio::null_buffers(), yield[ec]);
			if(check_error(ec, socket)) return;
			continue;
		}

		if (ec || n == 0)
		{
			// An error occurred, or we have reached the end of the file.
			// Either way we must exit the loop.
			break;
		}

		// Loop around to try calling sendfile again.
	}
}

#elif defined(_WIN32)

void server_base::coro_send_file(tls_socket_ptr socket, const std::string& filename, const boost::asio::yield_context& yield)
{
	coro_send_file_userspace(socket, filename, yield);
}

void server_base::coro_send_file(const socket_ptr& socket, const std::string& filename, const boost::asio::yield_context& yield)
{
	OVERLAPPED overlap;
	std::vector<boost::asio::const_buffer> buffers;

	SetLastError(ERROR_SUCCESS);

	std::size_t filesize = filesystem::file_size(filename);
	std::wstring filename_ucs2 = unicode_cast<std::wstring>(filename);
	HANDLE in_file = CreateFileW(filename_ucs2.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
	if (GetLastError() != ERROR_SUCCESS)
	{
		throw std::runtime_error("Failed to open the file");
	}
	BOOST_SCOPE_EXIT_ALL(in_file) {
		CloseHandle(&in_file);
	};

	HANDLE event = CreateEvent(nullptr, TRUE, TRUE, nullptr);
	if (GetLastError() != ERROR_SUCCESS)
	{
		throw std::runtime_error("Failed to create an event");
	}
	BOOST_SCOPE_EXIT_ALL(&overlap) {
		CloseHandle(overlap.hEvent);
	};

	overlap.hEvent = event;

	union DataSize
	{
		uint32_t size;
		char buf[4];
	} data_size {};
	data_size.size = htonl(filesize);

	async_write(*socket, boost::asio::buffer(data_size.buf, 4), yield);

	BOOL success = TransmitFile(socket->native_handle(), in_file, 0, 0, &overlap, nullptr, 0);
	if(!success) {
		if(WSAGetLastError() == WSA_IO_PENDING) {
			while(true) {
				// The request is pending. Wait until it completes.
				socket->async_write_some(boost::asio::null_buffers(), yield);

				DWORD win_ec = GetLastError();
				if (win_ec != ERROR_IO_PENDING && win_ec != ERROR_SUCCESS)
					throw std::runtime_error("TransmitFile failed");

				if(HasOverlappedIoCompleted(&overlap)) break;
			}
		} else {
			throw std::runtime_error("TransmitFile failed");
		}
	}
}

#else

void server_base::coro_send_file(tls_socket_ptr socket, const std::string& filename, const boost::asio::yield_context& yield)
{
	coro_send_file_userspace(socket, filename, yield);
}

void server_base::coro_send_file(const socket_ptr& socket, const std::string& filename, const boost::asio::yield_context& yield)
{
	coro_send_file_userspace(socket, filename, yield);
}

#endif

template<class SocketPtr> std::unique_ptr<simple_wml::document> server_base::coro_receive_doc(SocketPtr socket, const boost::asio::yield_context& yield)
{
	union DataSize
	{
		uint32_t size;
		char buf[4];
	} data_size {};
	boost::system::error_code ec;
	async_read(*socket, boost::asio::buffer(data_size.buf, 4), yield[ec]);
	if(check_error(ec, socket)) return {};
	uint32_t size = ntohl(data_size.size);

	if(size == 0) {
		ERR_SERVER <<
					  log_address(socket) <<
					  "\treceived invalid packet with payload size 0";
		return {};
	}
	if(size > simple_wml::document::document_size_limit) {
		ERR_SERVER <<
					  log_address(socket) <<
					  "\treceived packet with payload size over size limit";
		return {};
	}

	boost::shared_array<char> buffer{ new char[size] };
	async_read(*socket, boost::asio::buffer(buffer.get(), size), yield[ec]);
	if(check_error(ec, socket)) return {};

	try {
		simple_wml::string_span compressed_buf(buffer.get(), size);
		return std::make_unique<simple_wml::document>(compressed_buf);
	}  catch (simple_wml::error& e) {
		ERR_SERVER <<
			log_address(socket) <<
			"\tsimple_wml error in received data: " << e.message;
		async_send_error(socket, "Invalid WML received: " + e.message);
		return {};
	}
}
template std::unique_ptr<simple_wml::document> server_base::coro_receive_doc<socket_ptr>(socket_ptr socket, const boost::asio::yield_context& yield);
template std::unique_ptr<simple_wml::document> server_base::coro_receive_doc<tls_socket_ptr>(tls_socket_ptr socket, const boost::asio::yield_context& yield);

template<class SocketPtr> void server_base::send_doc_queued(SocketPtr socket, std::unique_ptr<simple_wml::document>& doc_ptr, boost::asio::yield_context yield)
{
	static std::map<SocketPtr, std::queue<std::unique_ptr<simple_wml::document>>> queues;

	queues[socket].push(std::move(doc_ptr));
	if(queues[socket].size() > 1) {
		return;
	}

	ON_SCOPE_EXIT(socket) { queues.erase(socket); };

	while(queues[socket].size() > 0) {
		coro_send_doc(socket, *(queues[socket].front()), yield);
		ON_SCOPE_EXIT(socket) { queues[socket].pop(); };
	}
}

template<class SocketPtr> void server_base::async_send_doc_queued(SocketPtr socket, simple_wml::document& doc)
{
	boost::asio::spawn(
		io_service_, [this, doc_ptr = doc.clone(), socket](boost::asio::yield_context yield) mutable {
			send_doc_queued(socket, doc_ptr, yield);
		}
#if BOOST_VERSION >= 108000
		, [](const std::exception_ptr& e) { if (e) std::rethrow_exception(e); }
#endif
	);
}

template<class SocketPtr> void server_base::async_send_error(SocketPtr socket, const std::string& msg, const char* error_code, const info_table& info)
{
	simple_wml::document doc;
	doc.root().add_child("error").set_attr_dup("message", msg.c_str());
	if(*error_code != '\0') {
		doc.child("error")->set_attr("error_code", error_code);
	}
	info_table_into_simple_wml(doc, "error", info);

	async_send_doc_queued(socket, doc);
}
template void server_base::async_send_error<socket_ptr>(socket_ptr socket, const std::string& msg, const char* error_code, const info_table& info);
template void server_base::async_send_error<tls_socket_ptr>(tls_socket_ptr socket, const std::string& msg, const char* error_code, const info_table& info);

template<class SocketPtr> void server_base::async_send_warning(SocketPtr socket, const std::string& msg, const char* warning_code, const info_table& info)
{
	simple_wml::document doc;
	doc.root().add_child("warning").set_attr_dup("message", msg.c_str());
	if(*warning_code != '\0') {
		doc.child("warning")->set_attr("warning_code", warning_code);
	}
	info_table_into_simple_wml(doc, "warning", info);

	async_send_doc_queued(socket, doc);
}
template void server_base::async_send_warning<socket_ptr>(socket_ptr socket, const std::string& msg, const char* warning_code, const info_table& info);
template void server_base::async_send_warning<tls_socket_ptr>(tls_socket_ptr socket, const std::string& msg, const char* warning_code, const info_table& info);

void server_base::load_tls_config(const config& cfg)
{
	tls_enabled_ = cfg["tls_enabled"].to_bool(false);
	if(!tls_enabled_) return;

	tls_context_.set_options(
		boost::asio::ssl::context::default_workarounds
		| boost::asio::ssl::context::no_sslv2
		| boost::asio::ssl::context::no_sslv3
		| boost::asio::ssl::context::single_dh_use
	);

	tls_context_.use_certificate_chain_file(cfg["tls_fullchain"].str());
	tls_context_.use_private_key_file(cfg["tls_private_key"].str(), boost::asio::ssl::context::pem);
	if(!cfg["tls_dh"].str().empty()) tls_context_.use_tmp_dh_file(cfg["tls_dh"].str());
}

std::string server_base::hash_password(const std::string& pw, const std::string& salt, const std::string& username)
{
	if(salt.length() < 12) {
		ERR_SERVER << "Bad salt found for user: " << username;
		return "";
	}

	std::string password = pw;

	// Apparently HTML key-characters are passed to the hashing functions of phpbb in this escaped form.
	// I will do closer investigations on this, for now let's just hope these are all of them.

	// Note: we must obviously replace '&' first, I wasted some time before I figured that out... :)
	for(std::string::size_type pos = 0; (pos = password.find('&', pos)) != std::string::npos; ++pos) {
		password.replace(pos, 1, "&amp;");
	}
	for(std::string::size_type pos = 0; (pos = password.find('\"', pos)) != std::string::npos; ++pos) {
		password.replace(pos, 1, "&quot;");
	}
	for(std::string::size_type pos = 0; (pos = password.find('<', pos)) != std::string::npos; ++pos) {
		password.replace(pos, 1, "&lt;");
	}
	for(std::string::size_type pos = 0; (pos = password.find('>', pos)) != std::string::npos; ++pos) {
		password.replace(pos, 1, "&gt;");
	}

	if(utils::md5::is_valid_prefix(salt)) {
		std::string hash = utils::md5(password, utils::md5::get_salt(salt), utils::md5::get_iteration_count(salt)).base64_digest();
		return salt+hash;
	} else if(utils::bcrypt::is_valid_prefix(salt)) {
		try {
			auto bcrypt_salt = utils::bcrypt::from_salted_salt(salt);
			auto hash = utils::bcrypt::hash_pw(password, bcrypt_salt);
			return hash.base64_digest();
		} catch(const utils::hash_error& err) {
			ERR_SERVER << "bcrypt hash failed for user " << username << ": " << err.what();
			return "";
		}
	} else {
		ERR_SERVER << "Unable to determine how to hash the password for user: " << username;
		return "";
	}
}

// This is just here to get it to build without the deprecation_message function
#include "deprecation.hpp"

std::string deprecated_message(const std::string&, DEP_LEVEL, const version_info&, const std::string&) {return "";}
