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

#ifndef SERVER_MULTI_THREADED_SERVER_HPP
#define SERVER_MULTI_THREADED_SERVER_HPP

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread/thread.hpp>
#include "umcd/server/connection.hpp"
#include "umcd/umcd_logger.hpp"

template <class Protocol>
class server_mt : boost::noncopyable
{
  public:
    typedef Protocol protocol_type;
  private:
    typedef connection<protocol_type> connection_type;
    typedef boost::shared_ptr<connection_type> connection_ptr;

    boost::asio::io_service io_service;
    boost::asio::ip::tcp::acceptor acceptor;

    connection_ptr new_connection;
    protocol_type& protocol;
    bool server_on;
    std::size_t thread_pool_size;

  public:
    explicit server_mt(const config& cfg, protocol_type& protocol);
    void run();

  private:
    void run_thread();
    void start_accept();
    void handle_accept(const boost::system::error_code& e);
    void handle_stop();
};

template <class Protocol>
server_mt<Protocol>::server_mt(const config &cfg, protocol_type& protocol) 
: acceptor(io_service)
, protocol(protocol)
, server_on(true)
{
  using namespace boost::asio::ip;

  thread_pool_size = cfg["threads"];
  if(thread_pool_size == 0)
  {
    thread_pool_size = boost::thread::hardware_concurrency();
    UMCD_LOG(info) << thread_pool_size << " cores found.";
  }
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
void server_mt<Protocol>::run_thread()
{
  while(server_on)
  {
    try
    {
      io_service.run();
    }
    catch(std::exception& e)
    {
      UMCD_LOG(error) << "Exception in server_mt::run(): handler shouldn't launch exception! (" << e.what() << ")";
    }
  }
}

template <class Protocol>
void server_mt<Protocol>::run()
{
  // Create a pool of threads to run all of the io_services.
  std::vector<boost::shared_ptr<boost::thread> > threads;
  for (std::size_t i = 0; i < thread_pool_size-1; ++i)
  {
    boost::shared_ptr<boost::thread> thread = boost::make_shared<boost::thread>(
          boost::bind(&server_mt::run_thread, this));
    threads.push_back(thread);
  }

  // This thread is also used.
  run_thread();

  // Wait for all threads in the pool to exit.
  for (std::size_t i = 0; i < threads.size(); ++i)
    threads[i]->join();
}

template <class Protocol>
void server_mt<Protocol>::start_accept()
{
  new_connection.reset(new connection_type(io_service, protocol));
  acceptor.async_accept(new_connection->get_socket(),
    boost::bind(&server_mt::handle_accept, this, boost::asio::placeholders::error)
  );
}

template <class Protocol>
void server_mt<Protocol>::handle_accept(const boost::system::error_code& e)
{
  if (!e)
  {
    new_connection->start();
  }
  start_accept();
}

template <class Protocol>
void server_mt<Protocol>::handle_stop()
{
  server_on = false;
  io_service.stop();
}

#endif // SERVER_MULTI_THREADED_SERVER_HPP