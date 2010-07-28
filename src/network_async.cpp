/* $Id$ */

/**
 * @file
 * @brief New network API implementation using ana.
 *
 * Copyright (C) 2010 Guillermo Biset.
 *
 * Part of the Battle for Wesnoth Project http://www.wesnoth.org/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * or at your option any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY.
 *
 * See the COPYING file for more details.
 */

#include "network_askync.hpp"

using namespace network;

/* --------------------------------- Client Implementation --------------------------------- */

client::client( handler& handler ) :
    client_( ... )
{
}

client::~client()
{
    delete client_;
}

void client::set_send_timeout( ana::timeout_policy type, size_t ms )
{
}

void client::set_connect_timeout( size_t ms  )
{
}

void client::async_connect( const std::string& address = "server.wesnoth.org",
                            const std::string& port    = "15000"  )
{
}

void client::async_connect_through_proxy( const std::string& proxy_addr  = "server.wesnoth.org",
                                          const std::string& proxy_port  = "15000",
                                          const std::string& user_name   = "",
                                          const std::string& password    = "")
{
}

operation_id client::async_send( const config&, ana::send_type = ana::COPY_BUFFER)
{
}

void client::waiting_for_message( time )
{
}

ana::stats* client::get_stats( ana::stat_type = ana::ACCUMULATED )
{
}

void client::cancel_pending( )
{
}

void client::disconnect()
{
}

std::string client::ip_address_of_server() const
{
}

/*------- Client handlers for ANA's network events. -------*/

void client::handle_connect( ana::error_code error, net_id server_id )
{
}

void client::handle_disconnect( ana::error_code error, net_id server_id)
{
}

void client::handle_receive( ana::error_code error, ana::net_id id, ana::detail::read_buffer buf)
{
}

void client::handle_send( ana::error_code error, ana::net_id client, ana::operation_id op_id)
{
}

/* --------------------------------- Server Implementation --------------------------------- */

server::server( port, handler& ) :
    server_( )
{
}

server::~server()
{
    delete server_;
}


void server::set_timeout( ana::net_id, ana::timeout_policy type, size_t ms )
{
}

operation_id server::async_send( ana::net_id, const cfg&, ana::send_type )
{
}

operation_id server::async_send( const cfg&, ana::send_type )
{
}

operation_id server::async_send( container_of_ids,  const cfg&, ana::send_type )
{
}

operation_id server::async_send_except( ana::net_id,  const cfg&, ana::send_type )
{
}

operation_id server::async_send_except( container_of_ids,  const cfg&, ana::send_type )
{
}

void server::waiting_for_message( ana::net_id, size_t ms_until_timeout )
{
}

ana::stats* server::get_stats( ana::stat_type = ana::ACCUMULATED )
{
}

ana::stats* server::get_stats( ana::net_id, ana::stat_type = ana::ACCUMULATED )
{
}

void server::cancel_pending( )
{
}

void server::cancel_pending( ana::net_id  client_id )
{
}

void server::disconnect()
{
}

void server::disconnect(ana::net_id)
{
}

std::string server::ip_address( ana::net_id ) const
{
}

/*------- Server handlers for ANA's network events. -------*/

void server::handle_connect( ana::error_code error, net_id server_id )
{
}

void server::handle_disconnect( ana::error_code error, net_id server_id)
{
}

void server::handle_receive( ana::error_code error, ana::net_id id, ana::detail::read_buffer buf)
{
}

void server::handle_send( ana::error_code error, ana::net_id client, ana::operation_id op_id)
{
}
