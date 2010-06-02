/**
 * @file ana.hpp
 * @brief Main include file for application developers that wish to use ana.
 *
 * ana: Asynchronous Network API.
 * <http://async-net-api.googlecode.com/>
 * Copyright (C) 2010 Guillermo Biset.
 *
 * This file is part of the ana project.
 *
 * Contents:       Main header file for ana providing the whole public API.
 *
 * System:         ana
 * Homepage:       <http://async-net-api.googlecode.com/>
 * Language:       C++
 *
 * Author:         Guillermo Biset
 * E-Mail:         billybiset AT gmail.com
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
 * - Boost, version 1.37 or older
 *
 */

#ifndef ANA_HPP
#define ANA_HPP

#include <boost/cstdint.hpp>
#include <boost/noncopyable.hpp>
#include <boost/system/error_code.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include <string>
#include <cstdlib>
#include <ctime>

/** @namespace ana
 *
 * Namespace for project ana, the entire API is under this namespce.
 */
namespace ana
{
    /** @name Type and constant definitions
     *
     * Definitions of main types and relevant constants.
     */
    //@{
    typedef uint32_t ana_uint  /** Standard unsigned int. */ ;
    typedef int32_t  ana_int   /** Standard int.          */ ;

    typedef ana_uint         client_id     /** Type of IDs of connected components, unique. */ ;
    typedef ana_uint         message_size  /** Message size type.                           */ ;

    typedef std::string port               /** Port type.                                   */ ;
    typedef std::string address            /** Address type.                                */ ;

    typedef bool        send_type          /** Send operation type, true to copy the buffer.    */ ;

    typedef boost::system::error_code error_code /** Standard error code, can evaluate to bool. */ ;

    const send_type ZeroCopy   = false     /** Don't copy the buffer. */ ;
    const send_type CopyBuffer = true      /** Copy the buffer.       */ ;

    const message_size HeaderLength = sizeof(ana_uint) /** Length of message header. */ ;

    const client_id ServerID = 0  /** The ID of the server application. */ ;

    /**
     * Timeout policies for send operations.
     *
     * \sa timer
     */
    enum timeout_policy
    {
        NoTimeouts       /** Don't use timers in any operation.                */,
        FixedTime        /** Use timers with a fixed time for every operation. */,
        TimePerKilobyte  /** Use timers, calculating the necessary time from
        the size of the buffer that is to be sent.        */
    };
    //@}

    /** @name Predicates over client ids.
     *
     * Declaration of types used in conditional send operations, e.g. send_if.
     * \sa send_if
     */
    //@{
    /** A boolean predicate of client IDs. Used for conditional send operations. */
    struct client_predicate
    {
        /**
         * Decides if a given condition applies to a client.
         *
         * @param client ID of the queried client.
         * @returns true if the condition holds for this client.
         */
        virtual bool selects(client_id) const = 0;
    };
    //@}

    /** @name Timers
     *
     * Definitions of timer related types.
     */
    //@{

    /**
     * General purpose asynchronous timer.
     */
    class timer
    {
        public:
            /** Standard constructor. */
            timer() :
                io_service_(),
                timer_(io_service_)
            {
            }

            /**
             * Wait in background a given amount of milliseconds.
             *
             * The method shouldn't be called with a size_t constant
             * directly. Instead, the user should use the functions in
             * the ana::time namespace.
             *
             * @param milliseconds : Amount of milliseconds to wait.
             * @param handler : Handler object to handle the timeout event.
             *
             * Examples:
             *    - wait( ana::time::seconds(5),
             *            boost::bind( &ChatServer::handle_timeout, this,
             *                         boost::asio::placeholders::error);
             *
             * \sa ana::time
             */
            template<class Handler>
            void wait(size_t milliseconds, Handler handler)
            {
                timer_.expires_from_now(milliseconds / 1000);
                timer_.async_wait(handler);
                boost::thread t( boost::bind( &boost::asio::io_service::run_one, &io_service_ ) );
            }

            /** Cancel the timer if running. */
            void cancel()
            {
                timer_.cancel();
            }

            /** Standard destructor, cancels pending operations and stops the I/O service. */
            ~timer()
            {
                timer_.cancel();
                io_service_.stop();
            }

        private:
            /** Private class providing traits for the timer type. */
            struct time_t_traits
            {
                // The time type.
                typedef std::time_t time_type;

