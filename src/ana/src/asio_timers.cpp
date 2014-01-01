
/**
 * @file
 * @brief Implementation details for the ana project dealing with timers.
 *
 * ana: Asynchronous Network API.
 * Copyright (C) 2010 - 2014 Guillermo Biset.
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

#include "../api/ana.hpp"
#include <boost/bind.hpp>
#include <boost/thread.hpp>

namespace ana
{
    namespace detail
    {
        // Get the current time.
        time_t_traits::time_type time_t_traits::now()
        {
            return std::time(0);
        }

        // Add a duration to a time.
        time_t_traits::time_type time_t_traits::add(const time_type& t, const duration_type& d)
        {
            return t + d.value;
        }

        // Subtract one time from another.
        time_t_traits::duration_type time_t_traits::subtract(const time_type& t1,
                                                             const time_type& t2)
        {
            return duration_type(t1 - t2);
        }

        // Test whether one time is less than another.
        bool time_t_traits::less_than(const time_t_traits::time_type& t1,
                                      const time_t_traits::time_type& t2)
        {
            return t1 < t2;
        }

        // Convert to POSIX duration type.
        boost::posix_time::time_duration time_t_traits::to_posix_duration(const duration_type& d)
        {
            return boost::posix_time::seconds(d.value);
        }
    }


    timer::timer() :
        holds_fresh_io_service_( true ),
        io_service_(new boost::asio::io_service() ),
        timer_thread_( NULL ),
        timer_(*io_service_)
    {
    }

    timer::timer( boost::asio::io_service& io ) :
        holds_fresh_io_service_( false ),
        io_service_( &io ),
        timer_thread_( NULL ),
        timer_(*io_service_)
    {
    }

    timer::~timer()
    {
        if ( holds_fresh_io_service_ )
        {
            io_service_->stop();
            timer_thread_->join();
            delete timer_thread_;
        }
    }

    void timer::run()
    {
        if ( holds_fresh_io_service_ )
            io_service_->run_one();
    }

    namespace detail
    {
        void timed_sender::set_timeouts(timeout_policy type, size_t ms)
        {
            timeout_milliseconds_ = ms;
            if (ms > 0)
                timeout_type_     = type;
            else
                timeout_type_     = NoTimeouts;
        }

        bool timed_sender::timeouts_enabled() const
        {
            return (timeout_milliseconds_ != 0) && (timeout_type_ != NoTimeouts);
        }

        timed_sender::timed_sender() :
            timeout_type_( NoTimeouts ),
            timeout_milliseconds_( 0 )
        {
        }
    }
}

