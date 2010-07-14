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
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include "serialization/parser.hpp"

#include "network_manager_ana.hpp"
#include "serialization/binary_or_text.hpp"

// Begin ana_send_handler implementation ---------------------------------------------------------------

ana_send_handler::ana_send_handler( size_t buf_size, size_t calls) :
    mutex_(),
    target_calls_( calls ),
    error_code_(),
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
    mutex_.lock();
    mutex_.unlock();
}

void ana_send_handler::handle_send(ana::error_code error_code, ana::net_id /*client*/)
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

// Begin ana_receive_handler implementation ------------------------------------------------------------

ana_receive_handler::ana_receive_handler( ) :
    mutex_(),
    handler_mutex_(),
    timeout_called_mutex_(),
    error_code_(),
    buffer_(),
    receive_timer_( NULL ),
    finished_( false )
{
//     std::cout << "DEBUG: Constructing a new ana_receive_handler...\n"; // too much output
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
                                boost::bind(&ana_receive_handler::handle_timeout, this, ana::timeout_error ) );
        }
    }

    mutex_.lock();
    mutex_.unlock();
}

void ana_receive_handler::handle_message(ana::error_code error_c, ana::net_id, ana::detail::read_buffer read_buffer)
{
    boost::mutex::scoped_lock lock( handler_mutex_);

    delete receive_timer_;
    receive_timer_ = NULL;

    buffer_ = read_buffer;
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

    error_code_ = error_c;
    if (! finished_ )
    {
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
//         if (error_code)
//             std::cout << "DEBUG: Receive attempt timed out\n";
//         else
//             std::cout << "DEBUG: Shouldn't reach here\n";

        error_code_ = error_code;
        finished_ = true;
        mutex_.unlock();
    }

    timeout_called_mutex_.unlock();
}

// Begin ana_multiple_receive_handler implementation ------------------------------------------------------------

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
    std::cout << "DEBUG: Constructing a new ana_multiple_receive_handler...\n";

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
                                boost::bind(&ana_multiple_receive_handler::handle_timeout, this, ana::timeout_error ) );
        }
    }
    mutex_.lock();
    mutex_.unlock();
}

void ana_multiple_receive_handler::handle_message(ana::error_code          error_c,
                                                  ana::net_id              id,
                                                  ana::detail::read_buffer read_buffer)
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
        if (error_code)
            std::cout << "DEBUG: Receive attempt timed out\n";
        else
            std::cout << "DEBUG: Shouldn't reach here\n";

        error_code_ = error_code;
        finished_ = true;
        mutex_.unlock();
    }

    timeout_called_mutex_.unlock();
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
    boost::mutex::scoped_lock lock(handler_mutex_);

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
    mutex_.lock();
    mutex_.unlock();
    handler_mutex_.lock();
    handler_mutex_.unlock();
}

const ana::error_code& ana_connect_handler::error() const
{
    return error_code_;
}

void ana_connect_handler::handle_connect(ana::error_code error_code, ana::net_id /*client*/)
{
    boost::mutex::scoped_lock lock(handler_mutex_);
    connected_ = true;

    if (! error_code)
        std::cout << "DEBUG: Connected.\n";
    else
        std::cout << "DEBUG: Can't connect. " << error_code << "\n";

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
    ana::stats_collector* stats = listener()->stats_collector();

    network::statistics result;

    if ( stats != NULL )
    {
        result.current     = stats->current_packet_out_total();
        result.current_max = stats->current_packet_out_size();
        result.total       = stats->get_stats( ana::ACCUMULATED )->bytes_out();
    }

    return result;
}

