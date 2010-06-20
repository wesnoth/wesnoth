/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(SPIRIT_PASS_FUNCTION_FEB_05_2007_1138AM)
#define SPIRIT_PASS_FUNCTION_FEB_05_2007_1138AM

#include <boost/spirit/home/support/unused.hpp>
#include <boost/optional.hpp>

namespace boost { namespace spirit { namespace qi { namespace detail
{
    template <typename Iterator, typename Context, typename Skipper>
    struct pass_function
    {
        pass_function(
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
            // return true if the parser succeeds
            typedef typename Component::director director;
            return director::parse(component, first, last, context, skipper, attr);
        }

        template <typename Component, typename Attribute>
        bool operator()(Component const& component, boost::optional<Attribute>& attr)
        {
            // return true if the parser succeeds
            typedef typename Component::director director;
            Attribute val;
            if (director::parse(component, first, last, context, skipper, val))
            {
                attr = val;
                return true;
            }
            return false;
        }

        template <typename Component>
        bool operator()(Component const& component)
        {
            // return true if the parser succeeds
            typedef typename Component::director director;
            return director::parse(component, first, last, context, skipper, unused);
        }

        Iterator& first;
        Iterator const& last;
        Context& context;
        Skipper const& skipper;
    };
}}}}

#endif
