//  Copyright (c) 2001-2008 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_FUNCTOR_DIRECTOR_APR_01_2007_0847AM)
#define BOOST_SPIRIT_FUNCTOR_DIRECTOR_APR_01_2007_0847AM

#include <boost/spirit/home/support/auxiliary/functor_holder.hpp>
#include <boost/spirit/home/support/component.hpp>
#include <boost/spirit/home/qi/domain.hpp>

namespace boost { namespace spirit { namespace qi
{
    // this is the director for all functor parsers
    struct functor_director
    {
        // return value of the parser
        template <typename Component, typename Context, typename Iterator>
        struct attribute
        {
            typedef typename
                result_of::subject<Component>::type::functor_type
            functor_type;

            typedef typename
                functor_type::template result<Iterator, Context>::type
            type;
        };

        // parse functionality, delegates back to the corresponding functor
        template <typename Component, typename Iterator, typename Context,
            typename Skipper, typename Attribute>
        static bool parse(Component const& component,
            Iterator& first, Iterator const& last, Context& context,
            Skipper const& skipper, Attribute& attr)
        {
            // main entry point, just forward to the functor parse function
            qi::skip(first, last, skipper);         // always do a pre-skip
            return subject(component).held->parse(first, last, context, attr);
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            return "functor";
        }
    };

}}}

#endif
