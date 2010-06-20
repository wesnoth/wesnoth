//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_KARMA_STRING_GENERATE_FEB_23_2007_1232PM)
#define BOOST_SPIRIT_KARMA_STRING_GENERATE_FEB_23_2007_1232PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <string>
#include <boost/spirit/home/karma/detail/generate_to.hpp>
#include <boost/spirit/home/support/char_class.hpp>

namespace boost { namespace spirit { namespace karma { namespace detail
{
    ///////////////////////////////////////////////////////////////////////////
    //  generate a string given by a pointer 
    template <typename OutputIterator, typename Char>
    inline bool 
    string_generate(OutputIterator& sink, Char const* str, unused_type = unused)
    {
        for (Char ch = *str; ch != 0; ch = *++str)
            detail::generate_to(sink, ch);
        return true;
    }

    ///////////////////////////////////////////////////////////////////////////
    //  generate a string given by a std::string
    template <typename OutputIterator, typename Char>
    inline bool 
    string_generate(OutputIterator& sink, std::basic_string<Char> const& str,
        unused_type = unused)
    {
        typedef std::basic_string<Char> string_type;
        
        typename string_type::const_iterator end = str.end();
        for (typename string_type::const_iterator it = str.begin(); 
             it != end; ++it)
        {
            detail::generate_to(sink, *it);
        }
        return true;
    }

    ///////////////////////////////////////////////////////////////////////////
    //  generate a string given by a pointer, converting according using a 
    //  given character class tag
    template <typename OutputIterator, typename Char, typename Tag>
    inline bool 
    string_generate(OutputIterator& sink, Char const* str, Tag tag)
    {
        for (Char ch = *str; ch != 0; ch = *++str)
            detail::generate_to(sink, ch, tag);
        return true;
    }

    ///////////////////////////////////////////////////////////////////////////
    //  generate a string given by a std::string, converting according using a 
    //  given character class tag
    template <typename OutputIterator, typename Char, typename Tag>
    inline bool 
    string_generate(OutputIterator& sink, std::basic_string<Char> const& str, 
        Tag tag)
    {
        typedef std::basic_string<Char> string_type;
        
        typename string_type::const_iterator end = str.end();
        for (typename string_type::const_iterator it = str.begin(); 
             it != end; ++it)
        {
            detail::generate_to(sink, *it, tag);
        }
        return true;
    }

}}}}

#endif
