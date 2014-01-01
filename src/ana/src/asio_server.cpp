
/**
 * @file
 * @brief Implementation of the server side for the ana project.
 *
 * ana: Asynchronous Network API.
 * Copyright (C) 2010 - 2014 Guillermo Biset.
 *
 * This file is part of the ana project.
 *
 * System:         ana
 * Language:       C++
 *
 * Author:         Guillermo Biset
 * E-Mail:         billybiset AT gmail DOT com
 *
 * ana is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * ana is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ana.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <cstdlib>
#include <iostream>
#include <algorithm>

#include "asio_server.hpp"

using namespace ana;

using boost::asio::ip::tcp;

asio_server::asio_server() :
    io_service_(),
    work_( io_service_ ),
    io_threads_(),
    acceptor_( NULL ),
    client_proxies_(),
    listening_(false),
    listener_( NULL ),
    connection_handler_( NULL ),
    last_client_proxy_( NULL ),
    stats_collector_( ),
    last_valid_operation_id_( ana::no_operation )
{
}

asio_server::~asio_server()
{
    io_service_.stop();

    std::list< boost::thread* >::iterator thread_it;

    thread_it = io_threads_.begin();

    while (thread_it != io_threads_.end())
    {
        (*thread_it)->join();
        thread_it = io_threads_.erase( thread_it );
    }

    /* Since the asio_client_proxy destructor removes the client from client_proxies_
       I'll just delete every proxy from a different list. */
    std::list<client_proxy*> copy( client_proxies_ );

    for (std::list<client_proxy*>::iterator proxy_it = copy.begin();
         proxy_it != copy.end();
         ++proxy_it)
    {
        delete *proxy_it;
    }

    assert( client_proxies_.empty() );

    delete last_client_proxy_;
}

server* ana::server::create()
{
    return new asio_server();
}

void asio_server::set_connection_handler( connection_handler* handler )
{
    connection_handler_ = handler;
}

void asio_server::run(port pt)
{
    tcp::acceptor* new_acceptor(new tcp::acceptor( io_service_,
                                                   tcp::endpoint(tcp::v4(), atoi( pt.c_str() ))));

    acceptor_.reset(new_acceptor);

    async_accept( connection_handler_ );

    run_listener( );

    io_threads_.push_back(
            new boost::thread( boost::bind( &boost::asio::io_service::run, &io_service_) ) );
}

void asio_server::async_accept( connection_handler* handler )
{
    try
    {
        last_client_proxy_ = new asio_client_proxy(io_service_, this);

        acceptor_->async_accept(last_client_proxy_->socket(),
                                boost::bind(&asio_server::handle_accept,
                                            this,
                                            boost::asio::placeholders::error,
                                            last_client_proxy_,
                                            handler));
    }
    catch( const std::exception&)
    {
        delete last_client_proxy_;
    }
}

void asio_server::register_client(client_proxy* client)
{
    client_proxies_.push_back(client);

    if (listening_)
    {
        client->set_listener_handler( listener_ );
        client->run_listener( );
    }
}

void asio_server::deregister_client(client_proxy* client)
{
    client_proxies_.remove( client );
}

void asio_server::handle_accept(const boost::system::error_code& ec,
                               asio_client_proxy*                client,
                               connection_handler*               handler )
{
    if (! ec)
    {
        if ( raw_mode() ) // only test for the non default setting
            client->set_raw_data_mode();

        register_client(client);
        handler->handle_connect( ec, client->id() );
    }
    else
    {
        std::cerr << "Server: Error accepting client connection." << std::endl;
        delete client;
    }

    async_accept( handler );
}


void asio_server::run_listener( )
{
    for (std::list<client_proxy*>::iterator it( client_proxies_.begin() );
         it != client_proxies_.end();
         ++it)
    {
        (*it)->set_listener_handler( listener_ );
        (*it)->run_listener( );
    }
}

void asio_server::set_listener_handler( listener_handler* listener )
{
    listening_ = true;
    listener_  = listener;

    for (std::list<client_proxy*>::iterator it( client_proxies_.begin() );
         it != client_proxies_.end();
         ++it)
    {
        (*it)->set_listener_handler( listener_ );
    }
}

