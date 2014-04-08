
/**
 * @file
 * @brief Main include file for application developers that wish to use ana.
 *
 * ana: Asynchronous Network API.
 * Copyright (C) 2010 - 2014 Guillermo Biset.
 *
 * This file is part of the ana project.
 *
 * Contents:       Main header file for ana providing the whole public API.
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

#ifdef DOXYGEN_ENABLE_ANA_MAINPAGE
/**
 * @mainpage ana: Asynchronous Network API
 *
 * @author Guillermo Biset
 *
 * @section intro Introduction
 *
 * ana is an API to develop simple server and client applications.
 *
 * @image html logo.svg
 *
 * This project is being carried out as part of a Google Summer of Code 2010
 * project to reimplement <A HREF="http://wesnoth.org"> Wesnoth</A>'s stack.
 *
 * The project's source code can be found in the src/ana directory of
 * the Battle for Wesnoth repository
 *
 * @section requirements Requirements
 * To compile ana, you need:
 *  - Boost, version 1.35 or newer
 */
#endif

#include <boost/cstdint.hpp>
#include <boost/noncopyable.hpp>
#include <boost/system/error_code.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/asio/detail/socket_ops.hpp>

#include <string>
#include <cstdlib>
#include <ctime>

#ifndef ANA_HPP
#define ANA_HPP

#include "common.hpp"               //Main definitions
#include "timers.hpp"               //Timer related
#include "stats.hpp"                //Network statistics
#include "predicates.hpp"           //Client predicates, used for conditional sending
#include "binary_streams.hpp"       //For serialization

/** @namespace ana
 *
 * Namespace for project ana, the entire API is under this namespace.
 */
namespace ana
{
    /** @name Miscellaneous Functions
     *
     * Functions for data conversions.
     */
    //@{
    /** Converts a 32 bit number in network byte order to a local number. */
    inline void network_to_host_long(ana_uint32& value)
    {
        value = ntohl(value);
    }
    /** Converts a 32 bit number into a network byte order number. */
    inline void host_to_network_long(ana_uint32& value)
    {
        value = htonl(value);
    }
    /** Converts a 16 bit number in network byte order to a local number. */
    inline void network_to_host_short(ana_uint16& value)
    {
        value = ntohs(value);
    }
    /** Converts a 16 bit number into a network byte order number. */
    inline void host_to_network_short(ana_uint16& value)
    {
        value = htons(value);
    }
    //@}

    /** @name Handler Interfaces
     *
     * Interfaces to handle network events.
     */
    //@{
    /**
     * Class that should be implemented to handle incoming messages or disconnections.
     */
    struct listener_handler
    {
        virtual ~listener_handler() {}

        /**
         * Handle an incoming message event.
         *
         * @param error_code : Error code of the client sending the message,
         *                     if it evaluates to false, then no error occurred.
         * @param net_id  : ID of the client that sends the message.
         * @param shared_buffer : The buffer from the incoming message.
         *
         * \sa read_buffer
         * \sa error_code
         * \sa net_id
         */
        virtual void handle_receive   (error_code, net_id, read_buffer) = 0;

        /**
         * Handle a disconnect event.
         *
         * @param error_code : Error code of the disconnecting client, it could
         *                     shed some light into why it got disconnected.
         *
         * @param net_id  : ID of the client that gets disconnected.
         *
         * \sa error_code
         * \sa net_id
         */
        virtual void handle_disconnect(error_code, net_id) = 0;
    };

    /**
     * Interface for every component that logs network statistics.
     */
    struct network_stats_logger
    {
        virtual ~network_stats_logger() {}

        /**
         * Get the associated collected stats as per a stat_type.
         *
         * @param type : stat_type to be collected ( ACCUMULATED, SECONDS, MINUTES, HOURS, DAYS )
         *
         * @returns A const pointer to a stats object holding the stats.
         *
         * \sa stats
         * \sa stat_type
         */
        virtual const stats* get_stats( stat_type type = ACCUMULATED ) const = 0;
    };

    /** Used for implementation purposes. */
    namespace detail
    {
        /** Last issued net_id.  */
        static net_id last_net_id_ = 0;

        /** Base class of network components. */
        class ana_component
        {
            public:
                virtual ~ana_component() {}

