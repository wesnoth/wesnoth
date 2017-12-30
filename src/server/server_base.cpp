/*
   Copyright (C) 2016 - 2017 by Sergey Popov <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License 2
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "server/server_base.hpp"

#include "lexical_cast.hpp"
#include "log.hpp"
#include "utils/functional.hpp"

static lg::log_domain log_server("server");
#define ERR_SERVER LOG_STREAM(err, log_server)
#define WRN_SERVER LOG_STREAM(warn, log_server)
#define LOG_SERVER LOG_STREAM(info, log_server)
#define DBG_SERVER LOG_STREAM(debug, log_server)

static lg::log_domain log_config("config");
#define ERR_CONFIG LOG_STREAM(err, log_config)
#define WRN_CONFIG LOG_STREAM(warn, log_config)

#include "server/send_receive_wml_helpers.ipp"

server_base::server_base(unsigned short port, bool keep_alive) :
	port_(port),
	keep_alive_(keep_alive),
	io_service_(),
	acceptor_(io_service_),
	#ifndef _WIN32
	input_(io_service_),
	sighup_(io_service_, SIGHUP),
	#endif
	sigs_(io_service_, SIGINT, SIGTERM)
{
}

void server_base::start_server()
{
	boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port_);
	acceptor_.open(endpoint.protocol());
	acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
	acceptor_.set_option(boost::asio::ip::tcp::acceptor::keep_alive(keep_alive_));
	acceptor_.bind(endpoint);
	acceptor_.listen();
	serve();

	handshake_response_.connection_num = 42;

#ifndef _WIN32
	sighup_.async_wait(
		[=](const boost::system::error_code& error, int sig)
			{ this->handle_sighup(error, sig); });
#endif
	sigs_.async_wait(std::bind(&server_base::handle_termination, this, _1, _2));
}

void server_base::serve()
{
	socket_ptr socket = std::make_shared<boost::asio::ip::tcp::socket>(std::ref(io_service_));
	acceptor_.async_accept(*socket, std::bind(&server_base::accept_connection, this, _1, socket));
}

void server_base::accept_connection(const boost::system::error_code& error, socket_ptr socket)
{
	if(accepting_connections())
		serve();
	if(error) {
		ERR_SERVER << "Accept failed: " << error.message() << "\n";
		return;
	}

	const std::string ip = client_address(socket);

	const std::string reason = is_ip_banned(ip);
	if (!reason.empty()) {
		LOG_SERVER << ip << "\trejected banned user. Reason: " << reason << "\n";
		async_send_error(socket, "You are banned. Reason: " + reason);
		return;
	} else if (ip_exceeds_connection_limit(ip)) {
		LOG_SERVER << ip << "\trejected ip due to excessive connections\n";
		async_send_error(socket, "Too many connections from your IP.");
		return;
	} else {

		DBG_SERVER << ip << "\tnew connection accepted\n";
		serverside_handshake(socket);
	}

}

void server_base::serverside_handshake(socket_ptr socket)
{
	boost::shared_array<char> handshake(new char[4]);
	async_read(
				*socket, boost::asio::buffer(handshake.get(), 4),
				std::bind(&server_base::handle_handshake, this, _1, socket, handshake)
				);
}

void server_base::handle_handshake(const boost::system::error_code& error, socket_ptr socket, boost::shared_array<char> handshake)
{
	if(check_error(error, socket))
		return;

	if(memcmp(handshake.get(), "\0\0\0\0", 4) != 0) {
		ERR_SERVER << client_address(socket) << "\tincorrect handshake\n";
		return;
	}
	async_write(
				*socket, boost::asio::buffer(handshake_response_.buf, 4),
				[=](const boost::system::error_code& error, size_t)
					{ if(!check_error(error, socket)) this->handle_new_client(socket); }
	);
}

#ifndef _WIN32
void server_base::read_from_fifo() {
	async_read_until(input_,
					 admin_cmd_, '\n',
					 [=](const boost::system::error_code& error, std::size_t bytes_transferred)
						{ this->handle_read_from_fifo(error, bytes_transferred); }
	);
}
#endif

void server_base::handle_termination(const boost::system::error_code& error, int signal_number)
{
	assert(!error);

	std::string signame;
	if(signal_number == SIGINT) signame = "SIGINT";
	else if(signal_number == SIGTERM) signame = "SIGTERM";
	else signame = lexical_cast<std::string>(signal_number);
	LOG_SERVER << signame << " caught, exiting without cleanup immediately.\n";
	exit(128 + signal_number);
}

void server_base::run() {
	try {
		io_service_.run();
		LOG_SERVER << "Server has shut down because event loop is out of work\n";
	} catch(const server_shutdown& e) {
		LOG_SERVER << "Server has been shut down: " << e.what() << "\n";
	}
}

std::string client_address(const socket_ptr socket)
{
	boost::system::error_code error;
	std::string result = socket->remote_endpoint(error).address().to_string();
	if(error)
		return "<unknown address>";
	else
		return result;
}

bool check_error(const boost::system::error_code& error, socket_ptr socket)
{
	if(error) {
		if(error == boost::asio::error::eof)
			LOG_SERVER << client_address(socket) << "\tconnection closed\n";
		else
			ERR_SERVER << client_address(socket) << "\t" << error.message() << "\n";
		return true;
	}
	return false;
}

void async_send_error(socket_ptr socket, const std::string& msg, const char* error_code)
{
	simple_wml::document doc;
	doc.root().add_child("error").set_attr_dup("message", msg.c_str());
	if(*error_code != '\0') {
		doc.child("error")->set_attr("error_code", error_code);
	}

	async_send_doc(socket, doc);
}

void async_send_warning(socket_ptr socket, const std::string& msg, const char* warning_code)
{
	simple_wml::document doc;
	doc.root().add_child("warning").set_attr_dup("message", msg.c_str());
	if(*warning_code != '\0') {
		doc.child("warning")->set_attr("warning_code", warning_code);
	}

	async_send_doc(socket, doc);
}

void async_send_message(socket_ptr socket, const std::string& msg)
{
	simple_wml::document doc;
	doc.root().add_child("message").set_attr_dup("message", msg.c_str());

	async_send_doc(socket, doc);
}
