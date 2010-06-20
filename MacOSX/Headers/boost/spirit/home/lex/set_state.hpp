//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_LEX_SET_STATE_FEB_13_2008_0719PM)
#define BOOST_SPIRIT_LEX_SET_STATE_FEB_13_2008_0719PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/xpressive/proto/proto.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace boost { namespace spirit 
{
    namespace tag
    {
        ///////////////////////////////////////////////////////////////////////
        // This is the tag returned by the set_state function
        template <typename String>
        struct set_state_tag 
        {
            String name;
        };
    }
        
    ///////////////////////////////////////////////////////////////////////////
    // These are the different overloads allowed for the set_state(...) 
    // construct, which is used by qi and by lex for lexer state switching 
    // from inside a (parser or lexer) semantic action
    ///////////////////////////////////////////////////////////////////////////
    inline proto::terminal<tag::set_state_tag<char const*> >::type
    set_state(char const *s)
    {
        proto::terminal<tag::set_state_tag<char const*> >::type that = {{s}};
        return that;
    }
    
    inline proto::terminal<tag::set_state_tag<wchar_t const*> >::type
    set_state(wchar_t const *s)
    {
        proto::terminal<tag::set_state_tag<wchar_t const*> >::type that = {{s}};
        return that;
    }
    
    template <typename Char, typename Traits, typename Allocator>
    inline proto::terminal<tag::set_state_tag<char const*> >::type
    set_state(std::basic_string<Char, Traits, Allocator> const& s)
    {
        typename proto::terminal<tag::set_state_tag<Char const*> >::type that = 
            {{s.c_str()}};
        return that;
    }

///////////////////////////////////////////////////////////////////////////////
}}

#endif