                /**
                 * Get the ID of this component.
                 *
                 * @returns : ID of the network component represented by this object.
                 */
                net_id id() const {return id_;}

                /**
                 * Disconnect the component.
                 */
                virtual void disconnect() = 0;

                /**
                 * Enter Raw Data mode, ana won't prefix your packets with header information.
                 *
                 * This is good for handshake procedures or every time you know how much you are
                 * supposed to receive. Combine this mode with a listener that is reading things
                 * the right way.
                 *
                 * \sa listener
                 */
                void set_raw_data_mode()     { raw_data_ = true;  }

                /**
                 * Enter header first mode, ana will prefix anything you send with size information
                 * first, so the listener will inform a new packet has been received only after it
                 * receives the whole packet.
                 */
                void set_header_first_mode() { raw_data_ = false; }

                /** Returns true iff the sender is in raw data mode. */
                bool raw_mode()    const {return raw_data_;   }

                /** Returns false iff the sender is in raw data mode. */
                bool header_mode() const {return ! raw_data_; }

                /**
                 * Get associated stats_collector object.
                 *
                 * @returns A pointer to the associated stats_collector object,
                 *          NULL if not keeping stats.
                 *
                 * \sa stats_collector.
                 */
                virtual ana::stats_collector& stats_collector() = 0;

            protected:
                /** Initialize component, assign fresh id and sets header-first and async modes. */
                ana_component() :
                    raw_data_( false ),
                    id_(++last_net_id_)
                {
                }

            private:
                /** The component is in raw data mode.*/
                bool raw_data_;

                /** This component's net_id. */
                const net_id     id_;
        };

        /**
         * Base class for any network entity that handles incoming messages.
         */
        class listener : public virtual network_stats_logger,
                         public virtual ana_component
        {
            public:
                /**
                 * Sets the handler for incoming messages.
                 *
                 * @param listener : Pointer to the listener_handler object that will
                 *                   handle following incoming message events.
                 *
                 * \sa listener_handler
                 */
                virtual void set_listener_handler( listener_handler* listener ) = 0;

                /**
                 * Sets the size of raw buffer.
                 *
                 * @param size : The requested size for raw buffers.
                 *
                 * @pre : Parameter should be positive.
                 */
                virtual void set_raw_buffer_max_size( size_t size ) = 0;

                /**
                 * Block the caller waiting for an incoming message of a certain amount of bytes.
                 *
                 * @param bis : Binary stream where the data will be stored.
                 * @param size : The amount of bytes trying to be read.
                 */
                virtual void wait_raw_object(ana::serializer::bistream& bis, size_t size) = 0;

                /** Start listening for incoming messages. */
                virtual void run_listener() = 0;

            protected:
                listener() {}
        };

        /** Provides send option setting to network components. */
        struct sender : public timed_sender,
                        public virtual ana_component
        {
        };
    } //namespace detail

    /**
     * Class that should be implemented to handle new connection events.
     */
    struct connection_handler
    {
        virtual ~connection_handler() {}

        /**
          * Handle new connection event.
          *
          * @param error_code : Error code of the event.
          *
          * @param net_id  : ID of the client that connects.
          *
          * \sa error_code
          * \sa net_id
          */
        virtual void handle_connect( error_code, net_id )  = 0;
    };

    /**
     * Class that should be implemented to handle send completion events.
     */
    struct send_handler
    {
        virtual ~send_handler() {}

        /**
         * Handle send completion event.
         *
         * @param error_code : Error code of the event.
         *
         * @param net_id  : ID of the client that sent the message.
         *
         * \sa error_code
         * \sa net_id
         */
        virtual void handle_send( error_code, net_id, operation_id ) = 0;
    };
    //@}

