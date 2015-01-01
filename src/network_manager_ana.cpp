
/**
 * @file
 * @brief Implementation file for network features using ana.
 *
 * Copyright (C) 2010 - 2015 Guillermo Biset.
 *
 * Part of the Battle for Wesnoth Project http://www.wesnoth.org/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY.
 *
 * See the COPYING file for more details.
 */

#include <iostream>

#include <boost/bind.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include "serialization/parser.hpp"

#include "network_manager_ana.hpp"
#include "serialization/binary_or_text.hpp"

#include "gettext.hpp"

// Begin ana_send_handler implementation ----------------------------------------------------------

ana_send_handler::ana_send_handler( size_t calls ) :
    mutex_(),
    target_calls_( calls ),
    error_code_()
{
    if ( calls > 0 )
        mutex_.lock();
}

ana_send_handler::~ana_send_handler()
{
    if ( target_calls_ > 0 )
        throw std::runtime_error("Handler wasn't called enough times.");
    mutex_.lock();
    mutex_.unlock();
}

void ana_send_handler::handle_send(ana::error_code   error_code,
                                   ana::net_id       /*client*/,
                                   ana::operation_id /*op_id*/)
{
    error_code_ = error_code;

    if ( --target_calls_ == 0 )
        mutex_.unlock();
}

void ana_send_handler::wait_completion()
{
    mutex_.lock();
    mutex_.unlock();
}

// Begin ana_handshake_finisher_handler implementation --------------------------------------------

ana_handshake_finisher_handler::ana_handshake_finisher_handler( ana::server*     server,
                                                                clients_manager* mgr)
    : server_( server ),
      manager_( mgr )
{
}

ana_handshake_finisher_handler::~ana_handshake_finisher_handler()
{
}

void ana_handshake_finisher_handler::handle_send(ana::error_code   ec,
                                                 ana::net_id       client,
                                                 ana::operation_id /*op_id*/)
{
    if ( ec )
        server_->disconnect( client );
    else
        manager_->connected( client );

    delete this;
}

// Begin ana_receive_handler implementation -------------------------------------------------------

ana_receive_handler::ana_receive_handler( ana_component_set::iterator iterator ) :
    iterator_( iterator ),
    mutex_(),
    handler_mutex_(),
    timeout_called_mutex_(),
    error_code_(),
    receive_timer_( NULL ),
    finished_( false )
{
    mutex_.lock();
    timeout_called_mutex_.lock();
}

ana_receive_handler::~ana_receive_handler()
{
    timeout_called_mutex_.lock();
    timeout_called_mutex_.unlock();
    handler_mutex_.lock();
    handler_mutex_.unlock();
}

void ana_receive_handler::wait_completion(ana::detail::timed_sender* component, size_t timeout_ms )
{
    {
        boost::mutex::scoped_lock lock( handler_mutex_);
        if ( finished_ )
        {
            mutex_.unlock();
            timeout_called_mutex_.unlock();
        }
        else if ( timeout_ms > 0 )
        {
            receive_timer_ = component->create_timer();

            receive_timer_->wait( ana::time::milliseconds(timeout_ms),
                                boost::bind(&ana_receive_handler::handle_timeout,
                                            this,
                                            ana::timeout_error ) );
        }
    }

    mutex_.lock();
    mutex_.unlock();
}

void ana_receive_handler::handle_receive(ana::error_code          error_c,
                                         ana::net_id              client,
                                         ana::read_buffer read_buffer)
{
    boost::mutex::scoped_lock lock( handler_mutex_);

    delete receive_timer_;
    receive_timer_ = NULL;

    (*iterator_)->add_buffer( read_buffer, client );

    error_code_ = error_c;

    if (! finished_ )
    {
        finished_ = true;
        mutex_.unlock();
    }
}

void ana_receive_handler::handle_disconnect(ana::error_code error_c, ana::net_id)
{
    boost::mutex::scoped_lock lock( handler_mutex_);

    delete receive_timer_;
    receive_timer_ = NULL;

    if (! finished_ )
    {
        error_code_ = error_c;

        finished_ = true;
        mutex_.unlock();
    }
}

