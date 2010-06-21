/* $Id$ */

/**
 * @file ana.hpp
 * @brief Main include file for application developers that wish to use ana.
 *
 * ana: Asynchronous Network API.
 * Copyright (C) 2010 Guillermo Biset.
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

/**
 * @mainpage ana: Asynchronous Network API
 *
 * @author Guillermo Biset
 *
 * @section intro Introduction
 *
 * ana is an API to develop simple server and client applications.
 *
 * @image html ana.png
 *
 * This project is being carried out as part of a Google Summer of Code 2010
 * project to reimplement <A HREF="http://wesnoth.org"> Wesnoth </A>'s stack.
 *
 * The projects' webpage is located at
 * <A HREF="http://async-net-api.googlecode.com"> GoogleCode </A>
 *
 * @section requirements requirements
 * To compile ana, you need:
 *  - Boost, version 1.35 or newer
 */

#include <boost/cstdint.hpp>
#include <boost/noncopyable.hpp>
#include <boost/system/error_code.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include <string>
#include <cstdlib>
#include <ctime>

#ifndef ANA_HPP
#define ANA_HPP

#define ANA_DETAIL_INTERNAL_HPP
#include "common.hpp"               //Main definitions
#include "timers.hpp"               //Timer related
#include "predicates.hpp"           //Client predicates, used for conditional sending
#include "binary_streams.hpp"       //For serialization
#undef  ANA_DETAIL_INTERNAL_HPP

/** @namespace ana
 *
 * Namespace for project ana, the entire API is under this namespce.
 */
namespace ana
{
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
        /**
         * Handle an incoming message event.
         *
         * @param error_code : Error code of the client sending the message,
         *                     if it evaluates to false, then no error occurred.
         * @param client_id  : ID of the client that sends the message.
         * @param shared_buffer : The buffer from the incoming message.
         *
         * \sa read_buffer
         * \sa error_code
         * \sa client_id
         * \sa ServerID
         */
        virtual void handle_message   (error_code, client_id, detail::read_buffer) = 0;

