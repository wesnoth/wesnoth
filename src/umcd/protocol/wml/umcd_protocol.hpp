/*
	Copyright (C) 2013 by Pierre Talbot <ptalbot@mopong.net>
	Part of the Battle for Wesnoth Project http://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#ifndef UMCD_PROTOCOL_HPP
#define UMCD_PROTOCOL_HPP

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
	 
#include "wml_exception.hpp"

#include "umcd/environment.hpp"
#include "umcd/wml_request.hpp"
#include "umcd/umcd_logger.hpp"
#include "umcd/wml_reply.hpp"
#include "umcd/request_info.hpp"

class wml_request;

class umcd_protocol : 
		public boost::enable_shared_from_this<umcd_protocol>
	,	private boost::noncopyable
{
public:
	static std::size_t REQUEST_HEADER_MAX_SIZE;
	static const std::size_t REQUEST_HEADER_SIZE_FIELD_LENGTH = 10;
	typedef boost::asio::ip::tcp::socket socket_type;
	typedef boost::asio::io_service io_service_type;

private:
	typedef basic_umcd_action action_type;
	typedef boost::shared_ptr<request_info> info_ptr;

public:
	static void load_config(const config& protocol_cfg);

	// This constructor is only called once in main, so the factory will be created once as well.
	umcd_protocol(io_service_type& io_service, const environment& serverinfo);

	void handle_request();
	// Precondition: handle_request has been called and connection has been initialized.
	void async_send_reply();

	wml_reply& get_reply();
	config& get_metadata();

	socket_type& socket();

private:
	void complete_request(const boost::system::error_code& error, std::size_t bytes_transferred);

	void async_send_error(const boost::system::error_condition& error);
	void async_send_invalid_packet(const std::string &where, const std::exception& e);
	void async_send_invalid_packet(const std::string &where, const twml_exception& e);

	// Precondition: size_of_request must be read.
	void read_request_body(const boost::system::error_code& error, std::size_t bytes_transferred);

	// Precondition: request_body must be read.
	void dispatch_request(const boost::system::error_code& error, std::size_t bytes_transferred);

private:
	const environment& environment_;
	socket_type socket_;
	boost::array<char, REQUEST_HEADER_SIZE_FIELD_LENGTH> raw_request_size_;
	std::string request_body_;
	wml_reply reply_;
	wml_request request_;
};

boost::shared_ptr<umcd_protocol> make_umcd_protocol(umcd_protocol::io_service_type& io_service, const environment& serverinfo);

#endif // UMCD_PROTOCOL_HPP
