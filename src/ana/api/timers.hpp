/* $Id$ */

/**
 * @file timers.hpp
 * @brief Implementation details for the ana project dealing with timers.
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

#include "buffers.hpp"

#ifndef ANA_TIMERS_HPP
#define ANA_TIMERS_HPP

#ifndef ANA_DETAIL_INTERNAL_HPP
#error "Private file, do not include directly."
#endif

namespace ana
{
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

    namespace detail
    {
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
    }
}

#endif