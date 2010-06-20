/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(SPIRIT_SEQUENTIAL_OR_MARCH_12_2007_1130PM)
#define SPIRIT_SEQUENTIAL_OR_MARCH_12_2007_1130PM

#include <boost/spirit/home/qi/detail/pass_function.hpp>
#include <boost/spirit/home/support/attribute_transform.hpp>
#include <boost/spirit/home/support/detail/what_function.hpp>
#include <boost/spirit/home/support/algorithm/any_ns.hpp>
#include <boost/fusion/include/as_vector.hpp>
#include <boost/fusion/include/for_each.hpp>

namespace boost { namespace spirit { namespace qi
{
    struct sequential_or
    {
        template <typename T>
        struct transform_child
        {
            typedef boost::optional<T> type;
        };

        template <typename All, typename Filtered>
        struct build_container
        {
            typedef
                typename fusion::result_of::as_vector<Filtered>::type
            type;
        };

        template <typename Component, typename Context, typename Iterator>
        struct attribute :
            build_fusion_sequence<
                sequential_or, Component, Iterator, Context
            >
        {
        };

        template <
            typename Component
          , typename Iterator, typename Context
          , typename Skipper, typename Attribute>
        static bool parse(
            Component const& component
          , Iterator& first, Iterator const& last
          , Context& context, Skipper const& skipper
          , Attribute& attr)
        {
            qi::detail::pass_function<Iterator, Context, Skipper>
                f(first, last, context, skipper);

            // return true if *any* of the parsers succeed
            // (we use the non-short-circuiting version: any_ns
            // to force all elements to be tested)
            return spirit::any_ns(component.elements, attr, f);
        }


        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            std::string result = "sequential-or[";
            fusion::for_each(component.elements,
                spirit::detail::what_function<Context>(result, ctx));
            result += "]";
            return result;
        }
    };
}}}

#endif
