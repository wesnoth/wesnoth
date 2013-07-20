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

#ifndef SERVER_BASIC_SERVER_HPP
#define SERVER_BASIC_SERVER_HPP

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread/thread.hpp>
#include "umcd/server/connection.hpp"
#include "umcd/umcd_logger.hpp"

template <class Protocol>
class basic_server : boost::noncopyable
{
  public:
    typedef Protocol protocol_type;
  protected:
    typedef connection<protocol_type> connection_type;
    typedef boost::shared_ptr<connection_type> connection_ptr;

    boost::asio::io_service io_service;
    boost::asio::ip::tcp::acceptor acceptor;

    connection_ptr new_connection;
    boost::shared_ptr<protocol_type> protocol;
    bool server_on;

  public:
    explicit basic_server(const config& cfg, const boost::shared_ptr<protocol_type>& protocol);
    void run();

  private:
    void start_accept();
    void handle_accept(const boost::system::error_code& e);
    void handle_stop();
};

template <class Protocol>
basic_server<Protocol>::basic_server(const config &cfg, const boost::shared_ptr<protocol_type>& protocol) 
: acceptor(io_service)
, protocol(protocol)
, server_on(true)
{
  using namespace boost::asio::ip;

  // Find an endpoint on the port specified, if none found, throw a runtime_error exception.
  std::string port = cfg["port"];
  tcp::resolver resolver(io_service);
  tcp::resolver::query query(port, tcp::resolver::query::address_configured);
  tcp::resolver::iterator endpoint_iter = resolver.resolve(query);
  tcp::resolver::iterator endpoint_end;

  for(; endpoint_iter != endpoint_end; ++endpoint_iter)
  {
    try
    {
      tcp::endpoint endpoint(*endpoint_iter);
      acceptor.open(endpoint.protocol());
      acceptor.bind(endpoint);
      acceptor.listen();
      UMCD_LOG(info) << "server: " << endpoint;
      break;
    }
    catch(std::exception &e)
    {
      UMCD_LOG(error) << e.what() << "\n";
    }
  }
  if(endpoint_iter == endpoint_end)
  {
    throw std::runtime_error("No endpoints found - Check the status of your network interfaces.\n");
  }
  start_accept();
}

template <class Protocol>
void basic_server<Protocol>::run()
{
  while(server_on)
  {
    try
    {
      io_service.run();
    }
    catch(std::exception& e)
    {
      UMCD_LOG(error) << "Exception in basic_server::run(): handler shouldn't launch exception! (" << e.what() << ")";
    }
  }
}

template <class Protocol>
void basic_server<Protocol>::start_accept()
{
  UMCD_LOG(trace) << "basic_server<Protocol>::start_accept()";
  protocol = boost::make_shared<protocol_type>(*protocol);
  new_connection = boost::make_shared<connection_type>(boost::ref(io_service), protocol);
  acceptor.async_accept(new_connection->get_socket(),
    boost::bind(&basic_server::handle_accept, this, boost::asio::placeholders::error)
  );
}

template <class Protocol>
void basic_server<Protocol>::handle_accept(const boost::system::error_code& e)
{
  UMCD_LOG(trace) << "basic_server<Protocol>::handle_accept()";
  if (!e)
  {
    new_connection->start();
  }
  start_accept();
}

template <class Protocol>
void basic_server<Protocol>::handle_stop()
{
  server_on = false;
  io_service.stop();
}

#endif // SERVER_BASIC_SERVER_HPP