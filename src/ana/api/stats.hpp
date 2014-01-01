
/**
 * @file
 * @brief Implementation details for the ana project dealing with network statistics.
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

#ifndef ANA_STATS_HPP
#define ANA_STATS_HPP

#include "timers.hpp"

namespace ana
{
    /** Type of collected network statistics. */
    enum stat_type
    {
        /** Statistics accumulated since creation of the network component. */
        ACCUMULATED,

        /** Network statistics for the last second. */
        SECONDS,

        /** Network statistics for the last minute. */
        MINUTES,

        /** Network statistics for the last hour. */
        HOURS,

        /** Network statistics for the last day. */
        DAYS
    };

    /**
     * A network statistics object, describes statistics for a given time period.
     */
    struct stats
    {
        virtual ~stats() {}

        /** Returns the amount of seconds since creation. */
        virtual size_t uptime()       const = 0;

        /** Returns the amount of received packets for a particular collected statistics object. */
        virtual size_t packets_in()   const = 0;

        /** Returns the amount of sent packets for a particular collected statistics object. */
        virtual size_t packets_out()  const = 0;

        /** Returns the amount of received bytes for a particular collected statistics object. */
        virtual size_t bytes_in()     const = 0;

        /** Returns the amount of sent bytes for a particular collected statistics object. */
        virtual size_t bytes_out()    const = 0;
    };

    /// @cond false
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
                        timer_.async_wait(boost::bind( &stats_logger::reset, this,
                                                       boost::asio::placeholders::error ) );
                    }
                }

                virtual ~stats_logger() {}

                void log_send( detail::shared_buffer buffer )
                {
                    ++packets_out_;
                    bytes_out_ += buffer->size() + HEADER_LENGTH;
                }

                void log_send( size_t size, bool finished_packet = false )
                {
                    bytes_out_ += size;
                    if (finished_packet)
                        ++packets_out_;
                }

                void log_receive( read_buffer buffer )
                {
                    ++packets_in_;
                    bytes_in_ += buffer->size();
                }

                void log_receive( size_t size, bool finished_packet = false )
                {
                    bytes_in_ += size;
                    if (finished_packet)
                        ++packets_in_;
                }

            private:
                void reset(const boost::system::error_code& /*ec*/)
                {
                    packets_in_  = 0;
                    packets_out_ = 0;
                    bytes_in_    = 0;
                    bytes_out_   = 0;

                    if (secs_to_reset_ > 0 )
                    {
                        timer_.expires_from_now( secs_to_reset_ );
                        timer_.async_wait(boost::bind( &stats_logger::reset, this,
                                                       boost::asio::placeholders::error ) );
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
                days_stats_( time::days(1), io_service_ ),
                current_packet_in_size_(0),
                current_packet_out_size_(0),
                current_packet_in_(0),
                current_packet_out_(0),
                current_packet_in_max_(0),
                current_packet_out_max_(0),
                current_packet_in_total_(0),
                current_packet_out_total_(0)
            {
                collector_thread_ = new boost::thread( boost::bind(&boost::asio::io_service::run,
                                                                   &io_service_) );
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

            size_t current_packet_in_size()   const { return current_packet_in_size_;   }
            size_t current_packet_out_size()  const { return current_packet_out_size_;  }

            size_t current_packet_in_last()   const { return current_packet_in_;        }
            size_t current_packet_in_max()    const { return current_packet_in_max_;    }
            size_t current_packet_in_total()  const { return current_packet_in_total_;  }

            size_t current_packet_out_last()  const { return current_packet_out_;       }
            size_t current_packet_out_max()   const { return current_packet_out_max_;   }
            size_t current_packet_out_total() const { return current_packet_out_total_; }

            void log_send( detail::shared_buffer buffer )
            {
                accumulator_.log_send  ( buffer );
                seconds_stats_.log_send( buffer );
                minutes_stats_.log_send( buffer );
                hours_stats_.log_send  ( buffer );
                days_stats_.log_send   ( buffer );

                log_current_packet_out(buffer->size(), true);
            }

            void log_send( size_t size, bool finished_packet = false )
            {
                accumulator_.log_send  ( size, finished_packet );
                seconds_stats_.log_send( size, finished_packet );
                minutes_stats_.log_send( size, finished_packet );
                hours_stats_.log_send  ( size, finished_packet );
                days_stats_.log_send   ( size, finished_packet );

                log_current_packet_out(size, finished_packet);
            }


            void log_receive( read_buffer buffer )
            {
                accumulator_.log_receive  ( buffer );
                seconds_stats_.log_receive( buffer );
                minutes_stats_.log_receive( buffer );
                hours_stats_.log_receive  ( buffer );
                days_stats_.log_receive   ( buffer );

                log_current_packet_in(buffer->size(), true);
            }

            void log_receive( size_t size, bool finished_packet = false )
            {
                accumulator_.log_receive  ( size, finished_packet );
                seconds_stats_.log_receive( size, finished_packet );
                minutes_stats_.log_receive( size, finished_packet );
                hours_stats_.log_receive  ( size, finished_packet );
                days_stats_.log_receive   ( size, finished_packet );

                log_current_packet_in(size, finished_packet);
            }

            void start_send_packet( size_t size )    { current_packet_out_size_ = size; }
            void start_receive_packet( size_t size ) { current_packet_in_size_  = size; }

            ~stats_collector()
            {
                io_service_.stop();
                collector_thread_->join();
                delete collector_thread_;
            }

        private:
            void log_current_packet_in(size_t size,bool finished_packet)
            {
                if (finished_packet)
                {
                    current_packet_in_       = 0;
                    current_packet_in_max_   = 0;
                    current_packet_in_total_ = 0;
                }
                else
                {
                    current_packet_in_        = size;
                    current_packet_in_max_    = (std::max)(size, current_packet_in_max_);
                    current_packet_in_total_ += size;
                }
            }
            void log_current_packet_out(size_t size, bool finished_packet)
            {
                if (finished_packet)
                {
                    current_packet_out_       = 0;
                    current_packet_out_max_   = 0;
                    current_packet_out_total_ = 0;
                }
                else
                {
                    current_packet_out_        = size;
                    current_packet_out_max_    = (std::max)(size, current_packet_out_max_);
                    current_packet_out_total_ += size;
                }
            }


            boost::asio::io_service io_service_;

            boost::thread*       collector_thread_;

            detail::stats_logger accumulator_;
            detail::stats_logger seconds_stats_;
            detail::stats_logger minutes_stats_;
            detail::stats_logger hours_stats_;
            detail::stats_logger days_stats_;

            size_t               current_packet_in_size_;
            size_t               current_packet_out_size_;

            size_t               current_packet_in_;
            size_t               current_packet_out_;
            size_t               current_packet_in_max_;
            size_t               current_packet_out_max_;
            size_t               current_packet_in_total_;
            size_t               current_packet_out_total_;
    };
    /// @endcond
}

#endif
