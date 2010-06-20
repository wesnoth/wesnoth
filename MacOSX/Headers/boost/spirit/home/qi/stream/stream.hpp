//  Copyright (c) 2001-2008 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_STREAM_MAY_05_2007_1228PM)
#define BOOST_SPIRIT_STREAM_MAY_05_2007_1228PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/qi/detail/string_parse.hpp>
#include <boost/spirit/home/qi/stream/detail/match_manip.hpp>
#include <boost/spirit/home/qi/stream/detail/iterator_istream.hpp>
#include <boost/spirit/home/support/detail/hold_any.hpp>

#include <iosfwd>
#include <sstream>

///////////////////////////////////////////////////////////////////////////////
namespace boost { namespace spirit
{
    // overload the streaming operators for the unused_type
    template <typename Char, typename Traits>
    inline std::basic_istream<Char, Traits>&
    operator>> (std::basic_istream<Char, Traits>& is, unused_type&)
    {
        return is;
    }
}}

///////////////////////////////////////////////////////////////////////////////
namespace boost { namespace spirit { namespace qi
{
    template <typename Char, typename T = spirit::hold_any>
    struct any_stream
    {
        template <typename Component, typename Context, typename Iterator>
        struct attribute
        {
            typedef T type;
        };

        template <
            typename Component
          , typename Iterator, typename Context
          , typename Skipper, typename Attribute>
        static bool parse(
            Component const& /*component*/
          , Iterator& first, Iterator const& last
          , Context& /*context*/, Skipper const& skipper
          , Attribute& attr)
        {
            typedef qi::detail::iterator_source<Iterator> source_device;
            typedef boost::iostreams::stream<source_device> instream;

            qi::skip(first, last, skipper);
            instream in (first, last);
            in >> attr;                       // use existing operator>>()
            return in.good() || in.eof();
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            return "any-stream";
        }
    };

}}}

#endif
