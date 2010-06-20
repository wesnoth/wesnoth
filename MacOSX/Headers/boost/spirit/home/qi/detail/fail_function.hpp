/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(SPIRIT_FAIL_FUNCTION_APR_22_2006_0159PM)
#define SPIRIT_FAIL_FUNCTION_APR_22_2006_0159PM

#include <boost/spirit/home/support/unused.hpp>

namespace boost { namespace spirit { namespace qi { namespace detail
{
    template <typename Iterator, typename Context, typename Skipper>
    struct fail_function
    {
        fail_function(
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
            // return true if the parser fails
            typedef typename Component::director director;
            return !director::parse(component, first, last, context, skipper, attr);
        }

        template <typename Component>
        bool operator()(Component const& component)
        {
            // return true if the parser fails
            typedef typename Component::director director;
            return !director::parse(component, first, last, context, skipper, unused);
        }

        Iterator& first;
        Iterator const& last;
        Context& context;
        Skipper const& skipper;
    };
}}}}

#endif
