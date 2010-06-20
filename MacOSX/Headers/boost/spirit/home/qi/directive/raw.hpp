/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(SPIRIT_RAW_APRIL_9_2007_0912AM)
#define SPIRIT_RAW_APRIL_9_2007_0912AM

#include <boost/spirit/home/support/component.hpp>
#include <boost/spirit/home/support/unused.hpp>
#include <boost/range/iterator_range.hpp>

namespace boost { namespace spirit { namespace qi
{
    struct raw_director
    {
        template <typename Component, typename Context, typename Iterator>
        struct attribute
        {
            typedef iterator_range<Iterator> type;
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
                result_of::subject<Component>::type::director
            director;

            qi::skip(first, last, skipper);
            Iterator i = first;
            if (director::parse(
                spirit::subject(component), i, last, context, skipper, unused))
            {
                attr = Attribute(first, i);
                first = i;
                return true;
            }
            return false;
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            std::string result = "raw[";

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
