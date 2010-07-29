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
    client_( ana::client::create( address, port) ),
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
    return client_->send( ana::buffer( compress_config( config ) ), this );
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

    //10 seconds to receive the ID or disconnect.
    client_->expecting_message( ana::time::seconds( 10 ) );
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
            read_config( buf, cfg )

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
        client_->cancel_pending(); //cancel pending listen operations in raw mode
        client_->run_listener();

        status_ = CONNECTED;
    }
    else if ( ! error )
        throw std::runtime_error("Can't receive a message while disconnected.");
}

void client::handle_send( ana::error_code error, ana::net_id client, ana::operation_id op_id)
{
    handler_.handle_send( error, client, op_id );
}

/* --------------------------------- Server Implementation --------------------------------- */

server::server( handler& handler, int port ) :
    server_( ana::server::create() ),
    handler_( handler )
{
    std::stringstream ss;
    ss << port;

    server_->run( ss.str() );
}

server::~server()
{
    delete server_;
}


void server::set_timeout( ana::net_id id, ana::timeout_policy type, size_t ms )
{
    server_->set_timeouts( id, type, ms);
}

operation_id server::async_send( ana::net_id id, const config& cfg )
{
    server_->send_one( id, ana::buffer( compress_config( config ) ), this );
}

operation_id server::async_send( const config& cfg )
{
    server_->send_all( ana::buffer( compress_config( cfg ) ), this );
}

operation_id server::async_send_except( ana::net_id id,  const config& cfg )
{
    server_->send_all_except( id, ana::buffer( compress_config( cfg ) ), this );
}

void server::waiting_for_message( ana::net_id id, size_t ms_until_timeout )
{
    server_->expecting_message( id, ms_until_timeout );
}

ana::stats* server::get_stats( ana::stat_type type )
{
    return server_->get_stats( type );
}

ana::stats* server::get_stats( ana::net_id id , ana::stat_type type )
{
    return server_->get_stats( id, type );
}

void server::cancel_pending( )
{
    server_->cancel_pending();
}

void server::cancel_pending( ana::net_id  client_id )
{
    server_->cancel_pending( client_id );
}

void server::disconnect()
{
    server_->disconnect();
}

void server::disconnect(ana::net_id client_id )
{
    server_->disconnect( client_id );
}

std::string server::ip_address( ana::net_id client_id ) const
{
    return server_->ip_address( client_id );
}

/*------- Server handlers for ANA's network events. -------*/

void server::handle_connect( ana::error_code error, net_id new_client_id )
{
    if ( ! error )
    {
        pending_ids_.insert( new_client_id );

        //Wait for handshake
        server_->expecting_message( new_client_id, ana::time::seconds(10) );
    }
}

void server::handle_disconnect( ana::error_code error, net_id server_id)
{
}

void server::handle_receive( ana::error_code error, ana::net_id id, ana::detail::read_buffer buf)
{
    std::set< ana::net_id >::iteartor it;

    it = pending_ids_.find( id );

    if ( it != pending_ids_.end() ) //It's pending handshake
    {
        if ( buf->size() != sizeof( wesnoth_id ) )
            server_->disconnect( id );
        else
        {
            wesnoth_id handshake;
            {
                ana::serializer::bistream bis( buffer->string() );

                bis >> handshake;
                ana::network_to_host_long( handshake ); //I'm expecting a 0 anyway
            }

            if ( handshake != 0 )
                server_->disconnect( id );
            else
            {
                //send back it's id
                wesnoth_id network_byte_order_id = id;

                ana::serializer::bostream bos;
                ana::host_to_network_long( network_byte_order_id );
                bos << network_byte_order_id;

                server_->send_one( id, ana::buffer( bos.str() ), handler );
                server_->set_header_first_mode( id );

                pending_ids_.erase( id );
            }
        }
    }
    else
    {
        config cfg;

        if ( ! error )
            read_config( buf, cfg )

        handler_.handle_receive( error, id, cfg );
    }
}

void server::handle_send( ana::error_code error, ana::net_id client, ana::operation_id op_id)
{
    handler_.handle_send( error, client, op_id );
}
