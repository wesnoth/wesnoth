/* $Id$ */

/**
 * @file network_manager_ana.cpp
 * @brief Implementation file for network features using ana.
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

#include <iostream>

#include <boost/bind.hpp>

#include "network_manager_ana.hpp"
#include "serialization/binary_or_text.hpp"

// Begin ana_send_handler implementation ---------------------------------------------------------------

ana_send_handler::ana_send_handler( send_stats_logger* logger, size_t buf_size, size_t calls) :
    mutex_(),
    target_calls_( calls ),
    error_code_(),
    logger_( logger ),
    buf_size_( buf_size )
{
    std::cout << "DEBUG: Constructing a new ana_send_handler...\n";
    if ( calls > 0 )
        mutex_.lock();
}

ana_send_handler::~ana_send_handler()
{
    std::cout << "DEBUG: Terminating an ana_send_handler...\n";
    if ( target_calls_ > 0 )
        throw std::runtime_error("Handler wasn't called enough times.");
}

void ana_send_handler::handle_send(ana::error_code error_code, ana::net_id /*client*/)
{
    if ( ! error_code )
        logger_->update_send_stats( buf_size_ );

    error_code_ = error_code;

    if ( --target_calls_ == 0 )
        mutex_.unlock();
}

void ana_send_handler::wait_completion()
{
    mutex_.lock();
    mutex_.unlock();
}

// Begin ana_connect_handler implementation ------------------------------------------------------------

ana_connect_handler::ana_connect_handler( ana::timer* timer ) :
    mutex_( ),
    timer_(timer),
    error_code_(),
    connected_(false)
{
    std::cout << "DEBUG: Constructing a new ana_connect_handler...\n";
    mutex_.lock();
}

void ana_connect_handler::handle_timeout(ana::error_code error_code)
{
    if ( ! connected_ ) // disregard this call after a connect termination (regardless of result)
    {
        if (error_code)
            std::cout << "DEBUG: Connection attempt timed out\n";
        else
            std::cout << "DEBUG: Shouldn't reach here\n";

        error_code_ = error_code;
        mutex_.unlock();
    }
}

ana_connect_handler::~ana_connect_handler()
{
    std::cout << "DEBUG: Terminating an ana_connect_handler...\n";
}

const ana::error_code& ana_connect_handler::error() const
{
    return error_code_;
}

void ana_connect_handler::handle_connect(ana::error_code error_code, ana::net_id /*client*/)
{
    connected_ = true;
    timer_->cancel();

    if (! error_code)
        std::cout << "DEBUG: Connected.\n";
    else
        std::cout << "DEBUG: Can't connect.\n";

    error_code_ = error_code;
    mutex_.unlock();
}

void ana_connect_handler::wait_completion()
{
    mutex_.lock();
    mutex_.unlock();
}

// Begin ana_component implementation ------------------------------------------------------------------

ana_component::ana_component( ) :
    base_( ana::server::create() ),
    is_server_( true ),
    id_( server()->id() ),
    send_stats_(),
    receive_stats_(),
    mutex_(),
    condition_(),
    buffers_()
{
}

ana_component::ana_component( const std::string& host, const std::string& port) :
    base_( ana::client::create(host,port) ),
    is_server_( false ),
    id_(  client()->id() ),
    send_stats_(),
    receive_stats_(),
    mutex_(),
    condition_(),
    buffers_()
{
}

network::statistics ana_component::get_send_stats() const
{
    return send_stats_;
}

network::statistics ana_component::get_receive_stats() const
{
    return receive_stats_;
}

ana::server* ana_component::server() const
{
    if( ! is_server_ )
        throw std::runtime_error("Component is not a server.");

    return boost::get<ana::server*>(base_);
}

ana::client* ana_component::client() const
{
    if( is_server_ )
        throw std::runtime_error("Component is not a client.");

    return boost::get<ana::client*>(base_);
}

bool ana_component::is_server() const
{
    return is_server_;
}

bool ana_component::is_client() const
{
    return ! is_server_;
}

ana::net_id ana_component::get_id() const
{
    return id_;
}

const ana::stats* ana_component::get_stats() const
{
    if ( is_server_)
        return server()->get_stats();
    else
        return client()->get_stats();
}

void ana_component::add_buffer(ana::detail::read_buffer buffer)
{
    {
        boost::lock_guard<boost::mutex> lock(mutex_);
        buffers_.push( buffer );
    }
    condition_.notify_all();
}

ana::detail::read_buffer ana_component::wait_for_element()
{
    boost::unique_lock<boost::mutex> lock(mutex_);

    while(buffers_.empty())
        condition_.wait(lock);

    const ana::detail::read_buffer buffer_ret = buffers_.front();

    buffers_.pop();

    return buffer_ret;
}

