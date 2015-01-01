
/**
 * @file
 * @brief Implementation details for the ana project dealing with buffers.
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

#ifndef ANA_BUFFERS_HPP
#define ANA_BUFFERS_HPP

#include <cstring>
#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>

#include "common.hpp"

namespace ana
{
    namespace detail
    {
        /**
         * A simple buffer to read things on, supports conversion to string.
         */
        class read_buffer_implementation : boost::noncopyable
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

                /**
                 * Resize the buffer, this won't affect the amount of memory usage.
                 * It is only possible to downsize the buffer.
                 *
                 * @pre : The new size is smaller than the original size used to construct this.
                 */
                void resize( size_t new_size )
                {
                    if ( new_size > size_ )
                        throw std::runtime_error("Can't expand a read_buffer.");

                    size_ = new_size;
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

                template<class T>
                void copy_to(T& x)
                {
                    assert( size_ == sizeof(T) );
                    std::memcpy( reinterpret_cast<void*>(&x), base(), sizeof(T) );
                }

            private:
                char*  base_;
                size_t size_;
        };

        /**
         * A buffer to be constructed from different buffers that can duplicate the other
         * and hold a local copy that will destruct with the object itself.
         */
        class copying_buffer : boost::noncopyable
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

                inline char*  base_char() const { return reinterpret_cast<char*>(base_); }

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

    } //namespace detail

    /** A shared pointer to a read_buffer, self destructs when no one is referencing it. */
    typedef boost::shared_ptr<detail::read_buffer_implementation> read_buffer;


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
     * Check:
     * <http://think-async.com/Asio/boost_asio_1_3_1/doc/html/boost_asio/reference/buffer.html>
     */
    //@{

    inline boost::asio::mutable_buffers_1 buffer(const boost::asio::mutable_buffer & b)
    {
        return boost::asio::buffer(b);
    }

    inline boost::asio::mutable_buffers_1 buffer(const boost::asio::mutable_buffer & b,
                                                 std::size_t max_size_in_bytes)
    {
        return boost::asio::buffer(b, max_size_in_bytes);
    }

    inline boost::asio::const_buffers_1 buffer(const boost::asio::const_buffer & b)
    {
        return boost::asio::buffer(b);
    }

    inline boost::asio::const_buffers_1 buffer(const boost::asio::const_buffer & b,
                                               std::size_t max_size_in_bytes)
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

    inline boost::asio::const_buffers_1 buffer(const std::string & data,
                                               std::size_t max_size_in_bytes)
    {
        return boost::asio::buffer(data, max_size_in_bytes);
    }
    //@}
}

#endif