        /**
         * Handle a disconnect event.
         *
         * @param error_code : Error code of the disconnecting client, it could
         *                     shed some light into why it got disconnected.
         *
         * @param client_id  : ID of the client that gets disconnected.
         *
         * \sa error_code
         * \sa client_id
         * \sa ServerID
         */
        virtual void handle_disconnect(error_code, client_id) = 0;
    };

    /** Used for implementation purposes. */
    namespace detail
    {
        /** Last issued client_id.  */ 
        static client_id last_client_id_ = 0;

        /**
         * Base class for any network entity that handles incoming messages.
         */
        class listener
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
                 * Get the ID of this listener.
                 *
                 * @returns : ID of the network component represented by this listener.
                 */
                client_id id() const {return id_;}

            protected:
                listener() :
                    id_(++last_client_id_)
                {
                }

                /** Start listening for incoming messages. */
                virtual void run_listener()                                     = 0;

                const client_id     id_               /** This proxy's client_id. */ ;
        };
    } //namespace details

    /**
     * Class that should be implemented to handle new connection events.
     */
    struct connection_handler
    {
        /**
          * Handle new connection event.
          *
          * @param error_code : Error code of the event.
          *
          * @param client_id  : ID of the client that connects.
          *
          * \sa error_code
          * \sa client_id
          * \sa ServerID
          */
        virtual void handle_connect( error_code, client_id )  = 0;
    };

    /**
     * Class that should be implemented to handle send completion events.
     */
    struct send_handler
    {
        /**
         * Handle send completion event.
         *
         * @param error_code : Error code of the event.
         *
         * @param client_id  : ID of the client that sent the message.
         *
         * \sa error_code
         * \sa client_id
         * \sa ServerID
         */
        virtual void handle_send( error_code, client_id ) = 0;
    };
    //@}

    /** @name Time duration functions. */
    //@{
    /** @namespace time
     *
     * Time conversion functions.
     */
    namespace time
    {
        /**
         * Create a time lapse from a given amount of milliseconds.
         *
         * @param ms : Milliseconds of elapsed time, must be a positive integer value.
         *
         * @returns : A time duration amount (in milliseconds) to be used with timers.
         */
        inline size_t milliseconds(size_t ms) { return ms;              }

        /**
         * Create a time lapse from a given amount of seconds.
         *
         * @param ms : Seconds of elapsed time.
         *
         * @returns : A time duration amount (in milliseconds) to be used with timers.
         */
        inline size_t seconds(double s)       { return size_t(s * 1000);}

        /**
         * Create a time lapse from a given amount of minutes.
         *
         * @param ms : Minutes of elapsed time.
         *
         * @returns : A time duration amount (in milliseconds) to be used with timers.
         */
        inline size_t minutes(double m)       { return seconds(m * 60); }

        /**
         * Create a time lapse from a given amount of hours.
         *
         * @param ms : Hours of elapsed time.
         *
         * @returns : A time duration amount (in milliseconds) to be used with timers.
         */
        inline size_t hours(double h)         { return minutes(h * 60); }
    }
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
                    public         detail::timed_sender
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
         * @param buffer : Buffer to be sent. Should be constucted with the buffer function.
         * @param handler : Handler of events resulting from this operation. It will be called with _each_
         *                  event, meaning that it will be called exactly once for every connected client.
         * @param send_type : Optional, type of the send operation. Defaults to a value of CopyBufer.
         *
         * Examples:
         *    - server_->send_all( ana::buffer( str ), this );
         *    - server_->send_all( ana::buffer( large_pod_array ), handler_object, ana::ZeroCopy );
         *
         * \sa send_type
         * \sa buffer
         * \sa send_handler
         */
        virtual void send_all(boost::asio::const_buffer, send_handler*, send_type = CopyBuffer )            = 0;

        /**
         * Send a buffer to every connected client that satisfies a given condition/property.
         *
         * @param buffer : Buffer to be sent. Should be constucted with the buffer function.
         * @param handler : Handler of events resulting from this operation. It will be called exactly
         *                  once for every client that holds the property.
         * @param send_type : Optional, type of the send operation. Defaults to a value of CopyBufer.
         *
         * Examples:
         *    - server_->send_if( ana::buffer( str() ), this,
         *                        create_predicate( boost::bind( std::not_equal_to<client_id>(), client, _1) ) );
         *
         * \sa client_predicate
         * \sa send_type
         * \sa buffer
         * \sa send_handler
         */
        virtual void send_if(boost::asio::const_buffer,
                                send_handler*, const client_predicate&, send_type = CopyBuffer )               = 0;

        /**
         * Send a buffer to every connected client except one. Equals a send_all if the client doesn't exist.
         *
         * @param buffer : Buffer to be sent. Should be constucted with the buffer function.
         * @param handler : Handler of a possible event resulting from this operation.
         * @param send_type : Optional, type of the send operation. Defaults to a value of CopyBufer.
         *
         * Examples:
         *    - server_->send_all_except( client, ana::buffer( str ), this, ana::ZeroCopy);
         *
         * \sa send_type
         * \sa buffer
         * \sa send_handler
         */
        virtual void send_all_except(client_id, boost::asio::const_buffer, send_handler*, send_type = CopyBuffer ) = 0;

        /**
         * Send a buffer to a connected client with a given client_id. Does nothing if no such client exists.
         *
         * @param buffer : Buffer to be sent. Should be constucted with the buffer function.
         * @param handler : Handler of a possible event resulting from this operation.
         * @param send_type : Optional, type of the send operation. Defaults to a value of CopyBufer.
         *
         * Examples:
         *    - server_->send_one( client, ana::buffer( str ), this, ana::ZeroCopy);
         *
         * \sa send_type
         * \sa buffer
         * \sa send_handler
         */
        virtual void send_one(client_id, boost::asio::const_buffer, send_handler*, send_type = CopyBuffer ) = 0;

        /**
         * Set the handler for new connection events.
         *
         * \sa connection_handler
         */
        virtual void set_connection_handler( connection_handler* ) = 0;

        /**
         * Start the server on a given port.
         *
         * @param port : The port to be used for the server incoming connections. The port shouldn't be
         *               currently occupied.
         */
        virtual void run(port pt)                                  = 0;

        /** Standard destructor. */
        virtual ~server() {}

        /**
         * A connected client's representative in the server side.
         */
        struct client_proxy : public virtual detail::listener,
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
             * \sa timed_sender
             */
            virtual void send(detail::shared_buffer, send_handler*, timed_sender* ) = 0;

            /** Standard destructor. */
            virtual ~client_proxy() {}

            // Allow server objects to invoke run_listener directly.
            using detail::listener::run_listener;
        };
    };

    /**
     * A network client.
     *
     * \sa listener
     * \sa timed_sender
     */
    struct client : public virtual detail::listener,
                    public         detail::timed_sender
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
         * \sa connection_handler
         */
        virtual void connect( connection_handler* ) = 0;

        /**
         * Attempt a connection to the server through a proxy.
         *
         * @param handler : The handler of the connection event.
         *
         * \sa connection_handler
         */
        virtual void connect_through_proxy(std::string proxy_address,
                                           std::string proxy_port,
                                           connection_handler* handler,
                                           std::string user_name = "",
                                           std::string password = "") = 0;

        /** Run the client listener, starts listening for incoming messages. */
        virtual void run() = 0;

        /**
         * Send a buffer or memory portion to the server.
         *
         * Examples:
         *    - client_->send( ana::buffer( str ), this );
         *    - client_->send( ana::buffer( large_pod_array ), handler_object, ana::ZeroCopy );
         *
         * @param buffer : The buffer being sent.
         * @param handler : Handler of events resulting from this operation.
         * @param send_type : Optional, type of the send operation. Defaults to a value of CopyBufer.
         *
         * \sa send_type
         * \sa buffer
         * \sa send_handler
         */
        virtual void send(boost::asio::const_buffer buffer, send_handler* handler, send_type type = CopyBuffer ) = 0;

        /** Standard destructor. */
        virtual ~client() {}
    };
    //@}
}

#endif
