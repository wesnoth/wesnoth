/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(SPIRIT_DIFFERENCE_FEB_11_2007_1250PM)
#define SPIRIT_DIFFERENCE_FEB_11_2007_1250PM

#include <boost/spirit/home/qi/domain.hpp>
#include <boost/spirit/home/support/component.hpp>
#include <boost/spirit/home/support/attribute_of.hpp>
#include <vector>

namespace boost { namespace spirit { namespace qi
{
    struct difference
    {
        template <typename Component, typename Context, typename Iterator>
        struct attribute
        {
            typedef typename
                result_of::left<Component>::type
            left_type;

            typedef typename
                traits::attribute_of<
                    qi::domain, left_type, Context, Iterator>::type
            type;
        };

        template <
            typename Component
          , typename Iterator, typename Context
          , typename Skipper, typename Attribute>
        static bool parse(
            Component const& component
          , Iterator& first, Iterator const& last
          , Context& context, Skipper const& skipper
          , Attribute& attr)
        {
            typedef typename
                result_of::left<Component>::type::director
            ldirector;

            typedef typename
                result_of::right<Component>::type::director
            rdirector;

            // Unlike classic Spirit, with this version of difference, the rule
            // lit("policeman") - "police" will always fail to match.

            // Spirit2 does not count the matching chars while parsing and
            // there is no reliable and fast way to check if the LHS matches
            // more than the RHS.

            // Try RHS first
            Iterator start = first;
            if (rdirector::parse(spirit::right(component), first, last, context, 
                skipper, unused))
            {
                // RHS succeeds, we fail.
                first = start;
                return false;
            }
            // RHS fails, now try LHS
            return ldirector::parse(spirit::left(component), first, last, 
                context, skipper, attr);
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            std::string result = "difference[";

            typedef typename
                result_of::left<Component>::type::director
            ldirector;

            typedef typename
                result_of::right<Component>::type::director
            rdirector;

            result += ldirector::what(spirit::left(component), ctx);
            result += ", ";
            result += rdirector::what(spirit::right(component), ctx);
            result += "]";
            return result;
        }
    };
}}}

#endif
