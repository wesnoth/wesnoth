
/**
 * @file
 * @brief Header file for network features using ana.
 *
 * Copyright (C) 2010 - 2014 Guillermo Biset.
 *
 * Part of the Battle for Wesnoth Project http://www.wesnoth.org/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
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

/**
 * A representative of a network component to the application.
 */
class ana_component
{
    public:
        /** Constructs a server component. */
        ana_component( );

        ~ana_component( );

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
         * @pre : This component is a server.
         */
        ana::server* server() const;

        /**
         * Get the pointer to an ana::client object for this component.
         *
         * @pre : This component is a client.
         */
        ana::client* client() const;

        /**
         * Get the pointer to an ana::listener object for this component.
         * Both an ana::client an the ana::server are listeners.
         */
        ana::detail::listener* listener() const;

        /** Returns true iff this component is a server. */
        bool is_server() const;

        /** Returns true iff this component is a client. */
        bool is_client() const;

        /** Returns this component's id. */
        ana::net_id get_id() const;

        network::connection get_wesnoth_id() const;

        void set_wesnoth_id( network::connection ) ;

        /** Returns a pointer to the ana::stats object for accumulated network stats. */
        const ana::stats* get_stats( ana::stat_type type = ana::ACCUMULATED ) const;

        /** Push a buffer to the queue of incoming messages. */
        void add_buffer(ana::read_buffer buffer, ana::net_id id);

        /**
         * Blocking operation to wait for a message in a component.
         *
         * @returns The buffer that was received first from all pending buffers.
         */
        ana::read_buffer wait_for_element();

        /** Returns the network id of the oldest sender of a pending buffer. */
        network::connection oldest_sender_id_still_pending();

        /** Returns true iff. the component has a read buffer ready that hasn't been returned. */
        bool new_buffer_ready(); // non const due to mutex blockage

    private:
        boost::variant<ana::server*, ana::client*> base_;

        bool        is_server_;

        ana::net_id         id_;
        network::connection wesnoth_id_;

        //Buffer queue attributes
        boost::mutex                   mutex_;
        boost::condition_variable      condition_;

        std::queue< ana::read_buffer > buffers_;
        std::queue< network::connection >      sender_ids_;
};

typedef std::set<ana_component*> ana_component_set;

/**
 * Manages connected client ids for a given server.
 */
class clients_manager : public ana::connection_handler
{
    public:
        /** Constructor. */
        clients_manager( ana::server* );

        /** Returns the amount of components connected to this server. */
        size_t client_amount() const;

        void connected( ana::net_id id );

        void remove( ana::net_id id );

        void handshaked( ana::net_id id );

        bool has_connection_pending() const;

        bool is_pending_handshake( ana::net_id ) const;

        bool is_a_client( ana::net_id id ) const;

        network::connection get_pending_connection_id();

    private:
        virtual void handle_connect(ana::error_code error, ana::net_id client);

        virtual void handle_disconnect(ana::error_code /*error*/, ana::net_id client);

        ana::server*                    server_; // the server managing these clients

        std::set< ana::net_id >         ids_;
        std::set< network::connection > pending_ids_;
        std::set< ana::net_id >         pending_handshakes_;
};

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
         * @param calls [optional, default 1] : The amount of calls to the handler expected.
         */
        ana_send_handler( size_t calls = 1 );

        /** Destructor, checks that the necessary calls were made. */
        ~ana_send_handler();

        /** Locks current thread until all the calls are made. */
        void wait_completion();

        const ana::error_code& error() const
        {
            return error_code_;
        }

    private:
        virtual void handle_send(ana::error_code, ana::net_id, ana::operation_id);

        boost::mutex       mutex_;
        size_t             target_calls_;
        ana::error_code    error_code_;
};

class ana_handshake_finisher_handler : public ana::send_handler
{
    public:
        ana_handshake_finisher_handler( ana::server*, clients_manager* );

