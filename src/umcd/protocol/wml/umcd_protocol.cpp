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

#include <boost/lexical_cast.hpp>
#include <boost/current_function.hpp>
#include <boost/assert.hpp>

#include "umcd/protocol/wml/umcd_protocol.hpp"
#include "umcd/special_packet.hpp"
#include "umcd/umcd_error.hpp"

std::size_t umcd_protocol::REQUEST_HEADER_MAX_SIZE = 8192;
const std::size_t umcd_protocol::REQUEST_HEADER_SIZE_FIELD_LENGTH;

#define FUNCTION_TRACER() UMCD_LOG_IP_FUNCTION_TRACER(socket_)

umcd_protocol::umcd_protocol(io_service_type& io_service, const environment& serverinfo)
: environment_(serverinfo)
, socket_(io_service)
{
}

wml_reply& umcd_protocol::get_reply()
{
	return reply_;
}

config& umcd_protocol::get_metadata()
{
	return request_.get_metadata();
}

void umcd_protocol::load_config(const config& protocol_cfg)
{
	REQUEST_HEADER_MAX_SIZE = protocol_cfg["max_header_size"];
}

umcd_protocol::socket_type& umcd_protocol::socket()
{
	return socket_;
}

void umcd_protocol::complete_request(const boost::system::error_code& error, std::size_t)
{
	FUNCTION_TRACER();
	if(error)
	{
		UMCD_LOG_IP(info, socket_) << " -- unable to send data to the client (" << error.message() << "). Connection dropped.";
		// TODO close socket.
	}
}

void umcd_protocol::async_send_reply()
{
	FUNCTION_TRACER();

	boost::asio::async_write(socket_
		, reply_.to_buffers()
		, boost::bind(&umcd_protocol::complete_request, shared_from_this()
			, boost::asio::placeholders::error
			, boost::asio::placeholders::bytes_transferred)
	);
}

void umcd_protocol::async_send_error(const boost::system::error_condition& error)
{
	reply_ = make_error_reply(error.message(), REQUEST_HEADER_SIZE_FIELD_LENGTH);
	async_send_reply();
}

void umcd_protocol::async_send_invalid_packet(const std::string &where, const std::exception& e)
{
	UMCD_LOG_IP(error, socket_) << " -- invalid request at " << where << " (" << e.what() << ")";
	async_send_error(make_error_condition(invalid_packet));
}

void umcd_protocol::async_send_invalid_packet(const std::string &where, const twml_exception& e)
{
	UMCD_LOG_IP(error, socket_) << " -- invalid request at " << where 
										<< " (user message=" << e.user_message << " ; dev message=" << e.dev_message << ")";
	async_send_error(make_error_condition(invalid_packet));
}

void umcd_protocol::read_request_body(const boost::system::error_code& error, std::size_t)
{
	FUNCTION_TRACER();
	if(!error)
	{
		try
		{
			// NOTE: We encapsulate the boost::array into a string because it old lexical_cast does not support boost::array. Change this when it'll be supported.
			std::string request_size_s = std::string(raw_request_size_.data(), raw_request_size_.size());
			std::size_t request_size = boost::lexical_cast<std::size_t>(request_size_s);
			UMCD_LOG_IP(debug, socket_) << " -- Request of size: " << request_size;
			if(request_size > REQUEST_HEADER_MAX_SIZE)
			{
				async_send_error(make_error_condition(request_header_too_large));
			}
			else
			{
				request_body_.resize(request_size);
				boost::asio::async_read(socket_, boost::asio::buffer(&request_body_[0], request_body_.size())
					, boost::bind(&umcd_protocol::dispatch_request, shared_from_this()
					, boost::asio::placeholders::error
					, boost::asio::placeholders::bytes_transferred)
				);
			}
		}
		catch(const std::exception& e)
		{
			async_send_invalid_packet(BOOST_CURRENT_FUNCTION, e);
		}
	}
}

void umcd_protocol::dispatch_request(const boost::system::error_code& err, std::size_t)
{
	FUNCTION_TRACER();
	if(!err)
	{
		std::stringstream request_stream(request_body_);      
		try
		{
			// Retrieve request name.
			std::string request_name = peek_request_name(request_stream);
			UMCD_LOG_IP(info, socket_) << " -- request: " << request_name;
			info_ptr request_info = environment_.get_request_info(request_name);
			UMCD_LOG_IP(info, socket_) << " -- request:\n" << request_body_;

			request_ = wml_request();
			// Read into config and validate metadata.
			::read(request_.get_metadata(), request_stream, request_info->validator().get());
			UMCD_LOG_IP(debug, socket_) << " -- request validated.";

			request_info->action()->execute(shared_from_this());
		}
		catch(const std::exception& e)
		{
			async_send_invalid_packet(BOOST_CURRENT_FUNCTION, e);
		}
		catch(const twml_exception& e)
		{
			async_send_invalid_packet(BOOST_CURRENT_FUNCTION, e);
		}
	}
}

void umcd_protocol::handle_request()
{
	FUNCTION_TRACER(); // Because we trace with the IP, client_connection must be initialized first.
	boost::asio::async_read(socket_, boost::asio::buffer(raw_request_size_)
		, boost::bind(&umcd_protocol::read_request_body, shared_from_this()
			, boost::asio::placeholders::error
			, boost::asio::placeholders::bytes_transferred)
	);
}

boost::shared_ptr<umcd_protocol> make_umcd_protocol(umcd_protocol::io_service_type& io_service, const environment& serverinfo)
{
	return boost::make_shared<umcd_protocol>(boost::ref(io_service), boost::cref(serverinfo));
}