void ana_receive_handler::handle_timeout(ana::error_code error_code)
{
    boost::mutex::scoped_lock lock( handler_mutex_ );

    delete receive_timer_;
    receive_timer_ = NULL;

    if (! finished_ )
    {
        error_code_ = error_code;

        finished_ = true;
        mutex_.unlock();
    }

    timeout_called_mutex_.unlock();
}

// Begin ana_multiple_receive_handler implementation ----------------------------------------------

ana_multiple_receive_handler::ana_multiple_receive_handler( ana_component_set& components ) :
    components_( components ),
    mutex_(),
    handler_mutex_(),
    timeout_called_mutex_(),
    error_code_(),
    buffer_(),
    wesnoth_id_(0),
    receive_timer_( NULL ),
    finished_( false )
{
    throw std::runtime_error("Multiple receive handler constructed");

    ana_component_set::iterator it;

    for (it = components_.begin(); it != components_.end(); ++it )
    {
        if ( (*it)->is_server() )
            (*it)->server()->set_listener_handler( this );
        else
            (*it)->client()->set_listener_handler( this );
    }

    mutex_.lock();
    timeout_called_mutex_.lock();
}

ana_multiple_receive_handler::~ana_multiple_receive_handler()
{
    timeout_called_mutex_.lock();
    timeout_called_mutex_.unlock();
    handler_mutex_.lock();
    handler_mutex_.unlock();
}

void ana_multiple_receive_handler::wait_completion(size_t timeout_ms )
{
    ana_component_set::iterator it;
    {
        boost::mutex::scoped_lock lock( handler_mutex_);

        it = components_.begin();

        ana::detail::timed_sender* component;

        if ( (*it)->is_server())
            component = (*it)->server();
        else
            component = (*it)->client();

        if ( finished_ )
        {
            mutex_.unlock();
            timeout_called_mutex_.unlock();
        }
        else if ( timeout_ms > 0 )
        {
            receive_timer_ = component->create_timer();

            receive_timer_->wait( ana::time::milliseconds(timeout_ms),
                                boost::bind(&ana_multiple_receive_handler::handle_timeout,
                                            this,
                                            ana::timeout_error ) );
        }
    }
    mutex_.lock();
    mutex_.unlock();
}

void ana_multiple_receive_handler::handle_receive(ana::error_code          error_c,
                                                  ana::net_id              id,
                                                  ana::read_buffer read_buffer)
{
    boost::mutex::scoped_lock lock( handler_mutex_);

    delete receive_timer_;
    receive_timer_ = NULL;

    buffer_ = read_buffer;
    error_code_ = error_c;

    ana_component_set::iterator it;
    it = std::find_if( components_.begin(), components_.end(),
                    boost::bind(&ana_component::get_id, _1) == id );

    if ( it != components_.end())
        wesnoth_id_ = (*it)->get_wesnoth_id();
    else
        throw std::runtime_error("Wrong read.");


    if (! finished_ )
    {
        finished_ = true;
        mutex_.unlock();
    }
}

void ana_multiple_receive_handler::handle_disconnect(ana::error_code error_c, ana::net_id)
{
    boost::mutex::scoped_lock lock( handler_mutex_);

    delete receive_timer_;
    receive_timer_ = NULL;

    error_code_ = error_c;
    if (! finished_ )
    {
        finished_ = true;
        mutex_.unlock();
    }
}

void ana_multiple_receive_handler::handle_timeout(ana::error_code error_code)
{
    boost::mutex::scoped_lock lock( handler_mutex_ );

    delete receive_timer_;
    receive_timer_ = NULL;

    if (! finished_ )
    {
        error_code_ = error_code;
        finished_ = true;
        mutex_.unlock();
    }

    timeout_called_mutex_.unlock();
}

// Begin ana_connect_handler implementation -------------------------------------------------------

ana_connect_handler::ana_connect_handler( ) :
    mutex_( ),
    error_code_()
{
    mutex_.lock();
}

ana_connect_handler::~ana_connect_handler()
{
    mutex_.lock();
    mutex_.unlock();
}

