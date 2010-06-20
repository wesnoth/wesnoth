/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_SPIRIT_EPS_MARCH_23_2007_0454PM)
#define BOOST_SPIRIT_EPS_MARCH_23_2007_0454PM

#include <boost/spirit/home/qi/domain.hpp>
#include <boost/spirit/home/qi/skip.hpp>
#include <boost/spirit/home/support/unused.hpp>
#include <boost/fusion/include/at.hpp>

namespace boost { namespace spirit { namespace qi
{
    struct eps_parser
    {
        template <typename Component, typename Context, typename Iterator>
        struct attribute
        {
            typedef unused_type type;
        };

        template <
            typename Component
          , typename Iterator, typename Context
          , typename Skipper, typename Attribute>
        static bool parse(
            Component const& /*component*/
          , Iterator& first, Iterator const& last
          , Context& /*context*/, Skipper const& skipper
          , Attribute& /*attr*/)
        {
            qi::skip(first, last, skipper);
            return true;
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            return "eps";
        }
    };

    struct semantic_predicate
    {
        template <typename Component, typename Context, typename Iterator>
        struct attribute
        {
            typedef unused_type type;
        };

        template <
            typename Component
          , typename Iterator, typename Context
          , typename Skipper, typename Attribute>
        static bool parse(
            Component const& component
          , Iterator& first, Iterator const& last
          , Context& context, Skipper const& skipper
          , Attribute& /*attr*/)
        {
            qi::skip(first, last, skipper);
            return fusion::at_c<0>(component.elements)(unused, context);
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            return "semantic-predicate";
        }
    };
}}}

#endif