network::statistics ana_component::get_receive_stats() const
{
    ana::stats_collector* stats = listener()->stats_collector();

    network::statistics result;

    if ( stats != NULL )
    {
        result.current     = stats->current_packet_in_total();
        result.current_max = stats->current_packet_in_size();
        result.total       = stats->get_stats( ana::ACCUMULATED )->bytes_in();
    }

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

const ana::stats* ana_component::get_stats() const
{
    return listener()->get_stats();
}

void ana_component::add_buffer(ana::detail::read_buffer buffer, ana::net_id id)
{
    {
        boost::lock_guard<boost::mutex> lock(mutex_);
        buffers_.push( buffer );

        if ( is_server_ )
            sender_ids_.push( id );
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

network::connection ana_component::oldest_sender_id_still_pending()
{
    boost::unique_lock<boost::mutex> lock(mutex_);

    if ( sender_ids_.empty())
        throw std::runtime_error("No pending buffer.");

    const network::connection id = sender_ids_.front();

    sender_ids_.pop();

    return id;
}

// Begin clients_manager  implementation ---------------------------------------------------------------

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
    std::cout << "New client connected with id " << client << "\n";

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

void clients_manager::has_connected( ana::net_id id )
{
    pending_ids_.insert( network::connection( id ) );
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

    clients_manager* manager = new clients_manager( server );
    server_manager_[ server ] = manager;

    server->set_connection_handler( manager );
    server->set_listener_handler( this );
    server->set_raw_data_mode();

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

        connect_timer_ = client->create_timer();

        ana_connect_handler handler(connect_timer_);

        connect_timer_->wait( ana::time::seconds(10), // 10 seconds to connection timeout, TODO: configurable
                            boost::bind(&ana_connect_handler::handle_timeout, &handler, ana::timeout_error) );

        client->set_raw_data_mode();
        client->connect( &handler );
        client->set_listener_handler( this );
        client->run();

        client->start_logging();

        handler.wait_completion(); // just wait for handler to finish

        delete connect_timer_;

        if( handler.error() )
        {
            network::disconnect( client->id() );
            return 0;
        }
        else
        {
            //Send handshake
            ana::serializer::bostream bos;

            uint32_t handshake( 0 );
            bos << handshake;

            ana_send_handler send_handler( bos.str().size() );

            client->send( ana::buffer( bos.str()), &send_handler, ana::ZERO_COPY );

            send_handler.wait_completion();

            if ( send_handler.error() )
                return 0;
            else
            {
                uint32_t my_id;
                ana::serializer::bistream bis;

                client->wait_raw_object(bis, sizeof(my_id) );

                bis >> my_id;
                ana::network_to_host_long( my_id );
                std::cout << "DEBUG: Received id " << my_id << "\n";

                new_component->set_wesnoth_id( my_id );

                client->set_header_first_mode();
                client->run_listener();

                return network::connection( client->id() );
            }
        }
    }
    catch( const std::exception& e )
    {
        std::cout << "DEBUG: Caught an exception while trying to connect.\n";
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
            std::map< ana::server*, clients_manager* >::const_iterator mgr = server_manager_.find( (*it)->server() );
            total += mgr->second->client_amount();
        }
    }

    return total;
}

void ana_network_manager::compress_config( const config& cfg, std::ostringstream& out)
{
    boost::iostreams::filtering_stream<boost::iostreams::output> filter;
    filter.push(boost::iostreams::gzip_compressor());
    filter.push(out);
    write(filter, cfg);
}

void ana_network_manager::read_config( const ana::detail::read_buffer& buffer, config& cfg)
{
    std::istringstream input( buffer->string() );

    read_gz(cfg, input);
}

size_t ana_network_manager::send_all( const config& cfg, bool zipped )
{
    std::cout << "DEBUG: Sending to everybody. " << (zipped ? "Zipped":"Raw") << "\n";


    std::ostringstream out;
    compress_config(cfg,out);

    std::set<ana_component*>::iterator it;

    for (it = components_.begin(); it != components_.end(); ++it)
    {
        if ( (*it)->is_server() )
        {
            const size_t necessary_calls = server_manager_[ (*it)->server() ]->client_amount();
            ana_send_handler handler( out.str().size(), necessary_calls );

            (*it)->server()->send_all( ana::buffer( out.str() ), &handler, ana::ZERO_COPY);
            handler.wait_completion(); // the handler will release the mutex after necessary_calls calls
        }
        else
        {
            ana_send_handler handler( out.str().size() );

            (*it)->client()->send( ana::buffer( out.str() ), &handler, ana::ZERO_COPY );
            handler.wait_completion();
        }
    }
    std::cout << "Sent data.\n";
    return out.str().size();
}

size_t ana_network_manager::send( network::connection connection_num , const config& cfg, bool /*zipped*/ )
{
    std::cout << "DEBUG: Single send...\n";

    std::ostringstream out;
    compress_config(cfg, out );

    return send_raw_data( out.str().c_str(), out.str().size(), connection_num );
}

size_t ana_network_manager::send_raw_data( const char* base_char, size_t size, network::connection connection_num )
{
    ana::net_id id( connection_num );
    ana_component_set::iterator it;

    it = std::find_if( components_.begin(), components_.end(),
                    boost::bind(&ana_component::get_id, _1) == id );

    if ( it != components_.end())
    {
        if ( (*it)->is_server() )
            throw std::runtime_error("Can't send to the server itself.");

        ana_send_handler handler( size );
        (*it)->client()->send( ana::buffer( base_char, size ), &handler); //, ana::ZERO_COPY);
        handler.wait_completion();

        if ( handler.error() )
            return 0;
        else
            return size;
    }
    else
    {
        for (it = components_.begin(); it != components_.end(); ++it)
        {
            if ((*it)->is_server())
            {
                // TODO: The next lines shouldn't be commented, no way the handler isn't called
                ana_send_handler handler( size );
//                 (*it)->server()->send_one( id, ana::buffer( base_char, size ), this, ana::ZERO_COPY);
                (*it)->server()->send_one( id, ana::buffer( base_char, size ), &handler); //, ana::ZERO_COPY);
                handler.wait_completion();
                if ( handler.error() )
                    return 0;
                else
                    return size;
            }
        }
    }

    return 0;
}

void ana_network_manager::send_all_except(const config& cfg, network::connection connection_num)
{
    std::cout << "DEBUG: send_all_except " << connection_num << "\n";

    std::ostringstream out;
    compress_config(cfg, out );

    ana_component_set::iterator it;

    ana::net_id id_to_avoid( connection_num ); //I should have issued this id earlier

    for ( it = components_.begin(); it != components_.end(); ++it)
    {
         (*it)->get_id();

         if ((*it)->is_client())
             throw std::runtime_error("send_all_except shouldn't be used on clients.");

         ana_send_handler handler( out.str().size() );
         (*it)->server()->send_all_except( id_to_avoid, ana::buffer( out.str() ), &handler, ana::ZERO_COPY);

         handler.wait_completion();
    }
}

network::connection ana_network_manager::read_from_ready_buffer( const ana_component_set::iterator& it, config& cfg)
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
        ana_receive_handler handler;
        (*it)->listener()->set_listener_handler( &handler );

        if ( (*it)->is_server() )
            handler.wait_completion( (*it)->server(), timeout_ms );
        else
            handler.wait_completion( (*it)->client(), timeout_ms );

        (*it)->listener()->set_listener_handler( this );

        if ( handler.error() )
            return 0;
        else
        {
            read_config( handler.buffer(), cfg);
            return (*it)->get_wesnoth_id();
        }
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
                // For concurrency reasons, this checks if the old handler was used before the wait operation
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
                        boost::bind(&ana_component::get_id, _1) == id );

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
            ana::detail::read_buffer buffer = (*it)->wait_for_element();

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
                        if ( buffer->size() != sizeof(uint32_t) ) // all handshakes are 4 bytes long
                            mgrs->first->disconnect( client );

                        uint32_t handshake;
                        {
                            ana::serializer::bistream bis( buffer->string() );

                            bis >> handshake;
                            ana::network_to_host_long( handshake ); //not necessary since I'm expecting a 0 anyway
                        }

                        if ( handshake != 0 )
                            mgrs->first->disconnect( client );

                        //send back it's id
                        ana::serializer::bostream bos;

                        uint32_t network_byte_order_id = client;

                        ana::host_to_network_long( network_byte_order_id );
                        bos << network_byte_order_id;

                        //TODO: I should check if this operation was successful
                        mgrs->first->send_one(client, ana::buffer( bos.str() ), this );
                        mgrs->first->set_header_first_mode( client );
                        mgrs->second->has_connected( client );
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
    std::cout << "DEBUG: Disconnected from server.\n";

    std::set< ana_component* >::iterator it;

    it = std::find_if( components_.begin(), components_.end(),
                        boost::bind(&ana_component::get_id, _1) == client );

    if ( it != components_.end() )
    {
        std::cout << "DEBUG: Removing bad component.\n";
        delete *it;
        components_.erase(it);
    }
}
