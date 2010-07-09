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

#ifndef NETWORK_MANAGER_ANA_HPP_INCLUDED
#define NETWORK_MANAGER_ANA_HPP_INCLUDED

/** Interface for objects that log send statistics. */
struct send_stats_logger
{
    virtual void update_send_stats( size_t ) = 0;
};

/**
 * A representative of a network component to the application.
 */
class ana_component : public send_stats_logger
{
    public:
        /** Constructs a server component. */
        ana_component( );

        /**
         * Constructs a client component.
         *
         * @param host : The hostname to which it is supposed to connect to.
         * @param port : The port it is supposed to connect to.
         */
        ana_component( const std::string& host, const std::string& port);

        /** Get network upload statistics for this component. */
        network::statistics get_send_stats() const;

        /** Get network download statistics for this component. */
        network::statistics get_receive_stats() const;

        /**
         * Get the pointer to an ana::server object for this component.
         *
         * @Pre : This component is a server.
         */
        ana::server* server() const;

        /**
         * Get the pointer to an ana::client object for this component.
         *
         * @Pre : This component is a client.
         */
        ana::client* client() const;

        /** Returns true iff this component is a server. */
        bool is_server() const;

        /** Returns true iff this component is a client. */
        bool is_client() const;

        /** Returns this component's id. */
        ana::net_id get_id() const;

        network::connection get_wesnoth_id() const;

        void set_wesnoth_id( network::connection ) ;

        /** Returns a pointer to the ana::stats object for accumulated network stats. */
        const ana::stats* get_stats() const;

        /** Push a buffer to the queue of incoming messages. */
        void add_buffer(ana::detail::read_buffer buffer);

        /**
         * Blocking operation to wait for a message in a component.
         *
         * @returns The buffer that was received first from all pending buffers.
         */
        ana::detail::read_buffer wait_for_element();

        bool new_buffer_ready(); // non const due to mutex blockage

        /** Log an incoming buffer. */
        void update_receive_stats( size_t buffer_size );

    private:
        virtual void update_send_stats( size_t buffer_size);

        boost::variant<ana::server*, ana::client*> base_;

        bool        is_server_;

        ana::net_id         id_;
        network::connection wesnoth_id_;

        network::statistics send_stats_;
        network::statistics receive_stats_;

        //Buffer queue attributes
        boost::mutex                   mutex_;
        boost::condition_variable      condition_;

        std::queue< ana::detail::read_buffer > buffers_;
};

typedef std::set<ana_component*> ana_component_set;


/**
 * To use the asynchronous library synchronously, objects of this
 * type lock a mutex until enough calls have been made to the
 * associated handler.
 */
class ana_send_handler : public ana::send_handler
{
    public:
        /**
         * Constructs a handler object.
         * @param logger : A pointer to an object logging send statistics.
         * @param buf_size : The size of the buffer being sent.
         * @param calls [optional, default 1] : The amount of calls to the handler expected.
         */
        ana_send_handler( send_stats_logger* logger, size_t buf_size, size_t calls = 1 );

        /** Destructor, checks that the necessary calls were made. */
        ~ana_send_handler();

        /** Locks current thread until all the calls are made. */
        void wait_completion();

        const ana::error_code& error() const
        {
            return error_code_;
        }

    private:
        virtual void handle_send(ana::error_code error_code, ana::net_id /*client*/);

        boost::mutex       mutex_;
        size_t             target_calls_;
        ana::error_code    error_code_;
        send_stats_logger* logger_;
        size_t             buf_size_;
};

/**
 * To use the asynchronous library synchronously, objects of this
 * type lock a mutex until enough calls have been made to the
 * associated handler.
 */
class ana_receive_handler : public ana::listener_handler
{
    public:
        /**
         * Constructs a reader handler object.
         */
        ana_receive_handler( );

        /** Destructor. */
        ~ana_receive_handler();

        /**
         * Attempts to read from those network components associated with this
         * handler object up until timeout_ms milliseconds.
         *
         * If the timeout parameter is 0, it will lock the current thread until
         * one of these components has received a message.
         *
         * @param component : A network component running an io_service which supports timeout capabilities.
         * @param timeout_ms : Amount of milliseconds to timeout the operation.
         */
        void wait_completion(ana::detail::timed_sender* component, size_t timeout_ms = 0);

        /** Returns the error_code from the operation. */
        const ana::error_code& error() const
        {
            return error_code_;
        }

        /** Returns the buffer from the operation. */
        ana::detail::read_buffer buffer() const
        {
            return buffer_;
        }

    private:
        virtual void handle_message   (ana::error_code, ana::net_id, ana::detail::read_buffer);
        virtual void handle_disconnect(ana::error_code, ana::net_id);

        void handle_timeout(ana::error_code error_code);

        boost::mutex             mutex_;
        boost::mutex             handler_mutex_;
        boost::mutex             timeout_called_mutex_;
        ana::error_code          error_code_;
        ana::detail::read_buffer buffer_;
        ana::timer*              receive_timer_;
        bool                     finished_;
};

/**
 * To use the asynchronous library synchronously, objects of this
 * type lock a mutex until enough calls have been made to the
 * associated handler.
 */