        ~ana_handshake_finisher_handler();
    private:

        virtual void handle_send(ana::error_code, ana::net_id, ana::operation_id);

        ana::server*       server_;
        clients_manager*   manager_;
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
        ana_receive_handler( ana_component_set::iterator );

        /** Destructor. */
        ~ana_receive_handler();

        /**
         * Attempts to read from those network components associated with this
         * handler object up until timeout_ms milliseconds.
         *
         * If the timeout parameter is 0, it will lock the current thread until
         * one of these components has received a message.
         *
         * @param component : A network component running an io_service which
         *                    supports timeout capabilities.
         * @param timeout_ms : Amount of milliseconds to timeout the operation.
         */
        void wait_completion(ana::detail::timed_sender* component, size_t timeout_ms = 0);

        /** Returns the error_code from the operation. */
        const ana::error_code& error() const
        {
            return error_code_;
        }

    private:
        virtual void handle_receive   (ana::error_code, ana::net_id, ana::read_buffer);
        virtual void handle_disconnect(ana::error_code, ana::net_id);

        void handle_timeout(ana::error_code error_code);

        ana_component_set::iterator iterator_;

        boost::mutex             mutex_;
        boost::mutex             handler_mutex_;
        boost::mutex             timeout_called_mutex_;
        ana::error_code          error_code_;
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
         * @param component : A network component running an io_service
         *                    which supports timeout capabilities.
         * @param timeout_ms : Amount of milliseconds to timeout the operation.
         */
        void wait_completion(size_t timeout_ms = 0);

        /** Returns the error_code from the operation. */
        const ana::error_code& error() const
        {
            return error_code_;
        }

        /** Returns the buffer from the operation. */
        ana::read_buffer buffer() const
        {
            return buffer_;
        }

        network::connection get_wesnoth_id() const
        {
            return wesnoth_id_;
        }

    private:
        virtual void handle_receive   (ana::error_code, ana::net_id, ana::read_buffer);
        virtual void handle_disconnect(ana::error_code, ana::net_id);

        void handle_timeout(ana::error_code error_code);

        ana_component_set& components_;

        boost::mutex             mutex_;
        boost::mutex             handler_mutex_;
        boost::mutex             timeout_called_mutex_;
        ana::error_code          error_code_;
        ana::read_buffer buffer_;
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
         * @param timer : A pointer to a running timer dealing with the timeout
         *                of this connect operation.
         */
        ana_connect_handler( );

        /** Destructor. */
        ~ana_connect_handler();

        /**
         * Checks if an error occurred during the connection procedure.
         *
         * @returns Error code of the operation.
         */
        const ana::error_code& error() const;

        /** Locks current thread until the connection attempt has finished. */
        void wait_completion();

    private:
        virtual void handle_connect(ana::error_code error_code, ana::net_id /*client*/);

        boost::mutex       mutex_;
        ana::error_code    error_code_;
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

        network::connection new_connection_id( );

        /**
         * Get the associated stats of a given component.
         *
         * @param connection_num : The ID of the network component.
         *
         * @returns A pointer to an ana::stats object of the given component.
         */
        const ana::stats* get_stats( network::connection connection_num = 0,
                                     ana::stat_type type = ana::ACCUMULATED);

        /** Close all connections and clean up memory. */
        void close_connections_and_cleanup();

        /** Throw a Client Disconnected network::error if a disconnection hasn't been informed. */
        void throw_if_pending_disconnection();

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
        size_t send_all( const config& cfg );

        /** Send data to the component with a given ID. */
        size_t send( network::connection connection_num , const config& cfg );

        size_t send_raw_data( const char*, size_t, network::connection);

        void send_all_except(const config& cfg, network::connection connection_num);