const ana::error_code& ana_connect_handler::error() const
{
    return error_code_;
}

void ana_connect_handler::handle_connect(ana::error_code error_code, ana::net_id /*client*/)
{
    error_code_ = error_code;
    mutex_.unlock();
}

void ana_connect_handler::wait_completion()
{
    mutex_.lock();
    mutex_.unlock();
}

// Begin ana_component implementation -------------------------------------------------------------

ana_component::ana_component( ) :
    base_( ana::server::create() ),
    is_server_( true ),
    id_( server()->id() ),
    wesnoth_id_( 0 ),
    mutex_(),
    condition_(),
    buffers_(),
    sender_ids_()
{
}

ana_component::ana_component( const std::string& host, const std::string& port) :
    base_( ana::client::create(host,port) ),
    is_server_( false ),
    id_(  client()->id() ),
    wesnoth_id_( 0 ),       // will change to the received id after connection
    mutex_(),
    condition_(),
    buffers_(),
    sender_ids_()
{
}

ana_component::~ana_component( )
{
    if ( is_server() )
        delete server();
    else
        delete client();
}

network::statistics ana_component::get_send_stats() const
{
    ana::stats_collector& stats = listener()->stats_collector();

    network::statistics result;

    result.current     = stats.current_packet_out_total();
    result.current_max = stats.current_packet_out_size();
    result.total       = stats.get_stats( ana::ACCUMULATED )->bytes_out();

    return result;
}

