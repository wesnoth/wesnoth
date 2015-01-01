
/**
 * @file
 * @brief Implementation of the client side of the ana project.
 *
 * ana: Asynchronous Network API.
 * Copyright (C) 2010 - 2015 Guillermo Biset.
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

#include <iostream>

#include <memory>

#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include "asio_client.hpp"

using boost::asio::ip::tcp;

asio_client::asio_client(ana::address address, ana::port pt) :
    asio_listener(),
    io_service_(),
    io_threads_(),
    work_( io_service_ ),
    socket_(io_service_),
    address_(address),
    port_(pt),
    connect_timeout_ms_( 0 ),
    proxy_( NULL ),
    use_proxy_( false ),
    stats_collector_( ),
    last_valid_operation_id_( ana::no_operation ),
    connection_informed_mutex_(),
    connection_informed_( false )
{
}

asio_client::~asio_client()
{
    disconnect_listener();

    std::list< boost::thread* >::iterator it;

    it = io_threads_.begin();

    while (it != io_threads_.end())
    {
        (*it)->join();
        it = io_threads_.erase( it );
    }
}

ana::client* ana::client::create(ana::address address, ana::port pt)
{
    return new asio_client(address, pt);
}

void asio_client::run()
{
    io_threads_.push_back(
                new boost::thread( boost::bind( &boost::asio::io_service::run, &io_service_) ) );
}

void asio_client::handle_proxy_connection(const boost::system::error_code& ec,
                                          ana::connection_handler*         handler,
                                          ana::timer*                      timer)
{
    delete timer;

    inform_connection_result( handler, ec);

    if ( ( ! ec ) && ( ana::client::header_mode() ) )
        run_listener();

    delete proxy_;
}

tcp::socket& asio_client::socket()
{
    return socket_;
}

void asio_client::handle_connect(const boost::system::error_code& ec,
                                 tcp::resolver::iterator          endpoint_iterator,
                                 ana::connection_handler*         handler,
                                 ana::timer*                      timer)
{
    if ( ! ec )
    {
        delete timer; // will call handle_timeout with ana::operation_aborted

        inform_connection_result( handler, ec);

        if ( ana::client::header_mode() )
            run_listener();
    }
    else
    {
        if ( endpoint_iterator == tcp::resolver::iterator() ) // finished iterating, not connected
            inform_connection_result( handler, ec);
        else
        {
            //retry
            socket_.close();

            tcp::endpoint endpoint = *endpoint_iterator;
            socket_.async_connect(endpoint,
                                  boost::bind(&asio_client::handle_connect, this,
                                              boost::asio::placeholders::error, ++endpoint_iterator,
                                              handler, timer));
        }
    }
}

void asio_client::inform_connection_result( ana::connection_handler* handler, ana::error_code ec)
{
    boost::mutex::scoped_lock lock( connection_informed_mutex_ );

    if( ! connection_informed_ )
    {
        connection_informed_ = true;
        handler->handle_connect( ec, 0 );
    }
}

void asio_client::handle_timeout(const boost::system::error_code& ec,
                                 ana::connection_handler*         handler,
                                 ana::timer*                      timer)
{
    if ( ec != ana::operation_aborted ) // Timed out before canceling
    {
        delete timer;

        inform_connection_result( handler, ec);
        cancel_pending();
    }
}

ana::timer* asio_client::start_connection_timer(ana::connection_handler* handler)
{
    if ( connect_timeout_ms_ == 0 )
        return NULL;
    else
    {
        ana::timer* running_timer( NULL );

        ana::client* sender = static_cast<ana::client*>(this);

        running_timer = sender->create_timer();

        running_timer->wait( connect_timeout_ms_,
                            boost::bind(&asio_client::handle_timeout, this,
                                        boost::asio::placeholders::error, handler,
                                        running_timer ) );
        return running_timer;
    }
}

void asio_client::connect( ana::connection_handler* handler )
{
    ana::timer* running_timer = start_connection_timer(handler);

    try
    {
        tcp::resolver resolver(io_service_);
        tcp::resolver::query query(address_.c_str(), port_.c_str() );
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

        tcp::endpoint endpoint = *endpoint_iterator;
        socket_.async_connect(endpoint,
                              boost::bind(&asio_client::handle_connect, this,
                                          boost::asio::placeholders::error, ++endpoint_iterator,
                                          handler, running_timer));
    }
    catch (const std::exception&)
    {
        inform_connection_result( handler, ana::generic_error );
    }
}

void asio_client::connect_through_proxy(std::string              proxy_address,
                                        std::string              proxy_port,
                                        ana::connection_handler* handler,
                                        std::string              user_name,
                                        std::string              password)
{
    ana::timer* running_timer = start_connection_timer(handler);

    use_proxy_ = true;

    proxy_information proxy_info;

    proxy_info.proxy_address = proxy_address;
    proxy_info.proxy_port    = proxy_port;
    proxy_info.user_name     = user_name;
    proxy_info.password      = password;

    proxy_ = new proxy_connection( socket_, proxy_info, address_, port_, running_timer);

    proxy_->connect( this, handler );
}

ana::operation_id asio_client::send(boost::asio::const_buffer buffer,
                               ana::send_handler*        handler,
                               ana::send_type            copy_buffer )
{
    ana::detail::shared_buffer s_buf(new ana::detail::copying_buffer(buffer, copy_buffer ) );

    asio_sender::send(s_buf,
                      socket_,
                      handler,
                      static_cast<ana::client*>(this),
                      ++last_valid_operation_id_     );

    return last_valid_operation_id_;
}

void asio_client::log_receive( ana::read_buffer buffer )
{
    stats_collector_.log_receive( buffer );
}

const ana::stats* asio_client::get_stats( ana::stat_type type ) const
{
    return stats_collector_.get_stats( type );
}

ana::stats_collector& asio_client::stats_collector()
{
    return stats_collector_;
}

void asio_client::cancel_pending()
{
    socket_.cancel();
}

void asio_client::set_connect_timeout( size_t ms )
{
    connect_timeout_ms_ = ms;
}

void asio_client::expecting_message( size_t ms_until_timeout )
{
    wait_for_incoming_message( ms_until_timeout );
}

std::string asio_client::ip_address() const
{
    return socket_.remote_endpoint().address().to_string();
}

void asio_client::disconnect_listener()
{
    io_service_.stop();
}

