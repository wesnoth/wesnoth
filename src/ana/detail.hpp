/* $Id$ */

/**
 * @file detail.hpp
 * @brief Implementation details for the ana project. Private file, do not include directly.
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

#ifndef DETAIL_INTERNAL_HPP
#error "Private file, do not include directly."
#endif

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
