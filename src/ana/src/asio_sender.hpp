
/**
 * @file
 * @brief Header file providing send capability to ana.
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

#ifndef ASIO_SENDER_HPP
#define ASIO_SENDER_HPP

#include <boost/asio.hpp>

#include "../api/ana.hpp"

using boost::asio::ip::tcp;

class asio_sender : private ana::detail::sender
{

    public:
        void send( ana::detail::shared_buffer,
                   tcp::socket&,
                   ana::send_handler*,
                   ana::detail::sender*,
                   ana::operation_id);

    private:
        void handle_sent_header(const boost::system::error_code& ec,
                                ana::serializer::bostream*,
                                tcp::socket*,
                                ana::detail::shared_buffer,
                                ana::send_handler*,
                                ana::timer*,
                                size_t,
                                ana::operation_id);

        void handle_partial_send( ana::detail::shared_buffer,
                                  const boost::system::error_code&,
                                  tcp::socket*,
                                  ana::send_handler*,
                                  ana::timer*,
                                  size_t,
                                  size_t,
                                  ana::operation_id);

        void handle_send(const boost::system::error_code&,
                         ana::send_handler*,
                         ana::timer*,
                         ana::operation_id,
                         bool from_timeout = false);
};

#endif

