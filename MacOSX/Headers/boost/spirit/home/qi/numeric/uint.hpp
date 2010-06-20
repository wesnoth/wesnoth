/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(SPIRIT_UINT_APR_17_2006_0901AM)
#define SPIRIT_UINT_APR_17_2006_0901AM

#include <boost/spirit/home/qi/skip.hpp>
#include <boost/spirit/home/qi/numeric/numeric_utils.hpp>
#include <boost/mpl/assert.hpp>

namespace boost { namespace spirit { namespace qi
{
    template <typename T, unsigned Radix, unsigned MinDigits, int MaxDigits>
    struct uint_parser
    {
        // check template parameter 'Radix' for validity
        BOOST_MPL_ASSERT_MSG(
            Radix == 2 || Radix == 8 || Radix == 10 || Radix == 16,
            not_supported_radix, ());

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
            qi::skip(first, last, skipper);
            return extract_uint<T, Radix, MinDigits, MaxDigits>
                ::call(first, last, attr);
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            return "unsigned integer";
        }
    };
}}}

#endif