                // The duration type.
                struct duration_type
                {
                    duration_type() : value(0) {}
                    duration_type(std::time_t v) : value(v) {}
                    std::time_t value;
                };

                // Get the current time.
                static time_type now()
                {
                    return std::time(0);
                }

                // Add a duration to a time.
                static time_type add(const time_type& t, const duration_type& d)
                {
                    return t + d.value;
                }

                // Subtract one time from another.
                static duration_type subtract(const time_type& t1, const time_type& t2)
                {
                    return duration_type(t1 - t2);
                }

                // Test whether one time is less than another.
                static bool less_than(const time_type& t1, const time_type& t2)
                {
                    return t1 < t2;
                }

                // Convert to POSIX duration type.
                static boost::posix_time::time_duration to_posix_duration(const duration_type& d)
                {
                    return boost::posix_time::seconds(d.value);
                }
            };

            boost::asio::io_service io_service_;

            boost::asio::basic_deadline_timer<std::time_t,time_t_traits> timer_;
    };


    /** Used for implementation purposes. */
    namespace detail
    {
        /**
        * Intended for private use, should be created with create_predicate.
        *
        * \sa create_predicate
        */
        template<class predicate>
        class _generic_client_predicate : public client_predicate
        {
            public:
                /** Construct via a generic object with overloaded operator(). */
                _generic_client_predicate(const predicate& pred)
                    : pred(pred)
                {
                }

                /** Copy constructor. */
                _generic_client_predicate(const _generic_client_predicate<predicate>& other )
                    : pred(other.pred)
                {
                }
            private:
                const predicate pred /** The predicate object. */ ;

                /**
                * Implementation of the selection method using operator() from the
                * predicate object.
                *
                * \sa client_predicate
                */
                virtual bool selects(client_id cid) const
                {
                    return pred(cid);
                }
        };

        /**
         * A simple buffer to read things on, supports conversion to string.
         */
        class read_buffer_implementation
        {
            public:
                /**
                 * Creates a new buffer in heap.
                 *
                 * @param size : The size of the buffer.
                 */
                read_buffer_implementation( size_t size ) :
                    base_( new char[ size ] ),
                    size_( size )
                {
                }

                /** Deletes the buffer from heap. */
                ~read_buffer_implementation()
                {
                    delete[] base_;
                }

                /** 
                 * Get the base address. 
                 *
                 * @returns A char* to the base address of the buffer.
                 */
                char* base_char() const
                {
                    return base_;
                }

                /** 
                 * Get the base address. 
                 *
                 * @returns A void* to the base address of the buffer.
                 */
                void* base() const
                {
                    return static_cast<void*>(base_);
                }

                /** Get the size of the buffer. */
                size_t size() const
                {
                    return size_;
                }

                /** Create a string from the buffer. */
                std::string string()
                {
                    return std::string( base_, size_ );
                }

            private:
                char*  base_;
                size_t size_;
        };

        /** A shared pointer to a read_buffer, self destructs when no one is left referencing it. */
        typedef boost::shared_ptr<read_buffer_implementation> read_buffer;

        /**
         * A buffer to be constructed from different buffers that can duplicate the other
         * and hold a local copy that will destruct with the object itself.
         */
        class copying_buffer
        {
            public:
                copying_buffer( boost::asio::const_buffer buffer, ana::send_type copy_buffer) :
                    size_(boost::asio::detail::buffer_size_helper(buffer) ),
                    base_( NULL ),
                    copy_( copy_buffer )
                {
                    if (copy_buffer)
                    {
                        base_ = std::malloc( size_ );
                        std::memcpy(base_, boost::asio::detail::buffer_cast_helper(buffer), size_);
                    }
                    else
                        base_ = const_cast<void*>(boost::asio::detail::buffer_cast_helper(buffer));
                }

                inline size_t size() const { return size_; }

                inline void*  base() const { return base_; }

                ~copying_buffer()
                {
                    if (copy_)
                        std::free( base_ );
                }

            private:
                const size_t size_;
                void*        base_;
                const bool   copy_;
        };

        /** A buffer that can be shared by many users. */
        typedef boost::shared_ptr<detail::copying_buffer> shared_buffer;