ana::operation_id asio_server::send_one(net_id                    id,
                                        boost::asio::const_buffer buffer,
                                        send_handler*             handler,
                                        send_type                 copy_buffer)
{
    return send_if(buffer,
                   handler,
                   create_predicate( boost::bind( std::equal_to<net_id>(), id, _1) ),
                   copy_buffer );
}

ana::operation_id asio_server::send_all_except(net_id                    id,
                                               boost::asio::const_buffer buffer,
                                               send_handler*             handler,
                                               send_type                 copy_buffer)
{
    return send_if(buffer,
                   handler,
                   create_predicate ( boost::bind( std::not_equal_to<net_id>(), id, _1) ),
                   copy_buffer );
}


ana::operation_id asio_server::send_if(boost::asio::const_buffer buffer,
                                       send_handler*             handler,
                                       const client_predicate&   predicate,
                                       send_type                 copy_buffer)
{
    // This allows me to copy the buffer only once for many send operations
    ana::detail::shared_buffer s_buf( new ana::detail::copying_buffer( buffer, copy_buffer ) );
    // it's a boost::shared_ptr

    bool at_least_one_holds_the_condition( false );

    ana::operation_id this_operation_id = ana::no_operation;

    for (std::list<client_proxy*>::iterator it(client_proxies_.begin());
         it != client_proxies_.end();
         ++it)
    {
        if ( predicate.selects( (*it)->id() ) )
        {
            if ( ! at_least_one_holds_the_condition )
                this_operation_id = ++last_valid_operation_id_;

            (*it)->send(s_buf, handler, this, this_operation_id);
            at_least_one_holds_the_condition = true;
        }
    }

    return this_operation_id;
}

ana::operation_id asio_server::send_all(boost::asio::const_buffer buffer,
                                        send_handler*             handler,
                                        send_type                 copy_buffer )
{
    // This allows me to copy the buffer only once for many send operations
    ana::detail::shared_buffer s_buf(new ana::detail::copying_buffer( buffer, copy_buffer ) );
    // it's a boost::shared_ptr


    if ( client_proxies_.empty() )
        return ana::no_operation;

    ++last_valid_operation_id_;

    std::for_each(client_proxies_.begin(),
                  client_proxies_.end(),
                  boost::bind(&client_proxy::send, _1,
                              s_buf, handler, this, last_valid_operation_id_));

    return last_valid_operation_id_;
}

std::string asio_server::ip_address( net_id id ) const
{
    std::list<ana::server::client_proxy*>::const_iterator it;

    it = std::find_if( client_proxies_.begin(), client_proxies_.end(),
                       boost::bind( &client_proxy::id, _1) == id );

    if ( it != client_proxies_.end() )
        return (*it)->ip_address();
    else
        return "";
}

const ana::stats* asio_server::get_client_stats( ana::net_id id, ana::stat_type type  ) const
{
    std::list<ana::server::client_proxy*>::const_iterator it;

    it = std::find_if( client_proxies_.begin(), client_proxies_.end(),
                       boost::bind( &client_proxy::id, _1) == id );

    if ( it != client_proxies_.end() )
        return (*it)->get_stats(type);
    else
        return NULL; //No such client
}

void asio_server::log_receive( ana::read_buffer buffer )
{
    stats_collector_.log_receive( buffer );
}

ana::timer* asio_server::create_timer()
{
    return new ana::timer( io_service_ );
}

const ana::stats* asio_server::get_stats( ana::stat_type type ) const
{
    return stats_collector_.get_stats( type );
}


asio_server::asio_client_proxy::asio_client_proxy(boost::asio::io_service& io_service,
                                                  asio_proxy_manager* server) :
    client_proxy(),
    asio_listener(),
    socket_(io_service),
    manager_(server),
    stats_collector_( )
{
}

asio_server::asio_client_proxy::~asio_client_proxy()
{
    manager_->deregister_client( this );
    socket_.close();
}

