/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(SPIRIT_SEQUENCE_BASE_APR_22_2006_0811AM)
#define SPIRIT_SEQUENCE_BASE_APR_22_2006_0811AM

#include <boost/spirit/home/qi/domain.hpp>
#include <boost/spirit/home/support/attribute_transform.hpp>
#include <boost/spirit/home/support/algorithm/any_if.hpp>
#include <boost/spirit/home/support/detail/what_function.hpp>
#include <boost/spirit/home/support/unused.hpp>
#include <boost/spirit/home/support/detail/values.hpp>
#include <boost/fusion/include/as_vector.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/mpl/identity.hpp>

namespace boost { namespace spirit { namespace qi
{
    template <typename Derived>
    struct sequence_base // this class is shared by sequence and expect
    {
        template <typename T>
        struct transform_child : mpl::identity<T> {};

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
                sequence_base<Derived>, Component, Iterator, Context
            >
        {
        };

        template <typename Iterator, typename Context>
        struct attribute_not_unused
        {
            template <typename Component>
            struct apply
              : spirit::traits::is_not_unused<typename
                    traits::attribute_of<
                        qi::domain, Component, Context, Iterator>::type
                > 
            {};
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
            Iterator iter = first;
            typedef attribute_not_unused<Iterator, Context> predicate;

            // return false if *any* of the parsers fail
            if (spirit::any_if(
                component.elements, attr
              , Derived::fail_function(iter, last, context, skipper), predicate()))
                return false;
            first = iter;
            return true;
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            std::string result = Derived::what_();
            fusion::for_each(component.elements,
                spirit::detail::what_function<Context>(result, ctx));
            result += "]";
            return result;
        }
    };
}}}

#endif
