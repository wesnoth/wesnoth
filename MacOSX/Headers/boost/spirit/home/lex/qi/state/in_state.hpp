//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_LEX_IN_STATE_OCT_09_2007_0748PM)
#define BOOST_SPIRIT_LEX_IN_STATE_OCT_09_2007_0748PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/xpressive/proto/proto.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace boost { namespace spirit { namespace qi
{
    ///////////////////////////////////////////////////////////////////////////
    // This is the tag returned by the in_state function
    template <typename String>
    struct in_state_tag 
    {
        String name;
    };

    ///////////////////////////////////////////////////////////////////////////
    // These are the different overloads allowed for the in_state(...) 
    // construct
    ///////////////////////////////////////////////////////////////////////////
    inline proto::terminal<in_state_tag<char const*> >::type
    in_state(char const *s)
    {
        proto::terminal<in_state_tag<char const*> >::type that = {{s}};
        return that;
    }

    inline proto::terminal<in_state_tag<wchar_t const*> >::type
    in_state(wchar_t const *s)
    {
        proto::terminal<in_state_tag<wchar_t const*> >::type that = {{s}};
        return that;
    }

    template <typename Char, typename Traits, typename Allocator>
    inline typename proto::terminal<in_state_tag<Char const*> >::type
    in_state(std::basic_string<Char, Traits, Allocator> const& s)
    {
        typedef std::basic_string<Char, Traits, Allocator> string_type;

        typename proto::terminal<in_state_tag<string_type> >::type that;
        that.s = s;

        return that;
    }

    ///////////////////////////////////////////////////////////////////////////
    // The following is a helper template allowing to use the in_state()[] as 
    // a skip parser
    ///////////////////////////////////////////////////////////////////////////
    template <typename Skipper, typename String = char const*>
    struct in_state_skipper;

    template <typename Skipper>
    struct in_state_skipper<Skipper, char const*>
      : proto::subscript<
            typename proto::terminal<in_state_tag<char const*> >::type,
            Skipper
        >::type
    {};

    template <typename Skipper>
    struct in_state_skipper<Skipper, wchar_t const*>
      : proto::subscript<
            typename proto::terminal<in_state_tag<wchar_t const*> >::type,
            Skipper
        >::type
    {};

    template <typename Skipper, typename Char, typename Traits, typename Allocator>
    struct in_state_skipper<Skipper, std::basic_string<Char, Traits, Allocator> >
      : proto::subscript<
            typename proto::terminal<in_state_tag<Char const*> >::type,
            Skipper
        >::type
    {};

}}}

#endif