    /** @name Main classes.
     *
     * Main classes in ana.
     */
    //@{
    /**
     * A network server. An object of this type can handle several connected clients.
     */
    struct server : public virtual detail::listener,
                    public virtual detail::sender
    {
        /**
         * Creates an ana server.
         *
         * Examples:
         *     - ana::server* server = ana::server::create();
         */
        static server* create();

        /**
         * Send a buffer to every connected client.
         *
         * @param buffer : Buffer to be sent. Should be constructed with the buffer function.
         * @param handler : Handler of events resulting from this operation. It will be called
         *                  with _each_ event, meaning that it will be called exactly once
         *                  for every connected client.
         * @param send_type : Optional, type of the send operation.
         *                    Defaults to a value of COPY_BUFFER.
         *
         * Examples:
         *    - server_->send_all( ana::buffer( str ), this );
         *    - server_->send_all( ana::buffer( large_pod_array ), handler_object, ana::ZERO_COPY );
         *
         * @returns : The unique operation id of the send operation.
         *
         * \sa send_type
         * \sa buffer
         * \sa send_handler
         */
        virtual operation_id send_all(boost::asio::const_buffer buffer,
                                      send_handler*             handler,
                                      send_type                 type = COPY_BUFFER ) = 0;

        /**
         * Send a buffer to every connected client that satisfies a given condition/property.
         *
         * @param buffer : Buffer to be sent. Should be constructed with the buffer function.
         * @param handler : Handler of events resulting from this operation. It will be called
         *                  exactly once for every client that holds the property.
         * @param send_type : Optional, type of the send operation. Defaults to COPY_BUFFER.
         *
         * Examples:
         *  - server_->send_if( ana::buffer( str() ), this,
         *             create_predicate( boost::bind( std::not_equal_to<net_id>(), client, _1) ) );
         *
         * @returns : The unique operation id of the send operation.
         *
         * \sa client_predicate
         * \sa send_type
         * \sa buffer
         * \sa send_handler
         */
        virtual operation_id send_if(boost::asio::const_buffer buffer,
                                     send_handler*             handler,
                                     const client_predicate&   predicate,
                                     send_type                 type = COPY_BUFFER ) = 0;

        /**
         * Send a buffer to every connected client except one.
         * Equals a send_all if the client doesn't exist.
         *
         * @param buffer : Buffer to be sent. Should be constructed with the buffer function.
         * @param handler : Handler of a possible event resulting from this operation.
         * @param send_type : Optional, type of the send operation. Defaults to COPY_BUFFER.
         *
         * Examples:
         *    - server_->send_all_except( client, ana::buffer( str ), this, ana::ZERO_COPY);
         *
         * @returns : The unique operation id of the send operation.
         *
         * \sa send_type
         * \sa buffer
         * \sa send_handler
         */
        virtual operation_id send_all_except(net_id                    except_id,
                                             boost::asio::const_buffer buffer,
                                             send_handler*             handler,
                                             send_type                 type = COPY_BUFFER ) = 0;

        /**
         * Send a buffer to a connected client with a given net_id.
         * Does nothing if no such client exists.
         *
         * @param buffer : Buffer to be sent. Should be constructed with the buffer function.
         * @param handler : Handler of a possible event resulting from this operation.
         * @param send_type : Optional, type of the send operation. Defaults to COPY_BUFFER.
         *
         * Examples:
         *    - server_->send_one( client, ana::buffer( str ), this, ana::ZERO_COPY);
         *
         * @returns : The unique operation id of the send operation.
         *
         * \sa send_type
         * \sa buffer
         * \sa send_handler
         */
        virtual operation_id send_one(net_id                    id,
                                      boost::asio::const_buffer buffer,
                                      send_handler*             handler,
                                      send_type                 type = COPY_BUFFER ) = 0;

        /**
         * Set the handler for new connection events.
         *
         * \sa connection_handler
         */
        virtual void set_connection_handler( connection_handler* ) = 0;

        /**
         * Start the server on a given port.
         *
         * Each time you call this method, a new thread will be started on the io_service object
         * from asio. This means that it is possible to have multiple threads running the service,
         * thus more threads will be able to run the handlers you implement.
         *
         * The drawback is, however, that if you run the service on multiple threads, then you must
         * be aware that the execution of your handlers may occur concurrently and thus you have to
         * prevent all of the troubles arising from this concurrency.
         *
         * Note that if you just call this method once, then you ensure mutual exclusion between
         * your handlers, just make sure you don't block waiting for a call to a handler from one
         * of your handlers, otherwise you'll always get a deadlock.
         *
         * @param port : The port to be used for the server incoming connections.
         *               The port shouldn't be currently occupied.
         */
        virtual void run(port port)                                = 0;

        /**
         * Disconnect a connected client by force.
         *
         * @param id : The net_id of the connected client.
         *
         * \sa net_id
         */
        virtual void disconnect( net_id id )                       = 0;

        /** Set a client to header-first mode. */
        virtual void set_header_first_mode( net_id id )            = 0;

        /** Allow external object to call set_header_first_mode() directly. */
        using ana::detail::ana_component::set_header_first_mode;

        /** Set a client to raw-data mode.     */
        virtual void set_raw_data_mode( net_id id )                = 0;

        /**
         * Cancel all pending network operations.
         *
         * Every pending operation handler will be invoked with ana::operation_aborted
         * as the corresponding error_code, except that the error code will be
         * boost::asio::error::operation_not_supported when run on Windows XP,
         * Windows Server 2003, and earlier versions of Windows,
         * unless BOOST_ASIO_ENABLE_CANCELIO is defined.
         */
        virtual void cancel_pending( )                             = 0;

        /**
         * Cancel all pending network operations for a given client.
         * Does nothing if the client_id doesn't belong to a connected client.
         * Every pending operation handler will be invoked with ana::operation_aborted
         * as the corresponding error_code.
         *
         * @param client_id : Network ID of the client.
         */
        virtual void cancel_pending( ana::net_id client_id )       = 0;

        /* Allow external object to call set_raw_data_mode() directly. */
        /**
         * Set the server to raw data mode, every time a client connects
         * it will use the current mode.
         */
        using ana::detail::ana_component::set_raw_data_mode;

        /** Returns the string representing the ip address of a connected client. */
        virtual std::string ip_address( net_id ) const = 0;

        /** Returns a pointer to an ana::stats object of a connected client. */
        virtual const stats* get_client_stats( net_id, stat_type ) const = 0;

        /**
         * Signal the server that you are waiting for a message from a given client in a certain
         * period of time.
         *
         * The time parameter indicates how long you are willing to wait.
         *
         * If a message is received before this time period then this call will be insignificant.
         * However, if no such message is received, the appropriate call to handle_receive will be
         * made with ana::timeout_error as the error_code parameter.
         *
         * @param id : The ana::net_id of the client you are expecting the message from. If the
         *             id is invalid, this call will have no effect.
         * @param time : The amount of time you are willing to wait.
         *
         * Use the methods described in the ana::time namespace to create time lapses.
         *
         * Examples:
         *     - client->waiting_for_message( ana::time::seconds( 5 ) );
         *
         * @sa error_code
         * @sa ana::time
         */
        virtual void expecting_message( net_id, size_t ms_until_timeout ) = 0;

        /** Standard destructor. */
        virtual ~server() {}

        /**
         * A connected client's representative in the server side.
         */
        struct client_proxy : public virtual detail::listener,
                              public         detail::sender,
                              boost::noncopyable
        {
            /**
             * Send a buffer to the corresponding client.
             *
             * @param buffer : The memory portion or buffer being sent.
             * @param handler : The handler of the completion or error event.
             * @param sender : The object with the timeout configuration.
             *
             * \sa shared_buffer
             * \sa send_handler
             * \sa sender
             */
            virtual void send(detail::shared_buffer, send_handler*, sender*, operation_id ) = 0;

            /** Standard destructor. */
            virtual ~client_proxy() {}

            /**
             * Cancel all pending network operations.
             *
             * Every pending operation handler will be invoked with ana::operation_aborted
             * as the corresponding error_code, except that the error code will be
             * boost::asio::error::operation_not_supported when run on Windows XP,
             * Windows Server 2003, and earlier versions of Windows,
             * unless BOOST_ASIO_ENABLE_CANCELIO is defined.
             */
            virtual void cancel_pending() = 0;

            /** Returns the string representing the ip address of the connected client. */
            virtual std::string ip_address() const = 0;

            /**
             * Signal the client proxy that you are waiting for a message from the actual client
             * before a given time.
             *
             * The time parameter indicates how long you are willing to wait.
             *
             * If a message is received before this time period then this call will be
             * insignificant.
             * However, if no such message is received, the appropriate call to handle_receive will
             * be made with ana::timeout_error as the error_code parameter.
             *
             * Use the methods described in the ana::time namespace to create time lapses.
             *
             * Examples:
             *     - client->waiting_for_message( ana::time::seconds( 5 ) );
             *
             * @sa error_code
             */
            virtual void expecting_message( size_t ms_until_timeout ) = 0;

            // Allow server objects to invoke run_listener directly.
            using detail::listener::run_listener;

            /** Allow external object to call set_header_first_mode() directly. */
            using ana::detail::ana_component::set_header_first_mode;

            /** Allow external object to call set_raw_data_mode() directly. */
            using ana::detail::ana_component::set_raw_data_mode;
        };
    };

