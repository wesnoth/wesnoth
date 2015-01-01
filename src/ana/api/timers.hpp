
/**
 * @file
 * @brief Implementation details for the ana project dealing with timers.
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

#ifndef ANA_TIMERS_HPP
#define ANA_TIMERS_HPP

#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include "buffers.hpp"

namespace ana
{
    /** @name Time duration functions. */
    //@{
    /**
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
         * @param s : Seconds of elapsed time.
         *
         * @returns : A time duration amount (in milliseconds) to be used with timers.
         */
        inline size_t seconds(double s)       { return size_t(s * 1000);}

        /**
         * Create a time lapse from a given amount of minutes.
         *
         * @param m : Minutes of elapsed time.
         *
         * @returns : A time duration amount (in milliseconds) to be used with timers.
         */
        inline size_t minutes(double m)       { return seconds(m * 60); }

        /**
         * Create a time lapse from a given amount of hours.
         *
         * @param h : Hours of elapsed time.
         *
         * @returns : A time duration amount (in milliseconds) to be used with timers.
         */
        inline size_t hours(double h)         { return minutes(h * 60); }

        /**
         * Create a time lapse from a given amount of days.
         *
         * @param d : Hours of elapsed time.
         *
         * @returns : A time duration amount (in milliseconds) to be used with timers.
         */
        inline size_t days(double d)         { return hours(d * 24); }
    }
    //@}

    /**
     * Timeout policies for send operations.
     *
     * \sa timer
     */
    enum timeout_policy
    {
        /** Don't use timers in any operation.                */
        NoTimeouts,

        /** Use timers with a fixed time for every operation. */
        FixedTime,

        /**
         * Use timers, calculating the necessary time from
         * the size of the buffer that is to be sent.
         */
        TimePerKilobyte
    };

    /** @name Timers
     *
     * Definitions of timer related types.
     */
    //@{

    /// @cond false
    namespace detail
    {
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
            static time_type now();

            // Add a duration to a time.
            static time_type add(const time_type& t, const duration_type& d);

            // Subtract one time from another.
            static duration_type subtract(const time_type& t1, const time_type& t2);

            // Test whether one time is less than another.
            static bool less_than(const time_type& t1, const time_type& t2);

            // Convert to POSIX duration type.
            static boost::posix_time::time_duration to_posix_duration(const duration_type& d);
        };
    }
    /// @endcond

    /** A boost::asio::basic_deadline_timer, can be used externally. */
    typedef boost::asio::basic_deadline_timer<std::time_t,detail::time_t_traits> boost_timer;

    /**
     * General purpose asynchronous timer.
     */
    class timer
    {
        public:
            /**
             * Standard constructor.
             * Creates its own io_service object and runs in its own thread.
             */
            timer();

            /**
             * Construct a timer object using a running io_service object.
             * The timer insures it won't stop the io_service.
             *
             * @pre The io_service will be running during the timer's lifetime.
             */
            timer( boost::asio::io_service& io );

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
                //conversion will use a double or float
                timer_.expires_from_now( milliseconds / 1000.0);
                timer_.async_wait(handler);

                if ( holds_fresh_io_service_ )
                    timer_thread_ = new boost::thread( boost::bind( &timer::run, this ) );
            }

            /** Standard destructor, cancels pending operations if handler wasn't called. */
            ~timer();

        private:
            void run();

            const bool holds_fresh_io_service_;

            boost::asio::io_service* io_service_;

            boost::thread* timer_thread_;

            boost_timer timer_;
    };

    namespace detail
    {
        /**
         * A network sender component that can be configured to issue timeout events.
         */
        class timed_sender
        {
            public:

				virtual ~timed_sender() {}

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
                void set_timeouts(timeout_policy type, size_t ms = 0);

                /**
                 * Start a timer given the current configuration.
                 *
                 * @param buffer : The buffer used in the send operation.
                 * @param handler : The handler of the timeout/abort event.
                 *
                 * @returns : A pointer to a newly created timer object.
                 */
                template<class Handler>
                void start_timer( timer* timer, shared_buffer buffer, Handler handler ) const
                {
                    if ( ! timeouts_enabled() )
                        delete timer;
                    else
                    {
                        if ( timer != NULL )
                        {
                            if ( timeout_type_ == TimePerKilobyte)
                                timer->wait( (buffer->size() / 1024.0) * timeout_milliseconds_,
                                             handler);
                            else
                                timer->wait( timeout_milliseconds_, handler);
                        }
                    }
                }

                bool timeouts_enabled() const;

                /**
                 * Creates a timer using the current io_service of this component.
                 * For efficiency reasons, always create timers this way when you have the chance.
                 */
                virtual timer* create_timer() = 0;

            protected:
                /** Standard constructor. */
                timed_sender();

                /** Type of timer policy.                 */
                timeout_policy timeout_type_         ;

                /** Amount of ms relevant to this policy. */
                size_t         timeout_milliseconds_ ;
        };
    }
}

#endif
