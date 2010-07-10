/* $Id$ */

/**
 * @file stats.hpp
 * @brief Implementation details for the ana project dealing with network statistics.
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

#ifndef ANA_STATS_HPP
#define ANA_STATS_HPP

#include "timers.hpp"

#ifndef ANA_DETAIL_INTERNAL_HPP
#error "Private file, do not include directly."
#endif

namespace ana
{
    enum stat_type
    {
        ACCUMULATED, 
        SECONDS,
        MINUTES,
        HOURS,
        DAYS
    };

    struct stats
    {
        virtual size_t uptime()       const = 0;
        virtual size_t packets_in()   const = 0;
        virtual size_t packets_out()  const = 0;
        virtual size_t bytes_in()     const = 0;
        virtual size_t bytes_out()    const = 0;
    };

    namespace detail
    {
        class stats_logger : public stats
        {
            public:
                stats_logger(size_t ms_to_reset, boost::asio::io_service& io_service) :
                    started_at_( std::time(0) ),
                    secs_to_reset_(ms_to_reset / 1000.0),
                    timer_(io_service),
                    start_time_( 0 ),
                    packets_in_( 0 ),
                    packets_out_( 0 ),
                    bytes_in_( 0 ),
                    bytes_out_( 0 )
                {
                    if (secs_to_reset_ > 0 )
                    {
                        timer_.expires_from_now( secs_to_reset_ );
                        timer_.async_wait(boost::bind( &stats_logger::reset, this, boost::asio::placeholders::error ) );
                    }
                }

                void log_send( detail::shared_buffer buffer )
                {
                    ++packets_out_;
                    bytes_out_ += buffer->size() + HEADER_LENGTH;
                }

                void log_receive( detail::read_buffer buffer )
                {
                    ++packets_in_;
                    bytes_in_ += buffer->size();
                }

                void log_receive( size_t size )
                {
                    bytes_in_ += size;
                }

            private:
                void reset(boost::system::error_code& /*ec*/)
                {
                    packets_in_  = 0;
                    packets_out_ = 0;
                    bytes_in_    = 0;
                    bytes_out_   = 0;

                    if (secs_to_reset_ > 0 )
                    {
                        timer_.expires_from_now( secs_to_reset_ );
                        timer_.async_wait(boost::bind( &stats_logger::reset, this, boost::asio::placeholders::error ) );
                    }
                }

                virtual size_t uptime()       const
                {
                    return std::time(0) - started_at_;
                }

                virtual size_t packets_in()   const
                {
                    return packets_in_;
                }

                virtual size_t packets_out()  const
                {
                    return packets_out_;
                }

                virtual size_t bytes_in()     const
                {
                    return bytes_in_;
                }

                virtual size_t bytes_out()    const
                {
                    return bytes_out_;
                }


                std::time_t  started_at_;

                double       secs_to_reset_;
                boost_timer  timer_;

                std::time_t start_time_;

                size_t packets_in_;
                size_t packets_out_;

                size_t bytes_in_;
                size_t bytes_out_;
        };
    }

    class stats_collector
    {
        public:
            stats_collector() :
                io_service_(),
                collector_thread_( NULL ),
                accumulator_( 0, io_service_),
                seconds_stats_( time::seconds(1), io_service_ ),
                minutes_stats_( time::minutes(1), io_service_ ),
                hours_stats_( time::hours(1), io_service_ ),
                days_stats_( time::days(1), io_service_ )
            {
                collector_thread_ = new boost::thread( boost::bind(&boost::asio::io_service::run, &io_service_) );
            }

            const stats* get_stats( stat_type type ) const
            {
                switch (type)
                {
                    case ACCUMULATED : return &accumulator_;
                    case SECONDS     : return &seconds_stats_;
                    case MINUTES     : return &minutes_stats_;
                    case HOURS       : return &hours_stats_;
                    case DAYS        : return &days_stats_;
                }
                throw std::runtime_error("Wrong stat stype requested.");
            }

            void log_send( detail::shared_buffer buffer )
            {
                accumulator_.log_send  ( buffer );
                seconds_stats_.log_send( buffer );
                minutes_stats_.log_send( buffer );
                hours_stats_.log_send  ( buffer );
                days_stats_.log_send   ( buffer );
            }

            void log_receive( detail::read_buffer buffer )
            {
                accumulator_.log_receive  ( buffer );
                seconds_stats_.log_receive( buffer );
                minutes_stats_.log_receive( buffer );
                hours_stats_.log_receive  ( buffer );
                days_stats_.log_receive   ( buffer );
            }

            ~stats_collector()
            {
                io_service_.stop();
                collector_thread_->join();
                delete collector_thread_;
            }

        private:
            boost::asio::io_service io_service_;

            boost::thread*       collector_thread_;

            detail::stats_logger accumulator_;
            detail::stats_logger seconds_stats_;
            detail::stats_logger minutes_stats_;
            detail::stats_logger hours_stats_;
            detail::stats_logger days_stats_;
    };
}

#endif