network::statistics ana_component::get_receive_stats() const
{
    ana::stats_collector& stats = listener()->stats_collector();

    network::statistics result;

    result.current     = stats.current_packet_in_total();
    result.current_max = stats.current_packet_in_size();
    result.total       = stats.get_stats( ana::ACCUMULATED )->bytes_in();

    return result;
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

ana::detail::listener* ana_component::listener() const
{
    if( is_server_ )
        return server();
    else
        return client();
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

network::connection ana_component::get_wesnoth_id() const
{
    return wesnoth_id_;
}

void ana_component::set_wesnoth_id( network::connection id )
{
    wesnoth_id_ = id;
}

const ana::stats* ana_component::get_stats( ana::stat_type type ) const
{
    return listener()->get_stats( type );
}

void ana_component::add_buffer(ana::read_buffer buffer, ana::net_id id)
{
    {
        boost::lock_guard<boost::mutex> lock(mutex_);
        buffers_.push( buffer );

        if ( is_server_ )
            sender_ids_.push( id );
    }
    condition_.notify_all();
}

ana::read_buffer ana_component::wait_for_element()
{
    boost::unique_lock<boost::mutex> lock(mutex_);

    while(buffers_.empty())
        condition_.wait(lock);

    const ana::read_buffer buffer_ret = buffers_.front();

    buffers_.pop();

    return buffer_ret;
}

network::connection ana_component::oldest_sender_id_still_pending()
{
    boost::unique_lock<boost::mutex> lock(mutex_);

    if ( sender_ids_.empty())
        throw std::runtime_error("No pending buffer.");

    const network::connection id = sender_ids_.front();

    sender_ids_.pop();

    return id;
}

// Begin clients_manager  implementation ----------------------------------------------------------

clients_manager::clients_manager( ana::server* server) :
    server_( server ),
    ids_(),
    pending_ids_(),
    pending_handshakes_()
{
}

size_t clients_manager::client_amount() const
{
    return ids_.size();
}

void clients_manager::handle_connect(ana::error_code error, ana::net_id client)
{
    if (! error )
    {
        ids_.insert( client );
        pending_handshakes_.insert( client );
    }
}

void clients_manager::handle_disconnect(ana::error_code /*error*/, ana::net_id client)
{
    ids_.erase(client);
    pending_ids_.erase( network::connection( client ) );
}

void clients_manager::connected( ana::net_id id )
{
    pending_ids_.insert( network::connection( id ) );
}

void clients_manager::remove( ana::net_id id )
{
    ids_.erase( id );
    pending_ids_.erase( network::connection( id ) );
    pending_handshakes_.erase( id );
}


void clients_manager::handshaked( ana::net_id id )
{
    pending_handshakes_.erase( id );
}

bool clients_manager::has_connection_pending() const
{
    return ! pending_ids_.empty();
}

bool clients_manager::is_pending_handshake( ana::net_id id ) const
{
    return pending_handshakes_.find( id ) != pending_handshakes_.end();
}

bool clients_manager::is_a_client( ana::net_id id ) const
{
    return ids_.find( id ) != ids_.end();
}

network::connection clients_manager::get_pending_connection_id()
{
    const network::connection result = *pending_ids_.begin();
    pending_ids_.erase( pending_ids_.begin() );
    return result;
}

// Begin ana_network_manager implementation -------------------------------------------------------

ana_network_manager::ana_network_manager() :
    connect_timer_( NULL ),
    components_(),
    server_manager_(),
    disconnected_components_(),
    disconnected_ids_(),
    proxy_settings_()
{
}

ana::net_id ana_network_manager::create_server( )
{
    ana_component* new_component = new ana_component( );
    components_.insert( new_component );

    ana::server* server = new_component->server();

    clients_manager* manager = new clients_manager( server );
    server_manager_[ server ] = manager;

    server->set_connection_handler( manager );
    server->set_listener_handler( this );
    server->set_raw_data_mode();

    return server->id();
}

network::connection ana_network_manager::create_client_and_connect(std::string host, int port)
{
    ana::net_id new_client_id = ana::invalid_net_id;

    try
    {
        std::stringstream ss;
        ss << port;

        ana_component* new_component = new ana_component( host, ss.str() );
        components_.insert( new_component );

        ana::client* const client = new_component->client();

        new_client_id = client->id();

        ana_connect_handler handler;

        client->set_raw_data_mode();
        client->set_connect_timeout( ana::time::seconds(10) );

        if ( proxy_settings_.enabled )
            client->connect_through_proxy( proxy_settings_.address,
                                           proxy_settings_.port,
                                           &handler,
                                           proxy_settings_.user,
                                           proxy_settings_.password);
        else
            client->connect( &handler );

        client->set_listener_handler( this );
        client->run();

        handler.wait_completion(); // just wait for handler to finish

        if( handler.error() )
        {
            network::disconnect( client->id() );
            throw network::error(_("Could not connect to host"), client->id() );
        }
        else
        {
            //Send handshake
            ana::serializer::bostream bos;

            ana::ana_uint32 handshake( 0 );
            bos << handshake;

            ana_send_handler send_handler;

            client->send( ana::buffer( bos.str()), &send_handler); //, ana::ZERO_COPY );

            send_handler.wait_completion();

            if ( send_handler.error() )
                throw network::error(_("Could not connect to host"), client->id() );
            else
            {
                ana::ana_uint32 my_id;
                ana::serializer::bistream bis;

                client->wait_raw_object(bis, sizeof(ana::ana_uint32) );

                bis >> my_id;
                ana::network_to_host_long( my_id );

                new_component->set_wesnoth_id( my_id );

                client->set_header_first_mode();
                client->run_listener();

                return network::connection( client->id() );
            }
        }
    }
    catch( const std::exception& )
    {
        throw network::error(_("Could not connect to host"), new_client_id );
        return 0;
    }
}

network::connection ana_network_manager::new_connection_id( )
{
    ana_component_set::iterator it;

    for (it = components_.begin(); it != components_.end(); ++it)
    {
        if ( (*it)->is_server() )
        {
            clients_manager* clients_mgr = server_manager_[ (*it)->server() ];
            if ( clients_mgr->has_connection_pending() )
                return clients_mgr->get_pending_connection_id();
        }
    }

    // No new connection
    return 0;
}


const ana::stats* ana_network_manager::get_stats( network::connection connection_num,
                                                  ana::stat_type      type)
{
    ana::net_id id( connection_num );
    std::set<ana_component*>::iterator it;

    if ( id == 0 )
    {
        if ( ! components_.empty() )
        {
            it = components_.begin();
            return (*it)->get_stats( type );
        }
        else
            return NULL;
    }
    else
    {
        it = std::find_if( components_.begin(), components_.end(),
                           boost::bind(std::logical_or<bool>(),
                           (boost::bind(&ana_component::get_wesnoth_id, _1) == connection_num),
                           (boost::bind(&ana_component::get_id, _1) == id ) ));
        //Make a broad attempt at finding it, test for both ANA's id and the assigned one.

        if ( it != components_.end())
            return (*it)->get_stats( type );
        else
        {
            for ( it = components_.begin() ; it != components_.end(); ++it)
            {
                if ( (*it)->is_server() )
                {
                    const ana::stats* res = (*it)->server()->get_client_stats(id,ana::ACCUMULATED);
                    if ( res != NULL )
                        return res;
                }
            }
        }

        return NULL;
    }
}

void ana_network_manager::close_connections_and_cleanup()
{
    for (ana_component_set::iterator it = components_.begin(); it != components_.end(); ++it)
        delete *it;

    std::map< ana::server*, clients_manager* >::iterator it;
    for ( it = server_manager_.begin(); it != server_manager_.end(); ++it)
        delete it->second;

    components_.clear();
    server_manager_.clear();
}

void ana_network_manager::throw_if_pending_disconnection()
{
    if ( ! disconnected_components_.empty() )
    {
        ana_component* component = disconnected_components_.front();
        disconnected_components_.pop();

        const ana::net_id id = component->get_id(); // Should I use wesnoth_id here?

        delete component;

        throw network::error(_("Client disconnected"),id);
    }

    if ( ! disconnected_ids_.empty() )
    {
        ana::net_id id = disconnected_ids_.front();
        disconnected_ids_.pop();
        throw network::error(_("Client disconnected"),id);
    }
}

void ana_network_manager::run_server(ana::net_id id, int port)
{
    std::stringstream ss;
    ss << port;

    ana_component_set::iterator it;

    it = std::find_if( components_.begin(), components_.end(),
                        boost::bind(&ana_component::get_id, _1) == id );

    if ( it == components_.end())
        throw std::runtime_error("No server with this id.");
    else
        if ( (*it)->is_server() )
            (*it)->server()->run( ss.str() );
        else
            throw std::runtime_error("Component is not a server.");

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
    // TODO:check if this is the intention

    size_t total(0);

    ana_component_set::const_iterator it;

    for (it = components_.begin(); it != components_.end(); ++it )
    {
        if ((*it)->is_client())
            ++total;
        else
        {
            std::map< ana::server*, clients_manager* >::const_iterator mgr = server_manager_.find(
(*it)->server() );
            total += mgr->second->client_amount();
        }
    }

    return total;
}


std::string ana_network_manager::compress_config( const config& cfg )
{
    std::ostringstream out;
    compress_config( cfg, out );
    return out.str( );
}


void ana_network_manager::compress_config( const config& cfg, std::ostringstream& out)
{
    boost::iostreams::filtering_stream<boost::iostreams::output> filter;
    filter.push(boost::iostreams::gzip_compressor());
    filter.push(out);
    write(filter, cfg);
    out.flush();
}


void ana_network_manager::read_config( const ana::read_buffer& buffer, config& cfg)
{
    std::istringstream input( buffer->string() );

    read_gz(cfg, input);
}

size_t ana_network_manager::send_all( const config& cfg )
{
    const std::string output_string = compress_config(cfg);

    std::set<ana_component*>::iterator it;

    for (it = components_.begin(); it != components_.end(); ++it)
    {
        if ( (*it)->is_server() )
        {
            const size_t necessary_calls = server_manager_[ (*it)->server() ]->client_amount();
            ana_send_handler handler( necessary_calls );

            (*it)->server()->send_all( ana::buffer( output_string ), &handler); //, ana::ZERO_COPY);
            handler.wait_completion(); // the handler will release the mutex after
                                       // necessary_calls calls
        }
        else
        {
            ana_send_handler handler;

            (*it)->client()->send( ana::buffer( output_string ), &handler); //, ana::ZERO_COPY );
            handler.wait_completion();
        }
    }
    return output_string.size();
}

size_t ana_network_manager::send( network::connection connection_num ,
                                  const config&       cfg )
{
    const std::string output_string = compress_config(cfg);


    return send_raw_data( output_string.c_str(), output_string.size(), connection_num );
}

size_t ana_network_manager::send_raw_data( const char*         base_char,
                                           size_t              size,
                                           network::connection connection_num )
{
    ana::net_id id( connection_num );
    ana_component_set::iterator it;

    it = std::find_if( components_.begin(), components_.end(),
                   boost::bind(std::logical_or<bool>(),
                       (boost::bind(&ana_component::get_wesnoth_id, _1) == connection_num),
                       (boost::bind(&ana_component::get_id, _1) == id ) ));
    //Make a broad attempt at finding it, test for both ANA's id and the assigned one.

    if ( it != components_.end())
    {
        if ( (*it)->is_server() )
            throw std::runtime_error("Can't send to the server itself.");

        ana_send_handler handler;
        (*it)->client()->send( ana::buffer( base_char, size ), &handler); //, ana::ZERO_COPY);
        handler.wait_completion();

        if ( handler.error() )
            return 0;
        else
            return size;
    }
    else
    {
        if ( components_.empty() )
            return 0;
        else
        {
            it = components_.begin();

            if ((*it)->is_server())
            {
                ana_send_handler handler;                   //, ana::ZERO_COPY);
                (*it)->server()->send_one( id, ana::buffer( base_char, size ), &handler);
                handler.wait_completion();
                if ( handler.error() )
                    return 0;
                else
                    return size;
            }
            else
            {
                ana_send_handler handler;
                (*it)->client()->send( ana::buffer( base_char, size ), &handler);
                handler.wait_completion();

                if ( handler.error() )
                    return 0;
                else
                    return size;
            }
        }
    }
}

void ana_network_manager::send_all_except(const config& cfg, network::connection connection_num)
{
    const std::string output_string = compress_config(cfg);


    ana_component_set::iterator it;

    ana::net_id id_to_avoid( connection_num ); //I should have issued this id earlier

    for ( it = components_.begin(); it != components_.end(); ++it)
    {
        if ((*it)->is_server())
        {
            if ( (*it)->get_id() != id_to_avoid )
            {
                if ( server_manager_[ (*it)->server() ]->is_a_client( id_to_avoid ) )
                {
                    const size_t clients_receiving_number
                                = server_manager_[ (*it)->server() ]->client_amount() - 1;

                    ana_send_handler handler( clients_receiving_number );
                    (*it)->server()->send_all_except( id_to_avoid,
                                                      ana::buffer( output_string ),
                                                      &handler); //, ana::ZERO_COPY);
                    handler.wait_completion();
                }
            }
        }
        else
        {
            if ( (*it)->get_wesnoth_id() != connection_num )
            {
                ana_send_handler handler;
                (*it)->client()->send( ana::buffer( output_string ), &handler); //, ana::ZERO_COPY);
                handler.wait_completion();
            }
        }
    }
}

network::connection ana_network_manager::read_from_ready_buffer(
                                         const ana_component_set::iterator& it, config& cfg)
{
    read_config( (*it)->wait_for_element(), cfg);

    return (*it)->get_wesnoth_id();
}

network::connection ana_network_manager::read_from( const ana_component_set::iterator& it,
                                                    config&             cfg,
                                                    size_t              timeout_ms)
{
    if (  (*it)->new_buffer_ready() )
        return read_from_ready_buffer( it, cfg );
    else if (timeout_ms == 0 )
        return 0;
    else
    {
        ana_receive_handler handler(it);
        (*it)->listener()->set_listener_handler( &handler );

        if (  (*it)->new_buffer_ready() )
            return read_from_ready_buffer( it, cfg );
        else
        {
            if ( (*it)->is_server() )
                handler.wait_completion( (*it)->server(), timeout_ms );
            else
                if ( (*it)->get_wesnoth_id() != 0 )
                    handler.wait_completion( (*it)->client(), timeout_ms );
                else
                    return 0;
                //Don't try to read from a still unconnected client
        }

        (*it)->listener()->set_listener_handler( this );

        if ( handler.error() )
            return 0;
        else
            return read_from_ready_buffer( it, cfg );
    }
}

network::connection ana_network_manager::read_from( network::connection connection_num,
                                                    config&             cfg,
                                                    size_t              timeout_ms)
{
    if ( components_.empty() )
        return 0;

    ana_component_set::iterator it;

    if ( connection_num == 0 )
    {
        if ( components_.size() == 1 )
            return read_from( components_.begin(), cfg, timeout_ms );
        else
        {
            //Check first if there is an available buffer
            for (it = components_.begin(); it != components_.end(); ++it)
                if (  (*it)->new_buffer_ready() )
                    return read_from_ready_buffer( it, cfg );

            // If no timeout was requested, return
            if (timeout_ms == 0 )
                return 0;

            // Wait timeout_ms milliseconds to see if any component will receive something
            ana_multiple_receive_handler handler( components_ );

            for (it = components_.begin(); it != components_.end(); ++it )
                (*it)->listener()->set_listener_handler( &handler );

            handler.wait_completion( timeout_ms );

            for (it = components_.begin(); it != components_.end(); ++it )
                (*it)->listener()->set_listener_handler( this );

            if ( handler.error() )
            {
                for (it = components_.begin(); it != components_.end(); ++it)
                    if (  (*it)->new_buffer_ready() )
                        return read_from_ready_buffer( it, cfg );

                // So nothing was read:
                return 0;
            }
            else
            {
                read_config( handler.buffer(), cfg);
                return handler.get_wesnoth_id();
            }
        }
    }
    else
    {
        ana::net_id id( connection_num );

        it = std::find_if( components_.begin(), components_.end(),
                    boost::bind(std::logical_or<bool>(),
                        (boost::bind(&ana_component::get_wesnoth_id, _1) == connection_num),
                        (boost::bind(&ana_component::get_id, _1) == id ) ) );
        //Make a broad attempt at finding it, test for both ANA's id and the assigned one.


        if ( it != components_.end())
            return read_from(it, cfg, timeout_ms);
        else
            throw std::runtime_error("Trying a network read from an invalid component id.");
    }
}

network::connection ana_network_manager::read_from_all( std::vector<char>& vec)
{
    ana_component_set::iterator it;

    if ( components_.empty() )
        return 0;

    for (it = components_.begin(); it != components_.end(); ++it)
    {
        if (  (*it)->new_buffer_ready() )
        {
            ana::read_buffer buffer = (*it)->wait_for_element();

            char* ch = buffer->base_char();
            for (size_t i = 0; i < buffer->size(); ++i)  // copy the buffer
                vec.push_back( *(ch++) );

            if ( (*it)->is_client() )
                return (*it)->get_wesnoth_id();
            else
                return (*it)->oldest_sender_id_still_pending();
        }
    }

    // there wasn't any buffer ready
    return 0;
}

network::statistics ana_network_manager::get_send_stats(network::connection handle)
{
    if ( handle != 0 )
    {
//         ana::net_id id( handle );
        std::set< ana_component* >::iterator it;

        it = std::find_if( components_.begin(), components_.end(),
                            boost::bind(&ana_component::get_wesnoth_id, _1) == handle );

        if ( it != components_.end() )
            return (*it)->get_send_stats( );
        else
            throw std::runtime_error("Trying to get stats from the wrong component.");
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

bool ana_network_manager::disconnect( network::connection handle)
{
    if ( handle == 0 )
        close_connections_and_cleanup();
    else
    {
        ana::net_id id( handle );
        ana_component_set::iterator it;

        it = std::find_if( components_.begin(), components_.end(),
                       boost::bind(std::logical_or<bool>(),
                           (boost::bind(&ana_component::get_wesnoth_id, _1) == handle),
                           (boost::bind(&ana_component::get_id, _1) == id ) ));
        //Make a broad attempt at finding it, test for both ANA's id and the assigned one.

        if ( it == components_.end())
            throw std::runtime_error("Trying to disconnect an invalid component.");
        else
        {
            if ( (*it)->is_server() )
                throw std::runtime_error("Can't disconnect server directly.");
            else
                (*it)->client()->disconnect();
        }
    }
    return true;
}

// --- Proxy methods
void ana_network_manager::enable_connection_through_proxy()
{
    proxy_settings_.enabled = true;
}

void ana_network_manager::set_proxy_address ( const std::string& address  )
{
    proxy_settings_.address = address;
}

void ana_network_manager::set_proxy_port    ( const std::string& port     )
{
    proxy_settings_.port = port;
}

void ana_network_manager::set_proxy_user    ( const std::string& user     )
{
    proxy_settings_.user = user;
}

void ana_network_manager::set_proxy_password( const std::string& password )
{
    proxy_settings_.password = password;
}
// --- End Proxy methods


void ana_network_manager::handle_send(ana::error_code error_code,
                                      ana::net_id client,
                                      ana::operation_id /*op_id*/)
{
    if ( error_code )
        network::disconnect( client );
}

void ana_network_manager::handle_receive( ana::error_code          error,
                                          ana::net_id              client,
                                          ana::read_buffer buffer)
{
    if (error)
        network::disconnect( client );
    else
    {
        std::set< ana_component* >::iterator it;

        it = std::find_if( components_.begin(), components_.end(),
                            boost::bind(&ana_component::get_id, _1) == client );

        if ( it != components_.end() )
            (*it)->add_buffer( buffer, client );
        else
        {
            if (components_.empty() )
                throw std::runtime_error("Received a message while no component was running.\n");

            std::map< ana::server*, clients_manager* >::iterator mgrs;

            for ( mgrs = server_manager_.begin(); mgrs != server_manager_.end(); ++mgrs)
            {
                if (mgrs->second->is_a_client( client ) ) // Is this your client?
                {
                    if ( mgrs->second->is_pending_handshake( client ) ) // Did he login already?
                    {
                        // all handshakes are 4 bytes long
                        if ( buffer->size() != sizeof(ana::ana_uint32) )
                            mgrs->first->disconnect( client );

                        ana::ana_uint32 handshake;
                        {
                            ana::serializer::bistream bis( buffer->string() );

                            bis >> handshake;
                            ana::network_to_host_long( handshake ); //I'm expecting a 0 anyway
                        }

                        if ( handshake != 0 )
                            mgrs->first->disconnect( client );
                        else
                        {
                            mgrs->second->handshaked( client );
                            //send back it's id
                            ana::serializer::bostream bos;
                            ana::ana_uint32 network_byte_order_id = client;
                            ana::host_to_network_long( network_byte_order_id );
                            bos << network_byte_order_id;

                            ana_handshake_finisher_handler* handler
                                = new ana_handshake_finisher_handler( mgrs->first,
                                                                      mgrs->second);

                            mgrs->first->send_one(client, ana::buffer( bos.str() ), handler );
                            mgrs->first->set_header_first_mode( client );
                        }
                    }
                    else // just add the buffer to the associated clients_manager
                    {
                        ana::net_id server_id = mgrs->first->id();

                        it = std::find_if( components_.begin(), components_.end(),
                                        boost::bind(&ana_component::get_id, _1) == server_id );

                        if ( (*it)->is_client() )
                            throw std::runtime_error("Wrong id to receive from.");

                        (*it)->add_buffer( buffer, client );
                    }
                }
            }
        }
    }
}

bool ana_component::new_buffer_ready() // non const due to mutex block
{
    boost::mutex::scoped_lock lock( mutex_ );

    return ! buffers_.empty();
}


void ana_network_manager::handle_disconnect(ana::error_code /*error_code*/, ana::net_id client)
{
    std::set< ana_component* >::iterator it;

    it = std::find_if( components_.begin(), components_.end(),
                        boost::bind(&ana_component::get_id, _1) == client );

    if ( it != components_.end() )
    {
        disconnected_components_.push( *it );
        components_.erase(it);
    }
    else
    {
        for (it = components_.begin(); it != components_.end(); ++it )
            if ( (*it)->is_server() )
                if ( server_manager_[ (*it)->server() ]->is_a_client( client ) )
                {
                    server_manager_[ (*it)->server() ]->remove( client );
                    disconnected_ids_.push( client );
                }
    }
}
