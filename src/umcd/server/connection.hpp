/*
	Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)

	Distributed under the Boost Software License, Version 1.0.
	(See http://www.boost.org/LICENSE_1_0.txt)
*/
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

#ifndef SERVER_CONNECTION_HPP
#define SERVER_CONNECTION_HPP

#include "umcd/umcd_logger.hpp"

#include <boost/type_traits.hpp>
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/current_function.hpp>

// Represents a single connection from a client.
template <class Protocol>
class connection
	: public boost::enable_shared_from_this<connection<Protocol> >,
		private boost::noncopyable
{

public:
	typedef Protocol protocol_type;
	typedef boost::asio::ip::tcp::socket socket_type;

public:
	// Construct a connection with the given io_service.
	explicit connection(boost::asio::io_service& io_service,
			const boost::shared_ptr<protocol_type>& protocol);

	// Get the socket associated with the connection.
	socket_type& get_socket();

	boost::asio::io_service& get_io_service();

	// Start the first asynchronous operation for the connection.
	void start();

private:
	socket_type socket_;
	boost::shared_ptr<protocol_type> protocol_;
};

template <class Protocol>
connection<Protocol>::connection(boost::asio::io_service& io_service, 
	const boost::shared_ptr<protocol_type>& protocol)
: socket_(io_service)
, protocol_(protocol)
{
}

template <class Protocol>
typename connection<Protocol>::socket_type& connection<Protocol>::get_socket()
{
	return socket_;
}

template <class Protocol>
boost::asio::io_service& connection<Protocol>::get_io_service()
{
	return socket_.get_io_service();
}

template <class Protocol>
void connection<Protocol>::start()
{
	UMCD_LOG_IP_FUNCTION_TRACER(get_socket());
	// Post is threadsafe. We want that start() returns immediately to accept other requests.
	get_io_service().post(
		boost::bind(&protocol_type::handle_request, protocol_, this->shared_from_this()));
}

#endif // SERVER_CONNECTION_HPP