    /**
     * A network client.
     *
     * \sa listener
     * \sa sender
     */
    struct client : public virtual detail::listener,
                    public         detail::sender
    {
        /**
         * Creates a client.
         *
         * @param address : The network address of the server.
         * @param port : The network port of the server.
         */
        static client* create(address address, port port);

        /**
         * Attempt a connection to the server.
         *
         * @param handler : The handler of the connection event.
         *
         * @returns : The unique operation id of the connect operation.
         *
         * \sa connection_handler
         */
        virtual void connect( connection_handler* ) = 0;

        /**
         * Attempt a connection to the server through a proxy.
         *
         * @param handler : The handler of the connection event.
         *
         * @returns : The unique operation id of the connect operation.
         *
         * \sa connection_handler
         */
        virtual void connect_through_proxy(std::string         proxy_address,
                                           std::string         proxy_port,
                                           connection_handler* handler,
                                           std::string         user_name = "",
                                           std::string         password  = "") = 0;

        /**
         * Run the client listener, starts listening for incoming messages.
         *
         * Each time you call this method, a new thread will be started on the io_service object
         * from asio. This means that it is possible to have multiple threads running the service,
         * thus more threads will be able to run the handlers you implement.
         *
         * The drawback is, however, that if you run the service on multiple threads, then you must
         * be aware that the execution of your handlers may occur concurrently and thus you have to
         * prevent all of the troubles arising from this concurrency.
         *
         * Note that if you just call this method once, then you ensure mutual exclusion between
         * your handlers, just make sure you don't block waiting for a call to a handler from one
         * of your handlers, otherwise you'll always get a deadlock.
         */
        virtual void run() = 0;

        /**
         * Send a buffer or memory portion to the server.
         *
         * Examples:
         *    - client_->send( ana::buffer( str ), this );
         *    - client_->send( ana::buffer( large_pod_array ), handler_object, ana::ZERO_COPY );
         *
         * @param buffer : The buffer being sent.
         * @param handler : Handler of events resulting from this operation.
         * @param send_type : Optional, type of the send operation. Defaults to COPY_BUFFER.
         *
         * @returns : The unique operation id of the send operation.
         *
         * \sa send_type
         * \sa buffer
         * \sa send_handler
         */
        virtual operation_id send(boost::asio::const_buffer buffer,
                                  send_handler*             handler,
                                  send_type                 type = COPY_BUFFER ) = 0;

        /**
         * Cancel all pending network operations.
         *
         * Every pending operation handler will be invoked with ana::operation_aborted
         * as the corresponding error_code, except that the error code will be
         * boost::asio::error::operation_not_supported when run on Windows XP,
         * Windows Server 2003, and earlier versions of Windows,
         * unless BOOST_ASIO_ENABLE_CANCELIO is defined.
         */
        virtual void cancel_pending( )                             = 0;

        /**
         * Signal the client that you are waiting for a message from the server before a given time.
         *
         * The time parameter indicates how long you are willing to wait.
         *
         * If a message is received before this time period then this call will be insignificant.
         * However, if no such message is received, the appropriate call to handle_receive will be
         * made with ana::timeout_error as the error_code parameter.
         *
         * Use the methods described in the ana::time namespace to create time lapses.
         *
         * Examples:
         *     - client->waiting_for_message( ana::time::seconds( 5 ) );
         *
         * @sa error_code
         */
        virtual void expecting_message( size_t ms_until_timeout ) = 0;

        /**
         * Set a timeout value for connection attempts. If attempting to connect through a proxy
         * this value will be used once for the whole connection attempt.
         *
         * You should use the time lapse constructors in namespace ana::time:
         *      - ana::time::seconds(10)
         *      - ana::time::minutes(1)
         *
         * @sa ana::time
         */
        virtual void set_connect_timeout( size_t ms ) = 0;

        /** Returns the string representing the ip address of the connected client. */
        virtual std::string ip_address() const = 0;

        /** Standard destructor. */
        virtual ~client() {}
    };
    //@}
}

#endif
