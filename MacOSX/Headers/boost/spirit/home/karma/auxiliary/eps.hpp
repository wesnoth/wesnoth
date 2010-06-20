//  Copyright (c) 2001-2008 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_KARMA_EPS_APRIL_21_2007_0246PM)
#define BOOST_SPIRIT_KARMA_EPS_APRIL_21_2007_0246PM

#include <boost/spirit/home/karma/domain.hpp>
#include <boost/spirit/home/karma/delimit.hpp>
#include <boost/spirit/home/support/unused.hpp>
#include <boost/fusion/include/at.hpp>

namespace boost { namespace spirit { namespace karma
{
    struct eps_generator
    {
        template <typename Component, typename Context, typename Unused>
        struct attribute
        {
            typedef unused_type type;
        };

        template <typename Component, typename OutputIterator,
            typename Context, typename Delimiter, typename Parameter>
        static bool
        generate(Component const& /*component*/, OutputIterator& sink,
            Context& /*ctx*/, Delimiter const& d, Parameter const& /*param*/)
        {
            karma::delimit(sink, d);
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
        template <typename Component, typename Context, typename Unused>
        struct attribute
        {
            typedef unused_type type;
        };

        template <typename Component, typename OutputIterator,
            typename Context, typename Delimiter, typename Parameter>
        static bool
        generate(Component const& component, OutputIterator& sink,
            Context& ctx, Delimiter const& d, Parameter const& /*param*/)
        {
            karma::delimit(sink, d);
            return fusion::at_c<0>(component.elements)(unused, ctx);
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            return "semantic-predicate";
        }
    };

}}}

#endif
