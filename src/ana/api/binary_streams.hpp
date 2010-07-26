/* $Id$ */

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

#ifndef BINARY_STREAMS_H
#define BINARY_STREAMS_H

#include <string>
#include <assert.h>
#include <stdint.h>

namespace ana
{
    namespace serializer
    {
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

        class bostream
        {
            private:
                template<class T, bool IsContainer> struct _inserter_helper;

                template<class T, bool IsContainer> friend struct _inserter_helper;

            public:
                bostream() :
                    _s()
                {
                }

                template <class T>
                bostream& operator<< (T x)
                {
                    _inserter_helper<T, template_is_container<T>::value >::call(this, x);
                    return *this;
                }

                /* Inserting a string inserts its size first. */
                bostream& operator<< (const std::string& s)
                {
                    (*this) << uint32_t( s.size() );
                    _s += s;
                    return *this;
                }

                template <class Other>
                bostream& operator<< (const std::vector<Other>& vec)
                {
                    const uint32_t size(vec.size());
                    (*this) << size;
                    for (size_t i(0); i < size; ++i)
                        (*this) << vec[i];

                    return *this;
                }

                bostream& operator<< (const char* cs)
                {
                    const std::string s(cs);
                    return operator<< (s);
                }

                const std::string& str() const
                {
                    return _s;
                }

                void clear()
                {
                    _s.clear();
                }

            private:
                std::string _s;
        };

        class bistream
        {
            private:
                template<class T> friend class container_reader;

            public:
                bistream(const std::string& str) :
                    _s(str),
                    _pos(0)
                {
                }

                bistream() :
                    _s(),
                    _pos(0)
                {
                }

                void str(const std::string& str)
                {
                    _pos = 0;
                    _s = str;
                }

                template <class T>
                bistream& operator >> (T& x)
                {
                    assert(_s.size() >= _pos + sizeof(x));
                    _pos += _s.copy(reinterpret_cast<char*>(&x), sizeof(x),_pos);
                    return *this;
                }

                bistream& operator >> (std::string& str)
                {
                    uint32_t size;
                    (*this) >> size;
                    assert(_s.size() >= size+_pos);
                    str  = _s.substr(_pos,size);
                    _pos += size;
                    return *this;
                }

                template <class Other>
                bistream& operator>> (std::vector<Other>& vec)
                {
                    uint32_t size;
                    (*this) >> size;
                    assert(_s.size() >= (size * sizeof(Other)) + _pos);
                    vec.resize(size);
                    for (size_t i(0); i < size; i++)
                        (*this) >> vec[i];

                    return *this;
                }

                void clear()
                {
                    _s.clear();
                    _pos = 0;
                }

            private:
                std::string _s;
                std::size_t _pos;
        };

        template<class T>
        class container_writer
        {
            public:
                container_writer( size_t size, bostream& bos) :
                    _elements_left( size ),
                    _bos( bos )
                {
                    _bos << uint32_t( size );
                }

                container_writer& operator<<(T element)
                {
                    assert( _elements_left > 0 );
                    --_elements_left;

                    _bos << element;

                    return *this;
                }

                ~container_writer()
                {
                    if ( _elements_left != 0 )
                        throw/* std::runtime_error*/("More elements were expected to be written.");
                }
            private:
                size_t    _elements_left;
                bostream& _bos;
        };

        template<class T>
        class container_reader
        {
            public:
                container_reader( bistream& bis) :
                    _elements_left( 0 ),
                    _bis( bis )
                {
                    _bis >> _elements_left;
                }

                container_reader& operator>>(T& element)
                {
                    assert( _elements_left > 0 );
                    --_elements_left;

                    _bis >> element;

                    return *this;
                }

                void skip(size_t elements = 1)
                {
                    if ( elements > _elements_left )
                        throw("Trying to skip too much.");

                    _elements_left -= elements;

                    _bis._pos += sizeof(T) * elements;

                    if ( _bis._pos > _bis._s.size() )
                        throw("Too much mas skipped.");
                }

                void finished()
                {
                    skip( _elements_left );
                    _elements_left = 0;
                }

                ~container_reader()
                {
                    if ( _elements_left != 0 )
                        finished();
                }

            private:
                size_t    _elements_left;
                bistream& _bis;
        };

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
                const uint32_t size(cont.size());
                (*bos) << size;

                typename T::const_iterator it( cont.begin() );

                for (; it != cont.end(); ++it)
                    (*bos) << *it;
            }
        };
    } //serializer namespace
} //ana namespace

#endif
