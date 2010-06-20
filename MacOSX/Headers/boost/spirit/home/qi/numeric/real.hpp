/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_SPIRIT_REAL_APR_18_2006_0850AM)
#define BOOST_SPIRIT_REAL_APR_18_2006_0850AM

#include <boost/spirit/home/qi/skip.hpp>
#include <boost/spirit/home/qi/numeric/real_policies.hpp>
#include <boost/spirit/home/qi/numeric/numeric_utils.hpp>
#include <boost/spirit/home/qi/numeric/detail/real_impl.hpp>

namespace boost { namespace spirit { namespace qi
{
    namespace detail
    {
        template <typename RealPolicies>
        struct real_policy;
    }

    template <
        typename T = double,
        typename RealPolicies = real_policies<T>
    >
    struct real_parser
    {
        template <typename Component, typename Context, typename Iterator>
        struct attribute
        {
            typedef T type;
        };

        template <
            typename Component
          , typename Iterator, typename Context
          , typename Skipper, typename Attribute>
        static bool parse(
            Component const& component
          , Iterator& first, Iterator const& last
          , Context& /*context*/, Skipper const& skipper
          , Attribute& attr)
        {
            RealPolicies const& p = detail::real_policy<RealPolicies>::get(
                fusion::at_c<0>(component.elements));

            qi::skip(first, last, skipper);
            return detail::real_impl<T, RealPolicies>::parse(first, last, attr, p);
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            return "real number";
        }
    };
}}}

#endif
