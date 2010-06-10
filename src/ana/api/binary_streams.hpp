/* $Id$ */

/**
 * @file binary_streams.hpp
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
        class bostream
        {
            public:
                bostream() :
                    _s()
                {
                }

                template <class T>
                bostream& operator<< (T x)
                {
                    _s.append(reinterpret_cast<char*>(&x), sizeof(T));
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
    } //serializer namespace
} //ana namespace

#endif
