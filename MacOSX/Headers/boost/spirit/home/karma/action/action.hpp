//  Copyright (c) 2001-2008 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_KARMA_ACTION_MAR_07_2007_0851AM)
#define BOOST_SPIRIT_KARMA_ACTION_MAR_07_2007_0851AM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/support/component.hpp>
#include <boost/spirit/home/support/detail/values.hpp>
#include <boost/spirit/home/support/attribute_of.hpp>
#include <boost/spirit/home/support/detail/action_dispatch.hpp>
#include <boost/spirit/home/karma/domain.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/identity.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/type_traits/is_same.hpp>
#include <vector>

namespace boost { namespace spirit { namespace karma
{
    ///////////////////////////////////////////////////////////////////////////
    struct sequence;    // forward declaration only
    
    ///////////////////////////////////////////////////////////////////////////
    struct action
    {
        template <typename Component, typename Context, typename Unused>
        struct attribute
          : traits::attribute_of<
                karma::domain,
                typename result_of::left<Component>::type,
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
            typedef typename
                result_of::left<Component>::type::director
            director;
            typedef typename is_same<director, sequence>::type is_sequence;

            typedef typename
                attribute<Component, Context, unused_type>::type
            param_type;

            // create a parameter if one is not supplied
            // this creates a _copy_ of the parameter because the semantic
            // action likely will change parts of this
            typename mpl::if_<
                is_same<typename remove_const<Parameter>::type, unused_type>,
                param_type,
                Parameter
            >::type p = spirit::detail::make_value<param_type>::call(param);

            // call the function, passing the parameter, the context
            // and a bool flag that the client can set to false to
            // fail generating.
            // The client can return false to fail parsing.
            bool pass = spirit::detail::action_dispatch(
                spirit::right(component), p, ctx, is_sequence());

            return pass &&
                director::generate(spirit::left(component), sink, ctx, d, p);
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            typedef typename
                spirit::result_of::left<Component>::type::director
            director;
            return director::what(spirit::left(component), ctx);
        }
    };

}}}

#endif