class ana_multiple_receive_handler : public ana::listener_handler
{
    public:
        /**
         * Constructs a reader handler object.
         */
        ana_multiple_receive_handler( ana_component_set& components );

        /** Destructor. */
        ~ana_multiple_receive_handler();

        /**
         * Attempts to read from those network components associated with this
         * handler object up until timeout_ms milliseconds.
         *
         * If the timeout parameter is 0, it will lock the current thread until
         * one of these components has received a message.
         *
         * @param component : A network component running an io_service which supports timeout capabilities.
         * @param timeout_ms : Amount of milliseconds to timeout the operation.
         */
        void wait_completion(size_t timeout_ms = 0);

        /** Returns the error_code from the operation. */
        const ana::error_code& error() const
        {
            return error_code_;
        }

        /** Returns the buffer from the operation. */
        ana::detail::read_buffer buffer() const
        {
            return buffer_;
        }

        network::connection get_wesnoth_id() const
        {
            return wesnoth_id_;
        }

    private:
        virtual void handle_message   (ana::error_code, ana::net_id, ana::detail::read_buffer);
        virtual void handle_disconnect(ana::error_code, ana::net_id);

        void handle_timeout(ana::error_code error_code);

        ana_component_set& components_;

        boost::mutex             mutex_;
        boost::mutex             handler_mutex_;
        boost::mutex             timeout_called_mutex_;
        ana::error_code          error_code_;
        ana::detail::read_buffer buffer_;
        network::connection      wesnoth_id_;
        ana::timer*              receive_timer_;
        bool                     finished_;
};



/**
 * To use the asynchronous library synchronously, objects of this
 * type lock a mutex until enough calls have been made to the
 * associated handler.
 */
class ana_connect_handler : public ana::connection_handler
{
    public:
        /**
         * Constructs a connection handler.
         *
         * @param timer : A pointer to a running timer dealing with the timeout of this connect operation.
         */
        ana_connect_handler( ana::timer* timer);

        /**
         * Handler of the timeout operation of the timer.
         */
        void handle_timeout(ana::error_code error_code);

        /** Destructor. */
        ~ana_connect_handler();

        /**
         * Checks if an error occured during the connection procedure.
         *
         * @returns Error code of the operation.
         */
        const ana::error_code& error() const;

        /** Locks current thread until the connection attempt has finished. */
        void wait_completion();

    private:
        virtual void handle_connect(ana::error_code error_code, ana::net_id /*client*/);

        boost::mutex       mutex_;
        ana::timer*        timer_;
        ana::error_code    error_code_;
        bool               connected_;
};

/**
 * Manages connected client ids for a given server.
 */
class clients_manager : public ana::connection_handler
{
    public:
        /** Constructor. */
        clients_manager();

        /** Returns the amount of components connected to this server. */
        size_t client_amount() const;

    private:
        virtual void handle_connect(ana::error_code error, ana::net_id client);

        virtual void handle_disconnect(ana::error_code /*error*/, ana::net_id client);

        std::set<ana::net_id> ids_;
};

/**
 * Provides network functionality for Wesnoth using the ana API and library.
 */
class ana_network_manager : public ana::listener_handler,
                            public ana::send_handler
{
    public:
        /** Constructor. */
        ana_network_manager();

        /**
         * Create a server component and return it's ID.
         *
         * @returns The ID of the new created server.
         */
        ana::net_id create_server( );

        /**
         * Create a client component and return it's network connection number.
         *
         * @returns The ID of the new created client, as a network::connection number.
         */
        network::connection create_client_and_connect(std::string host, int port);

        /**
         * Get the associated stats of a given component.
         *
         * @param connection_num : The ID of the network component.
         *
         * @returns A pointer to an ana::stats object of the given component.
         */
        const ana::stats* get_stats( network::connection connection_num );

        /**
         * Start a server on a given port.
         *
         * @param id : The ID of the server component.
         * @param port : The port on which to listen for new connections.
         */
        void run_server(ana::net_id id, int port);

        /** Get the IP address of a connected component by it's ID. */
        std::string ip_address( network::connection id );

        /** The amount of connected components to every server object created. */
        size_t number_of_connections() const;

        /** Send data to all created components. */
        size_t send_all( const config& cfg, bool zipped );

        /** Send data to the component with a given ID. */
        size_t send( network::connection connection_num , const config& cfg, bool zipped );

        void send_all_except(const config& cfg, network::connection connection_num);

        /** Read a message from a given component. */
        network::connection read_from( network::connection connection_num,
                                       ana::detail::read_buffer& buffer,
                                       size_t timeout_ms = 0 );

        /** Retrieve upload statistics on a given component. */
        network::statistics get_send_stats(network::connection handle);

        /** Retrieve download statistics on a given component. */
        network::statistics get_receive_stats(network::connection handle);

    private:
        virtual void handle_send(ana::error_code error_code, ana::net_id client);

        virtual void handle_message( ana::error_code          error,
                                     ana::net_id              client,
                                     ana::detail::read_buffer buffer);

        virtual void handle_disconnect(ana::error_code /*error_code*/, ana::net_id client);

        ana::timer*                connect_timer_;
        ana_component_set          components_;

        std::map< ana::server*, const clients_manager* > server_manager_;
};

#endif