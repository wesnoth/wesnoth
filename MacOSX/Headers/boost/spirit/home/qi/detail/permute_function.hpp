/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(SPIRIT_PERMUTE_FUNCTION_MARCH_13_2007_1129AM)
#define SPIRIT_PERMUTE_FUNCTION_MARCH_13_2007_1129AM

#include <boost/spirit/home/support/unused.hpp>
#include <boost/optional.hpp>

namespace boost { namespace spirit { namespace qi { namespace detail
{
    template <typename Iterator, typename Context, typename Skipper>
    struct permute_function
    {
        permute_function(
            Iterator& first, Iterator const& last
          , Context& context, Skipper const& skipper)
          : first(first)
          , last(last)
          , context(context)
          , skipper(skipper)
        {
        }

        template <typename Component, typename Attribute>
        bool operator()(Component const& component, Attribute& attr)
        {
            // return true if the parser succeeds and the slot is not yet taken
            typedef typename Component::director director;
            if (!*taken
                && director::parse(component, first, last, context, skipper, attr))
            {
                *taken = true;
                ++taken;
                return true;
            }
            ++taken;
            return false;
        }

        template <typename Component, typename Attribute>
        bool operator()(Component const& component, boost::optional<Attribute>& attr)
        {
            // return true if the parser succeeds and the slot is not yet taken
            typedef typename Component::director director;
            Attribute val;
            if (!*taken
                && director::parse(component, first, last, context, skipper, val))
            {
                attr = val;
                *taken = true;
                ++taken;
                return true;
            }
            ++taken;
            return false;
        }

        template <typename Component>
        bool operator()(Component const& component)
        {
            // return true if the parser succeeds and the slot is not yet taken
            typedef typename Component::director director;
            if (!*taken
                && director::parse(component, first, last, context, skipper, unused))
            {
                *taken = true;
                ++taken;
                return true;
            }
            ++taken;
            return false;
        }

        Iterator& first;
        Iterator const& last;
        Context& context;
        Skipper const& skipper;
        bool* taken;
    };
}}}}

#endif
