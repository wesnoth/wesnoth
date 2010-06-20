/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_SPIRIT_GET_CHAR_APR_20_2008_0618_PM)
#define BOOST_SPIRIT_GET_CHAR_APR_20_2008_0618_PM

#include <string>

namespace boost { namespace spirit { namespace qi { namespace detail
{
    // utility to get the (first) character from a primitive char,
    // a null terminated string and a std::basic_string

    template <typename Char>
    static Char get_char(Char ch)
    {
        return ch;
    }

    template <typename Char>
    static Char get_char(Char const* str)
    {
        return *str;
    }

    template <typename Char>
    static Char get_char(std::basic_string<Char> const& str)
    {
        return str[0];
    }
}}}}

#endif