        /**
         * A network sender component that can be configured to issue timeout events.
         */
        class timed_sender
        {
            public:
                /**
                 * Set the policy for timed operations (such as send.)
                 *
                 * @param type : Type of timeout policy.
                 * @param ms : Milliseconds related to the given policy, 0 means no timeouts.
                 *
                 * Examples:
                 *    - set_timeouts( ana::NoTimeouts );
                 *    - set_timeouts( ana::FixedTime, ana::time::minutes( 1 ) );
                 *
                 * \sa timeout_policy
                 * \sa ana::time
                 */
                void set_timeouts(timeout_policy type, size_t ms = 0)
                {
                    timeout_milliseconds_ = ms;
                    if (ms > 0)
                        timeout_type_     = type;
                    else
                        timeout_type_     = NoTimeouts;
                }

                /**
                 * Start a timer given the current configuration.
                 *
                 * @param buffer : The buffer used in the send operation.
                 * @param handler : The handler of the timeout/abort event.
                 *
                 * @returns : A pointer to a newly created timer object.
                 */
                template<class Handler>
                timer* start_timer( shared_buffer buffer, Handler handler ) const
                {
                    if ( (timeout_milliseconds_ == 0) || (timeout_type_ == NoTimeouts) )
                        return NULL;
                    else
                    {
                        timer* t = new timer();
                        switch ( timeout_type_ ) //this should be more OO looking
                        {
                            case TimePerKilobyte :
                                t->wait( (buffer->size() / 1024.0) * timeout_milliseconds_, handler);
                                break;
                            default :
                                t->wait( timeout_milliseconds_, handler);
                                break;
                        }
                        return t;
                    }
                }

            protected:
                /** Standard constructor. */
                timed_sender() :
                    timeout_type_( NoTimeouts ),
                    timeout_milliseconds_( 0 )
                {
                }

                timeout_policy timeout_type_            /** Type of timer policy.                 */ ;
                size_t         timeout_milliseconds_    /** Amount of ms relevant to this policy. */ ;
        };
    } //namespace details


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

    namespace detail
    {
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
                 * Get the client_id of this listener.
                 *
                 * @returns : ServerID for the server application, or a comparable client_id
                 *            unique to this listener.
                 *
                 * \sa ServerID
                 * \sa client_id
                 */
                virtual client_id id() const                                    = 0;

