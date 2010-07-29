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

/* STL headers ----------------------------------- */

#include <string>
#include <stringstream>

/* Boost headers --------------------------------- */

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>

/* Local headers --------------------------------- */

#include "network_askync.hpp"
#include "serialization/parser.hpp"

/* ----------------------------------------------- */

using namespace network;

/* ----------------------------------- Utility Functions ----------------------------------- */

namespace utils
{

    std::string compress_config( const config& cfg )
    {
        std::ostringstream out;
        compress_config( cfg, out );
        return out.str( );
    }


    void compress_config( const config& cfg, std::ostringstream& out)
    {
        boost::iostreams::filtering_stream<boost::iostreams::output> filter;
        filter.push(boost::iostreams::gzip_compressor());
        filter.push(out);
        write(filter, cfg);
        out.flush();
    }


    void read_config( const ana::detail::read_buffer& buffer, config& cfg)
    {
        std::istringstream input( buffer->string() );

        read_gz(cfg, input);
    }
}

/* --------------------------------- Client Implementation --------------------------------- */

client::client( handler& handler,
                const std::string& address = "server.wesnoth.org",
                const std::string& port    = "15000" )
  :
    status_( DISCONNECTED ),
    client_( address, port ),
    handler_( handler ),
    wesnoth_id_( 0 )
{
    client_->set_listener_handler( this );
    client_->set_raw_data_mode();
    client_->run();
    client_->start_logging();
}

client::~client()
{
    client_->disconnect();
    delete client_;
}

void client::set_send_timeout( ana::timeout_policy type, size_t ms )
{
    client_->set_timeouts( type, ms );
}

void client::async_connect( size_t timeout )
{
    //TODO: Deal with the case that the client is not disconnected.

    client_->set_connect_timeout( timeout );
    client_->connect();
}

void client::async_connect_through_proxy( size_t             timeout,
                                          const std::string& proxy_addr,
                                          const std::string& proxy_port,
                                          const std::string& user_name,
                                          const std::string& password)
{
    client_->set_connect_timeout( timeout );
    client_->connect_trough_proxy(proxy_addr, proxy_port, this, user_name, password);
}

operation_id client::async_send( const config& )
{
    return client_->send( ana::buffer( compress_config( config ) ) );
}

void client::waiting_for_message( size_t time )
{
    client_->expecting_message( time );
}

ana::stats* client::get_stats( ana::stat_type type )
{
    return client_->get_stats( type );
}

void client::cancel_pending( )
{
    client_->cancel_pending();
}

void client::disconnect()
{
    client_->disconnect();
}

std::string client::ip_address_of_server() const
{
    return client_->ip_address();
}

network::wesnoth_id client::get_wesnoth_id() const
{
    return wesnoth_id_;
}

/*------- Private methods ---------------------------------*/

void client::set_wesnoth_id( wesnoth_id id)
{
    wesnoth_id_ = id;
}

/*------- Client handlers for ANA's network events. -------*/

void client::handle_connect( ana::error_code error, net_id server_id )
{
    ana::serializer::bostream bos;

    uint32_t handshake( 0 );
    bos << handshake;

    client_->send( ana::buffer( bos.str()), this );

    status_ = PENDING_HANDSHAKE;
}

void client::handle_disconnect( ana::error_code error, net_id server_id)
{
    handler_.handle_disconnect( error, server_id );
}

void client::handle_receive( ana::error_code error, ana::net_id id, ana::detail::read_buffer buf)
{
    if ( status_ == CONNECTED )
    {
        config cfg;

        if ( ! error )
            read_config( buf, cfg)

        handler_.handle_receive( error, id, cfg );
    }
    else if ( status_ == PENDING_HANDSHAKE )
    {
        wesnoth_id my_id;
        ana::serializer::bistream bis;

        client->wait_raw_object(bis, sizeof(my_id) );

        bis >> my_id;
        ana::network_to_host_long( my_id );

        set_wesnoth_id( my_id );

        client_->set_header_first_mode();
        client_->run_listener();
    }
    else
        throw std::runtime_error("Can't receive a message while disconnected.");
}

void client::handle_send( ana::error_code error, ana::net_id client, ana::operation_id op_id)
{
    handler_.handle_receive( error, client, op_id );
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