void ana_component::update_receive_stats( size_t buffer_size )
{
    receive_stats_.current_max = ( buffer_size > receive_stats_.current_max) ? buffer_size : receive_stats_.current_max;
    receive_stats_.total      += buffer_size;
    receive_stats_.current     = buffer_size;
}

void ana_component::update_send_stats( size_t buffer_size)
{
    send_stats_.current_max = ( buffer_size > send_stats_.current_max) ? buffer_size : send_stats_.current_max;
    send_stats_.total      += buffer_size;
    send_stats_.current     = buffer_size;
}

// Begin clients_manager  implementation ---------------------------------------------------------------

clients_manager::clients_manager() :
    ids_()
{
}

size_t clients_manager::client_amount() const
{
    return ids_.size();
}

void clients_manager::handle_connect(ana::error_code error, ana::net_id client)
{
    if (! error )
        ids_.insert(client);
}

void clients_manager::handle_disconnect(ana::error_code /*error*/, ana::net_id client)
{
    ids_.erase(client);
}

// Begin ana_network_manager implementation ------------------------------------------------------------

ana_network_manager::ana_network_manager() :
    connect_timer_( NULL ),
    components_(),
    server_manager_()
{
}

ana::net_id ana_network_manager::create_server( )
{
    std::cout << "DEBUG: Creating server.\n";

    ana_component* new_component = new ana_component( );
    components_.insert( new_component );

    ana::server* server = new_component->server();

    clients_manager* manager = new clients_manager();
    server_manager_[ server ] = manager;

    server->set_connection_handler( manager );
    server->set_listener_handler( this );

    return server->id();
}

network::connection ana_network_manager::create_client_and_connect(std::string host, int port)
{
    std::cout << "DEBUG: Creating client and connecting...\n";

    try
    {
        std::stringstream ss;
        ss << port;

        ana_component* new_component = new ana_component( host, ss.str() );
        components_.insert( new_component );

        ana::client* const client = new_component->client();

        connect_timer_ = new ana::timer();

        ana_connect_handler handler(connect_timer_);

        connect_timer_->wait( ana::time::seconds(10), // 10 seconds to connection timeout
                            boost::bind(&ana_connect_handler::handle_timeout, &handler,
                                        boost::asio::error::make_error_code( boost::asio::error::timed_out ) ) );

        client->connect( &handler );

        client->set_listener_handler( this );
        client->run();
        client->start_logging();

        handler.wait_completion(); // just wait for handler to finish

        delete connect_timer_;

        if( ! handler.error() )
        {
            //Send handshake
            const std::string empty_str;
            client->send( ana::buffer( empty_str ), this );

            uint32_t my_id;
            ana::serializer::bistream bis;

            client->wait_raw_object(bis, sizeof(my_id) );

            bis >> my_id;

            std::cout << "DEBUG: Received id " << my_id << "\n";

            return network::connection( client->id() );
        }
        else
            return 0;
    }
    catch( const std::exception& e )
    {
        std::cout << "DEBUG: Caught an exception while trying to connect.\n";
        return 0;
    }
}

const ana::stats* ana_network_manager::get_stats( network::connection connection_num )
{
    ana::net_id id( connection_num );

    std::set<ana_component*>::iterator it;

    it = std::find_if( components_.begin(), components_.end(),
                        boost::bind(&ana_component::get_id, _1) == id );

    if ( it != components_.end())
        return (*it)->get_stats();

    return NULL;
}

void ana_network_manager::run_server(ana::net_id id, int port)
{
    std::stringstream ss;
    ss << port;

    std::set<ana_component*>::iterator it;

    it = std::find_if( components_.begin(), components_.end(),
                        boost::bind(&ana_component::get_id, _1) == id );

    if ( it != components_.end())
        return (*it)->server()->run( ss.str() );
}

std::string ana_network_manager::ip_address( network::connection id )
{
    std::set<ana_component*>::iterator it;

    for (it = components_.begin(); it != components_.end(); ++it)
    {
        if ( (*it)->is_server() )
        {
            const std::string ip = (*it)->server()->ip_address( ana::net_id( id ) );
            if  (ip != "")
                return ip;
        }
    }
    return "";
}

size_t ana_network_manager::number_of_connections() const
{
    return components_.size(); // TODO:check if this is the intention, guessing not
}

size_t ana_network_manager::send_all( const config& cfg, bool zipped )
{
    std::cout << "DEBUG: Sending to everybody...\n";
    std::stringstream out;
    config_writer cfg_writer(out, zipped);
    cfg_writer.write(cfg);

    std::set<ana_component*>::iterator it;

    for (it = components_.begin(); it != components_.end(); ++it)
    {
        if ( (*it)->is_server() )
        {
            const size_t necessary_calls = server_manager_[ (*it)->server() ]->client_amount();

            ana_send_handler handler( *it, out.str().size(), necessary_calls );

            (*it)->server()->send_all( ana::buffer( out.str() ), &handler, ana::ZERO_COPY);

            handler.wait_completion(); // the handler will release the mutex after necessary_calls calls
        }
    }
    return out.str().size();
}