            protected: // should be so?
                /** Start listening for incoming messages. */
                virtual void run_listener()                                     = 0;
        };
    }


    /**
      * Creates a client predicate to be used in send operations.
      *
      * This function can be used to create predicate objects from the standard library's
      * bind1st objects and from boost::bind too.
      *
      * Examples:
      *    - server_->send_if(boost::asio::buffer( str ), this, create_predicate(
      *                       boost::bind( std::not_equal_to<client_id>(), client, _1) ) );
      *
      * @param pred Predicate of the queried client.
      * @returns Predicate for client selection.
      */
    template<class predicate>
    detail::_generic_client_predicate<predicate> create_predicate(const predicate& pred)
    {
        return detail::_generic_client_predicate<predicate>(pred);
    }

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
        class client_proxy : public virtual detail::listener,
                             boost::noncopyable
        {
            public:
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

                /**
                 * Get the ID of this client.
                 *
                 * @returns : ID of the client represented by this proxy.
                 */
                virtual client_id id() const {return id_;}

                /** Standard destructor. */
                virtual ~client_proxy() {}

                // Allow server objects to invoke run_listener directly.
                using detail::listener::run_listener;

            protected:
                /** Standard constructor, sets the ID to the next available one. */
                client_proxy() : id_(++last_client_id_) {}

            private:
                static client_id    last_client_id_   /** Last issued client_id.  */ ;
                const client_id     id_               /** This proxy's client_id. */ ;
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

    /** @name Buffer creation methods
     *
     * @defgroup buffer
     *
     * API user should create buffers with one of these methods.
     *
     * Paraphrasing boost asio's documentation on the buffer function:
     * The ana::buffer function is used to create a buffer object to represent raw memory,
     * an array of POD elements, a vector of POD elements, or a std::string.
     *
     * Check <http://think-async.com/Asio/boost_asio_1_3_1/doc/html/boost_asio/reference/buffer.html>
     */
    //@{

    inline boost::asio::mutable_buffers_1 buffer(const boost::asio::mutable_buffer & b)
    {
        return boost::asio::buffer(b);
    }

    inline boost::asio::mutable_buffers_1 buffer(const boost::asio::mutable_buffer & b, std::size_t max_size_in_bytes)
    {
        return boost::asio::buffer(b, max_size_in_bytes);
    }

    inline boost::asio::const_buffers_1 buffer(const boost::asio::const_buffer & b)
    {
        return boost::asio::buffer(b);
    }

    inline boost::asio::const_buffers_1 buffer(const boost::asio::const_buffer & b, std::size_t max_size_in_bytes)
    {
        return boost::asio::buffer(b, max_size_in_bytes);
    }

    inline boost::asio::mutable_buffers_1 buffer(void * data, std::size_t size_in_bytes)
    {
        return boost::asio::buffer(data, size_in_bytes);
    }

    inline boost::asio::const_buffers_1 buffer(const void * data, std::size_t size_in_bytes)
    {
        return boost::asio::buffer(data, size_in_bytes);
    }

    template<typename PodType, std::size_t N>
    inline boost::asio::mutable_buffers_1 buffer(PodType & data)
    {
        return boost::asio::buffer(data);
    }

    template<typename PodType, std::size_t N>
    inline boost::asio::mutable_buffers_1 buffer(PodType & data, std::size_t max_size_in_bytes)
    {
        return boost::asio::buffer(data, max_size_in_bytes);
    }

    template<typename PodType, std::size_t N>
    inline boost::asio::const_buffers_1 buffer(const PodType & data)
    {
        return boost::asio::buffer(data);
    }

    template<typename PodType, std::size_t N>
    inline boost::asio::const_buffers_1 buffer(const PodType & data, std::size_t max_size_in_bytes)
    {
        return boost::asio::buffer(data, max_size_in_bytes);
    }

    template<typename PodType, std::size_t N>
    inline boost::asio::mutable_buffers_1 buffer(boost::array< PodType, N > & data)
    {
        return boost::asio::buffer(data);
    }

    template<typename PodType, std::size_t N> 
    inline boost::asio::mutable_buffers_1 buffer(boost::array< PodType, N > & data,
                                                 std::size_t max_size_in_bytes)
    {
        return boost::asio::buffer(data, max_size_in_bytes);
    }

    template<typename PodType, std::size_t N>
    inline boost::asio::const_buffers_1 buffer(boost::array< const PodType, N > & data)
    {
        return boost::asio::buffer(data);
    }

    template<typename PodType, std::size_t N>
    inline boost::asio::const_buffers_1 buffer(boost::array< const PodType, N > & data,
                                               std::size_t max_size_in_bytes)
    {
        return boost::asio::buffer(data, max_size_in_bytes);
    }

    template<typename PodType, std::size_t N>
    inline boost::asio::const_buffers_1 buffer(const boost::array< PodType, N > & data)
    {
        return boost::asio::buffer(data);
    }

    template<typename PodType, std::size_t N>
    inline boost::asio::const_buffers_1 buffer(const boost::array< PodType, N > & data,
                                               std::size_t max_size_in_bytes)
    {
        return boost::asio::buffer(data, max_size_in_bytes);
    }

    template<typename PodType, typename Allocator>
    inline boost::asio::mutable_buffers_1 buffer(std::vector< PodType, Allocator > & data)
    {
        return boost::asio::buffer(data);
    }

    template<typename PodType, typename Allocator>
    inline boost::asio::mutable_buffers_1 buffer(std::vector< PodType, Allocator > & data,
                                                 std::size_t max_size_in_bytes)
    {
        return boost::asio::buffer(data, max_size_in_bytes);
    }

    template<typename PodType, typename Allocator> 
    inline boost::asio::const_buffers_1 buffer(const std::vector< PodType, Allocator > & data)
    {
        return boost::asio::buffer(data);
    }

    template<typename PodType, typename Allocator>
    inline boost::asio::const_buffers_1 buffer(const std::vector< PodType, Allocator > & data,
                                                                          std::size_t max_size_in_bytes)
    {
        return boost::asio::buffer(data, max_size_in_bytes);
    }

    inline boost::asio::const_buffers_1 buffer(const std::string & data)
    {
        return boost::asio::buffer(data);
    }

    inline boost::asio::const_buffers_1 buffer(const std::string & data, std::size_t max_size_in_bytes)
    {
        return boost::asio::buffer(data, max_size_in_bytes);
    }
    //@}
}

#endif
