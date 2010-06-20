//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_KARMA_EOL_JUL_08_2008_1014AM)
#define BOOST_SPIRIT_KARMA_EOL_JUL_08_2008_1014AM

#include <boost/spirit/home/karma/domain.hpp>
#include <boost/spirit/home/karma/delimit.hpp>
#include <boost/spirit/home/support/unused.hpp>
#include <boost/spirit/home/karma/detail/generate_to.hpp>

namespace boost { namespace spirit { namespace karma
{
    struct eol_generator
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
            detail::generate_to(sink, '\n');
            karma::delimit(sink, d);          // always do post-delimiting
            return true;
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            return "eol";
        }
    };

}}}

#endif
