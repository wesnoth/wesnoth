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

#include <memory>
#include <array>

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/bind.hpp>

// Represents a single connection from a client.
template <class Protocol>
class connection
  : public std::enable_shared_from_this<connection<Protocol>>,
    private boost::noncopyable
{
public:
  typedef Protocol protocol_type;
private:
  boost::asio::ip::tcp::socket socket;
  protocol_type& protocol;
  std::array<char, 8192> buffer;

public:
  // Construct a connection with the given io_service.
  explicit connection(boost::asio::io_service& io_service,
      protocol_type& protocol);

  // Get the socket associated with the connection.
  boost::asio::ip::tcp::socket& get_socket();

  // Start the first asynchronous operation for the connection.
  void start();

private:
  // Handle completion of a read operation.
  void on_read_complete(const boost::system::error_code& e,
      std::size_t bytes_transferred);

  // Handle completion of a write operation.
  void on_write_complete(const boost::system::error_code& e);
};

template <class Protocol>
connection<Protocol>::connection(boost::asio::io_service& io_service, 
  protocol_type& protocol)
: socket(io_service)
, protocol(protocol)
{
}

template <class Protocol>
boost::asio::ip::tcp::socket& connection<Protocol>::get_socket()
{
  return socket;
}

template <class Protocol>
void connection<Protocol>::start()
{
  socket.async_read_some(boost::asio::buffer(buffer),
        boost::bind(&connection::on_read_complete, this->shared_from_this(),
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
}

template <class Protocol>
void connection<Protocol>::on_read_complete(const boost::system::error_code& e,
    std::size_t bytes_transferred)
{
  if (!e)
  {
    typename protocol_type::reply_type reply = protocol.handle_request(buffer.data());
    boost::asio::async_write(socket, reply.to_buffers(),
      boost::bind(&connection::on_write_complete, this->shared_from_this(),
      boost::asio::placeholders::error));

    std::cout << "[Info] Read complete with " << bytes_transferred << " bytes transferred." << std::endl;
  }
  else
  {
    std::cerr << "[Error] connection::on_read_complete: " << e << std::endl;
  }

  // If an error occurs then no new asynchronous operations are started. This
  // means that all shared_ptr references to the connection object will
  // disappear and the object will be destroyed automatically after this
  // handler returns. The connection class's destructor closes the socket.
}

template <class Protocol>
void connection<Protocol>::on_write_complete(const boost::system::error_code& e)
{
  if (!e)
  {
    // Initiate graceful connection closure.
    boost::system::error_code ignored_ec;
    socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
  }

  // No new asynchronous operations are started. This means that all shared_ptr
  // references to the connection object will disappear and the object will be
  // destroyed automatically after this handler returns. The connection class's
  // destructor closes the socket.
}

#endif // SERVER_CONNECTION_HPP
