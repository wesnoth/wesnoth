/* $Id$ */

/**
 * @file asio_sender.hpp
 * @brief Implementation file providing send capability to ana.
 *
 * ana: Asynchronous Network API.
 * Copyright (C) 2010 Guillermo Biset.
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

#include <boost/bind.hpp>

#include "asio_sender.hpp"

void asio_sender::send(ana::detail::shared_buffer buffer,
                       tcp::socket&               socket,
                       ana::send_handler*         handler,
                       ana::detail::sender*       sender)
{
    ana::timer* running_timer( NULL );
    try
    {
        running_timer = sender->start_timer( buffer,
                                             boost::bind(&asio_sender::handle_timeout, this,
                                                         boost::asio::placeholders::error, handler ) );

        if ( sender->raw_mode() )
        {
            boost::asio::async_write(socket, boost::asio::buffer(buffer->base(), buffer->size() ),
                                     boost::bind(&asio_sender::handle_send,this,
                                                 boost::asio::placeholders::error,
                                                 buffer, handler, running_timer));
        }
        else
        {
            ana::ana_uint32 size( buffer->size() );
            ana::host_to_network_long( size );

            ana::serializer::bostream* output_stream = new ana::serializer::bostream();
            (*output_stream) << size;

            //write the header first in a separate operation, then send the full buffer
            boost::asio::async_write(socket, boost::asio::buffer( output_stream->str() ),
                                        boost::bind(&asio_sender::handle_sent_header,this,
                                                    boost::asio::placeholders::error, output_stream,
                                                    &socket, buffer, handler, running_timer));
        }
    }
    catch(std::exception& e)
    {
        disconnect();
        delete running_timer;
    }
}


void asio_sender::handle_sent_header(const boost::system::error_code& ec,
                                     ana::serializer::bostream*       bos,
                                     tcp::socket*                     socket,
                                     ana::detail::shared_buffer       buffer,
                                     ana::send_handler*               handler,
                                     ana::timer*                      running_timer)
{
    delete bos;

    if ( ! ec )
    {
        boost::asio::async_write(*socket, boost::asio::buffer(buffer->base(), buffer->size() ),
                                 boost::bind(&asio_sender::handle_send,this,
                                             boost::asio::placeholders::error,
                                             buffer, handler, running_timer));

    }
    else
    {
        disconnect();
        delete running_timer;
    }
}

void asio_sender::handle_send(const boost::system::error_code& ec,
                              ana::detail::shared_buffer       buffer,
                              ana::send_handler*               handler,
                              ana::timer*                      running_timer)
{
    delete running_timer;

    log_conditional_send( buffer );

    handler->handle_send( ec, id() );

    if ( ec )
        disconnect();
}



void asio_sender::handle_timeout(const boost::system::error_code& ec, ana::send_handler* handler)
{
    if ( ec != boost::asio::error::operation_aborted) // The timer wasn't cancelled. So: inform this and disconnect
        handler->handle_send( ana::timeout_error , id() );
}

void asio_sender::log_conditional_send( const ana::detail::shared_buffer& buf )
{
    if (stats_collector() != NULL )
        stats_collector()->log_send( buf );
}