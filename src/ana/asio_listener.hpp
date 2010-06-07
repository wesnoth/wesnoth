/* $Id$ */

/**
 * @file asio_listener.hpp
 * @brief Header file of a listener for the ana project.
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
 * the Free Software Foundation, either version 3 of the License, or
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

#include "ana.hpp"

using boost::asio::ip::tcp;

class asio_listener : public virtual ana::detail::listener
{
    public:
        asio_listener( boost::asio::io_service&, tcp::socket&);

        virtual void set_listener_handler( ana::listener_handler* listener);
        virtual void run_listener();

        virtual ~asio_listener();
    private:
        virtual void disconnect_listener() {}

        void listen_one_message();

        void disconnect( ana::listener_handler* listener, boost::system::error_code error);

        void handle_header(char* header, const boost::system::error_code& , ana::listener_handler* );

        void handle_body( ana::detail::read_buffer , const boost::system::error_code& , ana::listener_handler* );

        /*attr*/
        boost::asio::io_service&   io_service_;
        tcp::socket&               socket_;
        ana::listener_handler*     listener_;
        char                       header_[ana::HeaderLength];
};

#endif