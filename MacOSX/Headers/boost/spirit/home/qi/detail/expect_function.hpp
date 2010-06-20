/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(SPIRIT_EXPECT_FUNCTION_APR_29_2007_0558PM)
#define SPIRIT_EXPECT_FUNCTION_APR_29_2007_0558PM

#include <boost/spirit/home/support/unused.hpp>

namespace boost { namespace spirit { namespace qi { namespace detail
{
    template <
        typename Iterator, typename Context
      , typename Skipper, typename Exception>
    struct expect_function
    {
        expect_function(
            Iterator& first, Iterator const& last
          , Context& context, Skipper const& skipper)
          : first(first)
          , last(last)
          , context(context)
          , skipper(skipper)
          , is_first(true)
        {
        }

        template <typename Component, typename Attribute>
        bool operator()(Component const& component, Attribute& attr)
        {
            // if we are testing the first component in the sequence,
            // return true if the parser fails, if this not the first
            // component, throw exception if the parser fails
            typedef typename Component::director director;
            if (!director::parse(component, first, last, context, skipper, attr))
            {
                if (is_first)
                {
                    is_first = false;
                    return true;
                }
                Exception x = {first, last, director::what(component, context) };
                throw x;
            }
            is_first = false;
            return false;
        }

        template <typename Component>
        bool operator()(Component const& component)
        {
            // if we are testing the first component in the sequence,
            // return true if the parser fails, if this not the first
            // component, throw exception if the parser fails
            typedef typename Component::director director;
            if (!director::parse(component, first, last, context, skipper, unused))
            {
                if (is_first)
                {
                    is_first = false;
                    return true;
                }
                Exception x = {first, last, director::what(component, context) };
                throw x;
            }
            is_first = false;
            return false;
        }

        Iterator& first;
        Iterator const& last;
        Context& context;
        Skipper const& skipper;
        bool is_first;
    };
}}}}

#endif