        /**
         * Read a message from a given component or from every one.
         *
         * @param connection_num : The id of the network component, 0 to read from every component.
         * @param cfg : The config input to place the read data.
         * @param timeout_ms : Amount of milliseconds to wait for the data.
         *
         * @returns The network::connection number of the component that read
         *          the data or 0 if an error occurred.
         */
        network::connection read_from( network::connection connection_num,
                                       config&             cfg,
                                       size_t              timeout_ms = 0 );

        /**
         * Read a message from a given component or from every one.
         *
         * @param it : The ana component to read from.
         * @param cfg : The config input to place the read data.
         * @param timeout_ms : Amount of milliseconds to wait for the data.
         *
         * @returns The network::connection number of the component that read
         *          the data or 0 if an error occurred.
         */
        network::connection read_from( const ana_component_set::iterator& it,
                                       config&             cfg,
                                       size_t              timeout_ms = 0 );

        network::connection read_from_all( std::vector<char>& );

        /**
         * Read a message from a given component or from every one.
         *
         * @param it : The ana component to read from.
         * @param cfg : The config input to place the read data.
         *
         * @returns The network::connection number of the component that read
         *          the data or 0 if an error occurred.
         */
        network::connection read_from_ready_buffer( const ana_component_set::iterator& it,
                                                    config&                            cfg);

        /** Retrieve upload statistics on a given component. */
        network::statistics get_send_stats(network::connection handle);

        /** Retrieve download statistics on a given component. */
        network::statistics get_receive_stats(network::connection handle);

        /**
         * Disconnect a given client.
         *
         * @param handle : The ID of the component (should be a client) to be disconnected.
         */
        bool disconnect( network::connection handle);

        //@{
        /**
         * Attempt to connect through a proxy (as opposed to directly.)
         *
         * Use the set_proxy_* methods to configure the connection options.
         */
        void enable_connection_through_proxy();

        /**
         * Set the address of the proxy. Default: "localhost".
         *
         * @param address: Network address where the proxy server should be running.
         */
        void set_proxy_address ( const std::string& address  );

        /**
         * Set the port of the proxy. Default: "3128".
         *
         * @param port: Network port where the proxy server should be listening.
         */
        void set_proxy_port    ( const std::string& port     );

        /**
         * Set the user to authenticate with the proxy. Default: "".
         *
         * @param user: User name to use for authentication purposes.
         */
        void set_proxy_user    ( const std::string& user     );

        /**
         * Set the password to authenticate with the proxy. Default: "".
         *
         * @param password: Password to use for authentication purposes.
         */
        void set_proxy_password( const std::string& password );
        //@}
    private:
        virtual void handle_send(ana::error_code, ana::net_id, ana::operation_id);

        virtual void handle_receive( ana::error_code          error,
                                     ana::net_id              client,
                                     ana::read_buffer buffer);

        virtual void handle_disconnect(ana::error_code /*error_code*/, ana::net_id client);

        struct proxy_settings
        {
            proxy_settings() :
                enabled(false),
                address(),
                port(),
                user(),
                password()
            {
            }

            bool        enabled;
            std::string address;
            std::string port;
            std::string user;
            std::string password;
        };

        /**
         * Pack a config object to an outpt stream using compression.
         *
         * @param cfg : The config object as input.
         * @param out : The output stream as output.
         */
        void compress_config( const config& cfg, std::ostringstream& out);

        std::string compress_config( const config& cfg);

        /**
         * Read a config object from an input buffer.
         *
         * @param buffer : The buffer with the compressed stream as input.
         * @param cfg : The config object as output.
         */
        void read_config( const ana::read_buffer& buffer, config& cfg);

        // Attributes
        ana::timer*                connect_timer_;
        ana_component_set          components_;

        std::map< ana::server*, clients_manager* > server_manager_;

        /** Clients that have disconnected from servers (used for client applications.) */
        std::queue< ana_component* > disconnected_components_;

        /** Client IDs that have disconnected from a serve (used in servers applications.) */
        std::queue< ana::net_id >    disconnected_ids_;

        proxy_settings               proxy_settings_;
};

#endif
