
/**
 * @file
 * @brief Header file of a listener for the ana project.
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

#ifndef ASIO_LISTENER_HPP
#define ASIO_LISTENER_HPP

#include <boost/asio.hpp>
#include <memory>

#include "../api/ana.hpp"

using boost::asio::ip::tcp;

class asio_listener : public virtual ana::detail::listener
{
    public:
        asio_listener( );

        virtual void set_listener_handler( ana::listener_handler* listener);
        virtual void run_listener();

        virtual ~asio_listener();

    protected:
        virtual tcp::socket& socket() = 0;

        void wait_for_incoming_message( size_t ms_to_timeout, ana::net_id id = 0  );

    private:
        virtual void disconnect_listener()                   {}

        virtual void set_raw_buffer_max_size( size_t size );

        virtual void wait_raw_object(ana::serializer::bistream& bis, size_t size);

        void listen_one_message();

        void disconnect( boost::system::error_code error);

        void handle_header(char* header, const boost::system::error_code& );

        void handle_body( ana::read_buffer , const boost::system::error_code& );

        void handle_partial_body( ana::read_buffer,
                                  const boost::system::error_code&,
                                  size_t accumulated,
                                  size_t last_msg_size);

        void handle_timeout( const boost::system::error_code&, ana::net_id);

        void handle_raw_buffer( ana::read_buffer, const boost::system::error_code&, size_t);

        /*attr*/
        bool                       disconnected_;
        ana::listener_handler*     listener_;
        char                       header_[ana::HEADER_LENGTH];
        size_t                     raw_mode_buffer_size_;

        ana::timer*                next_message_timer_;
};

#endif

