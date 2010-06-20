/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(SPIRIT_LIST_MARCH_24_2007_1031AM)
#define SPIRIT_LIST_MARCH_24_2007_1031AM

#include <boost/spirit/home/support/component.hpp>
#include <boost/spirit/home/support/detail/container.hpp>
#include <boost/spirit/home/support/attribute_transform.hpp>
#include <vector>

namespace boost { namespace spirit { namespace qi
{
    struct list
    {
        template <typename T>
        struct build_attribute_container
        {
            typedef std::vector<T> type;
        };

        template <typename Component, typename Context, typename Iterator>
        struct attribute :
            build_container<list, Component, Iterator, Context>
        {
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

            typename container::result_of::value<Attribute>::type val;
            if (ldirector::parse(
                    spirit::left(component)
                  , first, last, context, skipper, val)
                )
            {
                container::push_back(attr, val);
                Iterator i = first;
                while(
                    rdirector::parse(
                        spirit::right(component)
                      , i, last, context, skipper, unused)
                 && ldirector::parse(
                        spirit::left(component)
                      , i, last, context, skipper, val)
                    )
                {
                    container::push_back(attr, val);
                    first = i;
                }
                return true;
            }
            return false;
        }


        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            std::string result = "list[";

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
