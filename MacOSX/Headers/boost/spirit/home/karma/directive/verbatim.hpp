//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_KARMA_VERBATIM_MAR_02_2007_0303PM)
#define BOOST_SPIRIT_KARMA_VERBATIM_MAR_02_2007_0303PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/support/unused.hpp>
#include <boost/spirit/home/support/attribute_of.hpp>
#include <boost/spirit/home/karma/domain.hpp>
#include <boost/fusion/include/at.hpp>
#include <boost/fusion/include/value_at.hpp>

namespace boost { namespace spirit { namespace karma
{
    ///////////////////////////////////////////////////////////////////////////
    //  The verbatim generator is used for verbatim[...] directives.
    ///////////////////////////////////////////////////////////////////////////
    struct verbatim
    {
        template <typename Component, typename Context, typename Unused>
        struct attribute
          : traits::attribute_of<
                karma::domain,
                typename result_of::right<Component>::type, 
                Context
            >
        {
        };

        template <typename Component, typename OutputIterator, 
            typename Context, typename Delimiter, typename Parameter>
        static bool 
        generate(Component const& component, OutputIterator& sink, 
            Context& ctx, Delimiter const& d, Parameter const& param) 
        {
            //  the verbatim generator simply dispatches to the embedded 
            //  generator while supplying unused as the new delimiter
            //  to avoid delimiting down the generator stream
            typedef typename 
                spirit::result_of::right<Component>::type::director
            director;
            
            if (director::generate(spirit::right(component), sink, ctx, 
                    unused, param))
            {
                karma::delimit(sink, d);           // always do post-delimiting 
                return true;
            }
            return false;
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            std::string result = "verbatim[";

            typedef typename
                spirit::result_of::right<Component>::type::director
            director;

            result += director::what(spirit::right(component), ctx);
            result += "]";
            return result;
        }
    };

}}}

#endif
