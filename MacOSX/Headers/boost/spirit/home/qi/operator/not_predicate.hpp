/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(SPIRIT_NOT_PREDICATE_MARCH_23_2007_0618PM)
#define SPIRIT_NOT_PREDICATE_MARCH_23_2007_0618PM

#include <boost/spirit/home/support/component.hpp>
#include <boost/spirit/home/support/unused.hpp>

namespace boost { namespace spirit { namespace qi
{
    struct not_predicate
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
            typedef typename
                result_of::subject<Component>::type::director
            director;

            Iterator i = first;
            return !director::parse(
                subject(component), i, last, context, skipper, unused);
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            std::string result = "not-predicate[";

            typedef typename
                result_of::subject<Component>::type::director
            director;

            result += director::what(subject(component), ctx);
            result += "]";
            return result;
        }
    };
}}}

#endif