size_t ana_network_manager::send( network::connection connection_num , const config& cfg, bool zipped )
{
    std::cout << "DEBUG: Single send...\n";
    ana::net_id id( connection_num );

    std::stringstream out;
    config_writer cfg_writer(out, zipped);
    cfg_writer.write(cfg);

    std::set<ana_component*>::iterator it;

    for (it = components_.begin(); it != components_.end(); ++it)
    {
        if ( (*it)->is_server() )
        {
            ana_send_handler handler( *it, out.str().size() );
            (*it)->server()->send_one( id, ana::buffer( out.str() ), &handler, ana::ZERO_COPY);

            handler.wait_completion();
        }
    }
    return out.str().size();
}

ana::detail::read_buffer ana_network_manager::read_from( network::connection connection_num )
{
    std::set<ana_component*>::iterator it;

    if ( connection_num == 0 )
    {
        if ( components_.empty() )
            throw std::runtime_error("Trying to read but nothing was running.");

        if ( components_.size() == 1 )
        {
            it = components_.begin();
            return (*it)->wait_for_element();
        }
        else
            throw std::runtime_error("Global Buffer Queue here?");
    }
    else
    {
        ana::net_id id( connection_num );

        std::cout << "DEBUG: Trying to read something from (" << connection_num << " = " << id << ")\n";

        it = std::find_if( components_.begin(), components_.end(),
                        boost::bind(&ana_component::get_id, _1) == id );

        if ( it != components_.end())
            return (*it)->wait_for_element();
        else
            throw std::runtime_error("Trying a network read from an invalid component id.");
    }
}

network::statistics ana_network_manager::get_send_stats(network::connection handle)
{
    std::cout << "DEBUG: in get_send_stats to " << handle << "\n";

    if ( handle != 0 )
    {
        ana::net_id id( handle );
        std::set< ana_component* >::iterator it;

        it = std::find_if( components_.begin(), components_.end(),
                            boost::bind(&ana_component::get_id, _1) == id );

        if ( it != components_.end() )
            return (*it)->get_send_stats( );
        else
            throw std::runtime_error("Received message from a non connected component.");
    }
    else if( ! components_.empty() )
    {
        std::set< ana_component* >::iterator it = components_.begin();
        return (*it)->get_send_stats();
    }
    else
        return network::statistics();
}

network::statistics ana_network_manager::get_receive_stats(network::connection handle)
{
    if ( handle != 0 )
    {
        ana::net_id id( handle );
        std::set< ana_component* >::iterator it;

        it = std::find_if( components_.begin(), components_.end(),
                            boost::bind(&ana_component::get_id, _1) == id );

        if ( it != components_.end() )
            return (*it)->get_receive_stats( );
        else
            throw std::runtime_error("Received message from a non connected component.");
    }
    else if( ! components_.empty() )
    {
        std::set< ana_component* >::iterator it = components_.begin();
        return (*it)->get_receive_stats();
    }
    else
        return network::statistics();
}

void ana_network_manager::handle_send(ana::error_code error_code, ana::net_id client)
{
    if ( error_code )
        network::disconnect( client );
}

void ana_network_manager::handle_message( ana::error_code          error,
                                          ana::net_id              client,
                                          ana::detail::read_buffer buffer)
{
    if (! error)
    {
        std::cout << "DEBUG: Buffer received with size " << buffer->size() << " from " << client << "\n";

        std::set< ana_component* >::iterator it;

        for (it = components_.begin(); it != components_.end(); ++it)
            std::cout << "DEBUG: Component id : " << (*it)->get_id() << "\n";

        it = std::find_if( components_.begin(), components_.end(),
                            boost::bind(&ana_component::get_id, _1) == client );

        if ( it != components_.end() )
        {
            (*it)->update_receive_stats( buffer->size() );
            (*it)->add_buffer( buffer );
        }
        else
            throw std::runtime_error("Received message from a non connected component.");
    }
}

void ana_network_manager::handle_disconnect(ana::error_code /*error_code*/, ana::net_id client)
{
    std::cout << "DEBUG: Disconnected from server.\n";

    std::set< ana_component* >::iterator it;

    it = std::find_if( components_.begin(), components_.end(),
                        boost::bind(&ana_component::get_id, _1) == client );

    if ( it != components_.end() )
    {
        std::cout << "DEBUG: Removing bad component.\n";
        components_.erase(it);
    }
}
