//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_KARMA_NONE_MARCH_26_2007_1227PM)
#define BOOST_SPIRIT_KARMA_NONE_MARCH_26_2007_1227PM

#include <boost/spirit/home/karma/domain.hpp>
#include <boost/spirit/home/karma/delimit.hpp>
#include <boost/spirit/home/support/unused.hpp>

namespace boost { namespace spirit { namespace karma
{
    struct none
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
            karma::delimit(sink, d);           // always do post-delimiting 
            return false;
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            return "none";
        }
    };

}}}

#endif
