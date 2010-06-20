//  Copyright (c) 2001-2008 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_KARMA_DELIMIT_MAR_02_2007_0217PM)
#define BOOST_SPIRIT_KARMA_DELIMIT_MAR_02_2007_0217PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/support/component.hpp>
#include <boost/spirit/home/support/attribute_of.hpp>
#include <boost/spirit/home/karma/domain.hpp>
#include <boost/fusion/include/at.hpp>
#include <boost/fusion/include/value_at.hpp>

namespace boost { namespace spirit { namespace karma
{
    ///////////////////////////////////////////////////////////////////////////
    //  The delimit_space generator is used for delimit[...] directives.
    ///////////////////////////////////////////////////////////////////////////
    struct delimit_space
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
            Context& ctx, Delimiter const& /*d*/, Parameter const& param)
        {
            //  the delimit_space generator simply dispatches to the embedded
            //  generator while supplying a single space as the new delimiter
            //  to use
            typedef typename
                result_of::right<Component>::type::director
            director;

            return director::generate(spirit::right(component),
                sink, ctx, spirit::as_component(karma::domain(), ' '), param);
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            std::string result = "delimit[";

            typedef typename
                spirit::result_of::right<Component>::type::director
            director;

            result += director::what(spirit::right(component), ctx);
            result += "]";
            return result;
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    //  The delimit_ generator is used for delimit(d)[...] directives.
    ///////////////////////////////////////////////////////////////////////////
    struct delimit_
    {
        template <typename Component, typename Context, typename Unused>
        struct attribute
          : traits::attribute_of<
                karma::domain,
                typename result_of::subject<Component>::type,
                Context
            >
        {
        };

        template <typename Component, typename OutputIterator,
            typename Context, typename Delimiter, typename Parameter>
        static bool
        generate(Component const& component, OutputIterator& sink,
            Context& ctx, Delimiter const& /*d*/, Parameter const& param)
        {
            //  the delimit generator simply dispatches to the embedded
            //  generator while supplying it's argument as the new delimiter
            //  to use
            typedef typename
                spirit::result_of::subject<Component>::type::director
            director;

            return director::generate(spirit::subject(component), sink, ctx,
                spirit::as_component(
                    karma::domain(), spirit::argument1(component)),
                param);
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            std::string result = "delimit(";

            typedef typename
                spirit::result_of::argument1<Component>::type::director
            delimiter;

            result += delimiter::what(spirit::argument1(component), ctx);
            result +=")[";

            typedef typename
                spirit::result_of::subject<Component>::type::director
            director;

            result += director::what(spirit::subject(component), ctx);
            result += "]";
            return result;
        }
    };

}}}

#endif
