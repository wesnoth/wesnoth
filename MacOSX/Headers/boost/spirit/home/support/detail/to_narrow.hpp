/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman
    Copyright (c) 2001-2008 Hartmut Kaiser

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_SPIRIT_TO_NARROW_APRIL_29_2007_1122AM)
#define BOOST_SPIRIT_TO_NARROW_APRIL_29_2007_1122AM

#include <string>
#include <locale>
#include <memory>

namespace boost { namespace spirit { namespace detail
{
    ///////////////////////////////////////////////////////////////////////////
    template <typename Char>
    inline char to_narrow_char(Char ch)
    {
        typedef std::ctype<Char> ctype_type;
        return std::use_facet<ctype_type>(std::locale()).narrow(ch, '.');
    }

    inline char to_narrow_char(char ch)
    {
        return ch;
    }

    template <typename Char>
    inline std::size_t getlength(Char const* p)
    {
        std::size_t len = 0;
        while (*p)
            ++len, ++p;
        return len;
    }

    template <typename Char>
    inline std::string to_narrow_string(Char const* source)
    {
        typedef std::ctype<Char> ctype_type;

        std::size_t len = getlength(source);
        std::auto_ptr<char> buffer(new char [len+1]);
        std::use_facet<ctype_type>(std::locale())
            .narrow(source, source + len, '.', buffer.get());

        return std::string(buffer.get(), len);
    }

    inline std::string to_narrow_string(char const* source)
    {
        return source;
    }

    template <typename Char>
    inline std::string to_narrow_string(std::basic_string<Char> const& str)
    {
        return to_narrow_string(str.c_str());
    }

    inline std::string const& to_narrow_string(std::string const& str)
    {
        return str;
    }

}}}

#endif
