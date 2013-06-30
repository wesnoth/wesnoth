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

#include <boost/type_traits.hpp>

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

// Represents a single connection from a client.
template <class Protocol>
class connection
  : public boost::enable_shared_from_this<connection<Protocol> >,
    private boost::noncopyable
{
  typedef boost::asio::basic_socket_streambuf<boost::asio::ip::tcp> result_of_rdbuf;
  typedef boost::asio::ip::tcp::iostream tcp_iostream;

public:
  typedef Protocol protocol_type;
  typedef boost::remove_pointer<result_of_rdbuf>::type socket_type;

private:
  boost::asio::io_service& io_service;
  protocol_type& protocol;
  tcp_iostream data_stream;

public:
  // Construct a connection with the given io_service.
  explicit connection(boost::asio::io_service& io_service,
      protocol_type& protocol);

  // Get the socket associated with the connection.
  socket_type& get_socket();

  // Start the first asynchronous operation for the connection.
  void start();

private:
  // Handle request.
  void handle_request();
};

template <class Protocol>
connection<Protocol>::connection(boost::asio::io_service& io_service, 
  protocol_type& protocol)
: io_service(io_service)
, protocol(protocol)
{
}

template <class Protocol>
typename connection<Protocol>::socket_type& connection<Protocol>::get_socket()
{
  return *data_stream.rdbuf();
}

template <class Protocol>
void connection<Protocol>::start()
{
  io_service.post(
    boost::bind(&connection::handle_request, this->shared_from_this()));
}

template <class Protocol>
void connection<Protocol>::handle_request()
{
  std::cout << "[Info] enter connection::handle_request()" << std::endl;
  protocol.handle_request(data_stream);
  std::cout << "[Info] quit connection::handle_request()" << std::endl;
}

#endif // SERVER_CONNECTION_HPP