tcp::socket& asio_server::asio_client_proxy::socket()
{
    return socket_;
}

void asio_server::disconnect( ana::net_id id )
{
    std::list<ana::server::client_proxy*>::iterator it;

    it = std::find_if( client_proxies_.begin(), client_proxies_.end(),
                       boost::bind( &client_proxy::id, _1) == id );

    if ( it != client_proxies_.end() )
        delete *it;                     // it will erase it from client_proxies_
}

void asio_server::disconnect()
{
    io_service_.stop();

    std::list< boost::thread* >::iterator it;

    it = io_threads_.begin();

    while (it != io_threads_.end())
    {
        (*it)->join();
        it = io_threads_.erase( it );
    }

    for (std::list<client_proxy*>::iterator it = client_proxies_.begin();
         it != client_proxies_.end();
         ++it)
    {
        delete *it;
    }

    client_proxies_.clear();

    io_service_.reset();
}

void asio_server::set_raw_buffer_max_size( size_t size)
{
    for (std::list<client_proxy*>::iterator it = client_proxies_.begin();
         it != client_proxies_.end();
         ++it)
    {
        (*it)->set_raw_buffer_max_size( size );
    }
}

ana::stats_collector& asio_server::stats_collector()
{
    return stats_collector_;
}

void asio_server::expecting_message( net_id id, size_t ms_until_timeout )
{
    std::list<ana::server::client_proxy*>::iterator it;

    it = std::find_if( client_proxies_.begin(), client_proxies_.end(),
                       boost::bind( &client_proxy::id, _1) == id );

    if ( it != client_proxies_.end() )
        (*it)->expecting_message( ms_until_timeout );
}

void asio_server::set_header_first_mode( ana::net_id id )
{
    std::list<ana::server::client_proxy*>::const_iterator it;

    it = std::find_if( client_proxies_.begin(), client_proxies_.end(),
                       boost::bind( &client_proxy::id, _1) == id );

    if ( it != client_proxies_.end() )
        (*it)->set_header_first_mode();
}

void asio_server::set_raw_data_mode( ana::net_id id )
{
    std::list<ana::server::client_proxy*>::const_iterator it;

    it = std::find_if( client_proxies_.begin(), client_proxies_.end(),
                       boost::bind( &client_proxy::id, _1) == id );

    if ( it != client_proxies_.end() )
        (*it)->set_raw_data_mode();
}

const ana::stats* asio_server::asio_client_proxy::get_stats( ana::stat_type type ) const
{
    return stats_collector_.get_stats( type );
}

void asio_server::cancel_pending( )
{
    std::for_each( client_proxies_.begin(), client_proxies_.end(),
                   boost::bind(&ana::server::client_proxy::cancel_pending, _1 ) );
}

void asio_server::cancel_pending( ana::net_id client_id )
{
    std::list<ana::server::client_proxy*>::const_iterator it;

    it = std::find_if( client_proxies_.begin(), client_proxies_.end(),
                       boost::bind( &client_proxy::id, _1) == client_id );

    if ( it != client_proxies_.end() )
        (*it)->cancel_pending();
}


void asio_server::asio_client_proxy::send(ana::detail::shared_buffer buffer,
                                          send_handler*              handler,
                                          ana::detail::sender*       sender,
                                          ana::operation_id          op_id)
{
    asio_sender::send( buffer, socket_, handler, sender, op_id );
}

void asio_server::asio_client_proxy::disconnect_listener()
{
    cancel_pending();
    delete this;
}

void asio_server::asio_client_proxy::cancel_pending()
{
    socket_.cancel();
}


std::string asio_server::asio_client_proxy::ip_address() const
{
    return socket_.remote_endpoint().address().to_string();
}

ana::stats_collector& asio_server::asio_client_proxy::stats_collector()
{
    return stats_collector_;
}

ana::timer* asio_server::asio_client_proxy::create_timer()
{
    return new ana::timer( socket_.get_io_service() );
}

void asio_server::asio_client_proxy::expecting_message( size_t ms_until_timeout )
{
    wait_for_incoming_message( ms_until_timeout, id() );
}
