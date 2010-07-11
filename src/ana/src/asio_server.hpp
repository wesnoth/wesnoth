/* $Id$ */

/**
 * @file asio_server.hpp
 * @brief Header file of the server side for the ana project.
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

#ifndef ASIO_SERVER_HPP
#define ASIO_SERVER_HPP

#include <boost/asio.hpp>
#include <boost/interprocess/smart_ptr/unique_ptr.hpp>
#include <boost/function.hpp>

#include "ana.hpp"
#include "asio_listener.hpp"
#include "asio_sender.hpp"

using boost::asio::ip::tcp;

struct asio_proxy_manager
{
	virtual ~asio_proxy_manager() {}

    virtual void deregister_client(ana::server::client_proxy* client) = 0;
};

class asio_server : public  ana::server,
                    private asio_proxy_manager
{
    private:
        class asio_client_proxy : public virtual ana::server::client_proxy,
                                  public asio_listener,
                                  private asio_sender
        {
            public:
                asio_client_proxy(boost::asio::io_service& io_service, asio_proxy_manager* mgr);

                virtual tcp::socket& socket();

                virtual ~asio_client_proxy();
            private:
                virtual void disconnect() { disconnect_listener(); }

                virtual void disconnect_listener();

                virtual void send(ana::detail::shared_buffer, ana::send_handler*, ana::detail::sender* );

                virtual std::string ip_address( ) const;

                virtual void start_logging();
                virtual void stop_logging();

                virtual ana::stats_collector* stats_collector() { return stats_collector_; }

                virtual ana::timer* create_timer() { return new ana::timer( socket_.get_io_service() ); }

                virtual const ana::stats* get_stats( ana::stat_type type ) const;

                void log_conditional_receive( const ana::detail::read_buffer& buffer );

                tcp::socket           socket_;
                asio_proxy_manager*   manager_;
                ana::stats_collector* stats_collector_;
        };

    public:
        asio_server();

        virtual ~asio_server();

    private:
        virtual void set_connection_handler( ana::connection_handler* );

        virtual void send_all(boost::asio::const_buffer, ana::send_handler*, ana::send_type );
        virtual void send_if (boost::asio::const_buffer, ana::send_handler*, const ana::client_predicate&, ana::send_type );
        virtual void send_one(ana::net_id, boost::asio::const_buffer, ana::send_handler*, ana::send_type );
        virtual void send_all_except(ana::net_id, boost::asio::const_buffer, ana::send_handler*, ana::send_type );

        virtual void set_listener_handler( ana::listener_handler* );
        virtual void run_listener();
        virtual void run(ana::port pt);

        virtual void deregister_client(client_proxy* client);

        virtual std::string ip_address( ana::net_id ) const;

        virtual const ana::stats* get_client_stats( ana::net_id, ana::stat_type ) const;

        virtual void log_receive( ana::detail::read_buffer buffer );

        virtual void start_logging();
        virtual void stop_logging();

        virtual const ana::stats* get_stats( ana::stat_type type ) const;

        virtual void disconnect()                                          {}
        virtual void wait_raw_object(ana::serializer::bistream& , size_t ) {}
        virtual void set_raw_buffer_max_size( size_t )                     {}

        virtual ana::stats_collector* stats_collector() { return stats_collector_; }

        virtual ana::timer* create_timer() { return new ana::timer( io_service_); }

        void handle_accept (const boost::system::error_code& ec,asio_client_proxy* client, ana::connection_handler* );

        void register_client(client_proxy* client);

        void run_acceptor_thread(asio_server* obj, ana::connection_handler* );

        void async_accept( ana::connection_handler* );

        boost::asio::io_service      io_service_;
        boost::thread                io_thread_;
        std::auto_ptr<tcp::acceptor> acceptor_;
        std::list<client_proxy*>     client_proxies_;
        bool                         listening_;
        ana::listener_handler*       listener_;
        ana::connection_handler*     connection_handler_;
        asio_client_proxy*           last_client_proxy_;

        ana::stats_collector*        stats_collector_;
};

#endif
