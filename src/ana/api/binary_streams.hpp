
/**
 * @file
 * @brief Minimal serialization library.
 *
 * This file is part of the MiLi Minimalistic Library :
 *      mili.googlecode.com
 *
 * binary_streams: A minimal library supporting encoding of different data
 *                 types in a single binary stream.
 *
 * Copyright (C) 2009, 2010  Guillermo Biset
 *
 * Subsystem: ana: Asynchronous Network API.
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

#ifndef BINARY_STREAMS_HPP
#define BINARY_STREAMS_HPP

#include <list>
#include <map>
#include <set>
#include <vector>

#include "common.hpp"

namespace ana
{
    /**
     * Module for object serialization and Marshaling.
     */
    namespace serializer
    {
        /// @cond false
        template <class T>
        struct template_is_container { enum {  value = 0   }; };

        // This could/should be done with Template Meta Programming
        template <class T>
        struct template_is_container< std::vector<T> > { enum {  value = 1 }; };

        template <class K, class D>
        struct template_is_container< std::vector<K,D> > { enum {  value = 1 }; };

        template <class T>
        struct template_is_container< std::list<T> > { enum {  value = 1 }; };

        template <class K, class D>
        struct template_is_container< std::list<K,D> > { enum {  value = 1 }; };

        template <class K, class D, class C>
        struct template_is_container< std::set<K,D,C> > { enum {  value = 1 }; };

        template <class K, class D>
        struct template_is_container< std::map<K,D> > { enum {  value = 1 }; };

        template <class K, class D>
        struct template_is_container< std::multimap<K,D> > { enum {  value = 1 }; };
        /// @endcond

        /**
         * Output stream serialization. This class provides stream functionality to serialize
         * objects in a way similar to that of std::cout or std::ostringstream.
         *
         * Extracted from MiLi (mili.googlecode.com).
         *
         * An extensive example of its usage can be seen in:
         *    http://code.google.com/p/mili/source/browse/trunk/example_binary-streams.cpp
         */
        class bostream
        {
            private:
                template<class T, bool IsContainer> struct _inserter_helper;

                template<class T, bool IsContainer> friend struct _inserter_helper;

            public:
                /**
                 * Standard constructor.
                 */
                bostream() :
                    _s()
                {
                }

                /**
                 * Insert any object to the stream.
                 *
                 * @param x : A copy of the object being inserted.
                 */
                template <class T>
                bostream& operator<< (T x)
                {
                    _inserter_helper<T, template_is_container<T>::value >::call(this, x);
                    return *this;
                }

                /**
                 * Insert a string to the stream.
                 */
                bostream& operator<< (const std::string& s)
                {
                    (*this) << ana::ana_uint32( s.size() );
                    _s += s;
                    return *this;
                }

                /**
                 * Insert a vector of elements.
                 */
                template <class Other>
                bostream& operator<< (const std::vector<Other>& vec)
                {
                    const ana::ana_uint32 size(vec.size());
                    (*this) << size;
                    for (size_t i(0); i < size; ++i)
                        (*this) << vec[i];

                    return *this;
                }

                /** Insert a literal string. */
                bostream& operator<< (const char* cs)
                {
                    const std::string s(cs);
                    return operator<< (s);
                }

                /** Obtain the string representing the stream. */
                const std::string& str() const
                {
                    return _s;
                }

                /** Clear the stream. Enables the user to use it several times. */
                void clear()
                {
                    _s.clear();
                }

            private:

                /** The representation of the stream in memory. */
                std::string _s;
        };

        /**
         * Input stream serialization. This class provides stream functionality to serialize
         * objects in a way similar to that of std::cin or std::istringstream.
         *
         * Extracted from MiLi (mili.googlecode.com).
         *
         * An extensive example of its usage can be seen in:
         *    http://code.google.com/p/mili/source/browse/trunk/example_binary-streams.cpp
         */
        class bistream
        {
            private:
                template<class T> friend class container_reader;

            public:
                /**
                 * Construct a new input stream object using a string representing a binary stream
                 * as input.
                 */
                bistream(const std::string& str) :
                    _s(str),
                    _pos(0)
                {
                }

                /**
                 * Creates a new input stream object, but with no data.
                 */
                bistream() :
                    _s(),
                    _pos(0)
                {
                }

                /**
                 * Set the representation binary string.
                 *
                 * @param str : The new binary stream representation string.
                 */
                void str(const std::string& str)
                {
                    _pos = 0;
                    _s = str;
                }

                /**
                 * Read an element.
                 *
                 * @param x : A reference to the element you are reading into.
                 *
                 * @pre : The remaining input stream holds enough data to read the element.
                 */
                template <class T>
                bistream& operator >> (T& x)
                {
                    assert(_s.size() >= _pos + sizeof(x));
                    _pos += _s.copy(reinterpret_cast<char*>(&x), sizeof(x),_pos);
                    return *this;
                }

                /**
                 * Read a string.
                 *
                 * @param str : A reference to the string you are reading into.
                 *
                 * @pre : The stream is situated in a position holding a 32 bit unsigned integer
                 *        and then at least this very number of bytes remain.
                 */
                bistream& operator >> (std::string& str)
                {
                    ana::ana_uint32 size;
                    (*this) >> size;
                    assert(_s.size() >= size+_pos);
                    str  = _s.substr(_pos,size);
                    _pos += size;
                    return *this;
                }

                /**
                 * Read a vector.
                 *
                 * @param vec : A reference to the vector you are reading into.
                 *
                 * @pre : The stream is situated in a position holding a 32 bit unsigned integer
                 *        and then at least this very number of elements remain.
                 */
                template <class Other>
                bistream& operator>> (std::vector<Other>& vec)
                {
                    ana::ana_uint32 size;
                    (*this) >> size;
                    assert(_s.size() >= (size * sizeof(Other)) + _pos);
                    vec.resize(size);
                    for (size_t i(0); i < size; i++)
                        (*this) >> vec[i];

                    return *this;
                }

                /** Clear the input stream. */
                void clear()
                {
                    _s.clear();
                    _pos = 0;
                }

            private:

                /** The string representing the input stream. */
                std::string _s;

                /** The position the stream is reading from.  */
                std::size_t _pos;
        };

        /**
         * A helper class to insert containers from single elements. Use this when you know that
         * the data will later be read into a vector or some other container but you don't have
         * such a container for insertion, you want to create it on the go.
         *
         * @param T : The type of the elements in the container.
         */
        template<class T>
        class container_writer
        {
            public:

                /**
                 * Default constructor.
                 *
                 * @param size : The amount of elements you will write.
                 * @param bos : A reference to the output stream where you will create the
                 *              container.
                 */
                container_writer( size_t size, bostream& bos) :
                    _elements_left( size ),
                    _bos( bos )
                {
                    _bos << ana::ana_uint32( size );
                }

                /**
                 * Push an element.
                 */
                container_writer& operator<<(T element)
                {
                    assert( _elements_left > 0 );
                    --_elements_left;

                    _bos << element;

                    return *this;
                }

                /**
                 * Default destructor.
                 *
                 * @pre : The elements inserted equals the amount of elements that were promised
                 *        to be inserted at the time of creation (size parameter.)
                 */
                ~container_writer()
                {
                    if ( _elements_left != 0 )
                        assert(false && "More elements were expected to be written.");
                }
            private:

                /** The amount of elements you have yet to insert. */
                size_t    _elements_left;

                /** A reference to the output stream. */
                bostream& _bos;
        };

        /**
         * A helper class to read from containers one by one. Use this when you want to read
         * something inserted as a container one by one, you can also use it to know how many
         * elements were inserted.
         *
         * @param T : The type of the elements in the container.
         */
        template<class T>
        class container_reader
        {
            public:
                /**
                 * Standard constructor.
                 *
                 * @param bis : The input stream holding the data.
                 */
                container_reader( bistream& bis) :
                    _elements_left( 0 ),
                    _bis( bis )
                {
                    _bis >> _elements_left;
                }

                /**
                 * Read an element.
                 */
                container_reader& operator>>(T& element)
                {
                    assert( _elements_left > 0 );
                    --_elements_left;

                    _bis >> element;

                    return *this;
                }

                /**
                 * Skip a given amount of elements, default: 1.
                 *
                 * Examples:
                 *    - skip();
                 *    - skip(10);
                 *
                 * @param elements : The amount of elements you want to skip. Default: 1.
                 *
                 * @pre : At least the amount of elements you want to skip remain.
                 */
                void skip(size_t elements = 1)
                {
                    if ( elements > _elements_left )
                        throw("Trying to skip too much.");

                    _elements_left -= elements;

                    _bis._pos += sizeof(T) * elements;

                    if ( _bis._pos > _bis._s.size() )
                        throw("Too much mas skipped.");
                }

                /**
                 * Signal that you have finished reading. It is the same as skipping the amount
                 * of elements left.
                 */
                void finished()
                {
                    skip( _elements_left );
                    _elements_left = 0;
                }

                /**
                 * Returns the amount of elements that still haven't been read from the container.
                 */
                size_t elements_left() const
                {
                    return _elements_left;
                }

                /**
                 * Standard destructor. Finishes the reading process if necessary.
                 */
                ~container_reader()
                {
                    if ( _elements_left != 0 )
                        finished();
                }

            private:

                /** The amount of elements that still haven't been read from the container. */
                size_t    _elements_left;

                /** A reference to the input stream. */
                bistream& _bis;
        };

        /// @cond false
        template<class T>
        struct bostream::_inserter_helper<T, false>
        {
            static void call(bostream* bos, const T& x)
            {
                bos->_s.append(reinterpret_cast<const char*>(&x), sizeof(T));
            }
        };

        template<class T>
        struct bostream::_inserter_helper<T, true>
        {
            static void call(bostream* bos, const T& cont)
            {
                const ana::ana_uint32 size(cont.size());
                (*bos) << size;

                typename T::const_iterator it( cont.begin() );

                for (; it != cont.end(); ++it)
                    (*bos) << *it;
            }
        };
        /// @endcond

    } //serializer namespace
} //ana namespace

#endif
