/* $Id$ */

/**
 * @file network_manager_ana.hpp
 * @brief Header file for network features using ana.
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

#include <set>
#include <queue>

#include <boost/thread.hpp>
#include <boost/variant.hpp>

#include "network.hpp"
#include "ana/api/ana.hpp"

struct send_stats_logger
{
    virtual void update_send_stats( size_t ) = 0;
};

class ana_handler : public ana::send_handler
{
    public:
        ana_handler( boost::mutex& mutex, send_stats_logger* logger, size_t buf_size, size_t calls = 1 );

        ~ana_handler();

    private:
        virtual void handle_send(ana::error_code error_code, ana::net_id /*client*/);

        boost::mutex&      mutex_;
        size_t             target_calls_;
        ana::error_code    error_code_;
        send_stats_logger* logger_;
        size_t             buf_size_;
};

class ana_connect_handler : public ana::connection_handler
{
    public:
        ana_connect_handler( boost::mutex& mutex, ana::timer* timer);

        void handle_timeout(ana::error_code error_code);

        ~ana_connect_handler();

        const ana::error_code& error() const;

    private:
        virtual void handle_connect(ana::error_code error_code, ana::net_id /*client*/);

        boost::mutex&      mutex_;
        ana::timer*        timer_;
        ana::error_code    error_code_;
        bool               connected_;
};


class ana_component : public send_stats_logger
{
    public:
        ana_component( );

        ana_component( const std::string& host, const std::string& port);

        network::statistics get_send_stats() const;

        network::statistics get_receive_stats() const;

        ana::server* server() const;

        ana::client* client() const;

        bool is_server() const;

        bool is_client() const;

        ana::net_id get_id() const;

        const ana::stats* get_stats() const;

        void add_buffer(ana::detail::read_buffer buffer);

        ana::detail::read_buffer wait_for_element();

        void update_receive_stats( size_t buffer_size );

    private:
        virtual void update_send_stats( size_t buffer_size);

        boost::variant<ana::server*, ana::client*> base_;

        bool        is_server_;
        ana::net_id id_;

        network::statistics send_stats_;
        network::statistics receive_stats_;

        //Buffer queue attributes
        boost::mutex                   mutex_;
        boost::condition_variable      condition_;

        std::queue< ana::detail::read_buffer > buffers_;
};

class clients_manager : public ana::connection_handler
{
    public:
        clients_manager();

        size_t client_amount() const;

    private:
        virtual void handle_connect(ana::error_code error, ana::net_id client);

        virtual void handle_disconnect(ana::error_code /*error*/, ana::net_id client);

        std::set<ana::net_id> ids_;
};

class ana_network_manager : public ana::listener_handler,
                            public ana::send_handler
{
    public:
        ana_network_manager();

        ana::net_id create_server( );

        network::connection create_client_and_connect(std::string host, int port);

        const ana::stats* get_stats( network::connection connection_num );

        void run_server(ana::net_id id, int port);

        std::string ip_address( network::connection id );

        size_t number_of_connections() const;

        size_t send_all( const config& cfg, bool zipped );

        size_t send( network::connection connection_num , const config& cfg, bool zipped );

        ana::detail::read_buffer read_from( network::connection connection_num );

        network::statistics get_send_stats(network::connection handle);

        network::statistics get_receive_stats(network::connection handle);

    private:
        virtual void handle_send(ana::error_code error_code, ana::net_id client);

        virtual void handle_message( ana::error_code          error,
                                     ana::net_id              client,
                                     ana::detail::read_buffer buffer);

        virtual void handle_disconnect(ana::error_code /*error_code*/, ana::net_id client);

        ana::timer*                connect_timer_;
        std::set< ana_component* > components_;

        std::map< ana::server*, const clients_manager* > server_manager_;
};