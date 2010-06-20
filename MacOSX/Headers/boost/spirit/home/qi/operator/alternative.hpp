/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman
    Copyright (c) 2001-2008 Hartmut Kaiser

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(SPIRIT_ALTERNATIVE_FEB_05_2007_1153AM)
#define SPIRIT_ALTERNATIVE_FEB_05_2007_1153AM

#include <boost/spirit/home/qi/detail/alternative_function.hpp>
#include <boost/spirit/home/support/attribute_transform.hpp>
#include <boost/spirit/home/support/detail/what_function.hpp>
#include <boost/spirit/home/support/unused.hpp>
#include <boost/spirit/home/support/as_variant.hpp>
#include <boost/fusion/include/any.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/include/mpl.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/push_front.hpp>
#include <boost/variant.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/mpl/end.hpp>
#include <boost/mpl/find_if.hpp>
#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/identity.hpp>

namespace boost { namespace spirit { namespace qi
{
    struct alternative
    {
        template <typename T>
        struct transform_child : mpl::identity<T> {};

        template <typename All, typename Filtered>
        struct build_container
        {
            // if the original attribute list does not contain any unused
            // attributes it is used, otherwise a single unused_type is
            // pushed to the front the list. This is to make sure that if
            // there is an unused in the list it is the first one.
            typedef typename
                mpl::find_if<All, is_same<mpl::_, unused_type> >::type
            unused_;

            typedef typename
                mpl::eval_if<
                    is_same<unused_, typename mpl::end<All>::type>,
                    mpl::identity<All>,
                    fusion::result_of::push_front<Filtered, unused_type>
                >::type
            attribute_sequence;

            // Ok, now make a variant over the attribute_sequence. It's
            // a pity that make_variant_over does not support forward MPL
            // sequences. We use our own conversion metaprogram (as_variant).
            typedef typename
                as_variant<attribute_sequence>::type
            type;
        };

        template <typename Component, typename Context, typename Iterator>
        struct attribute :
            build_fusion_sequence<alternative, Component, Iterator, Context>
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
            detail::alternative_function<Iterator, Context, Skipper, Attribute>
                f(first, last, context, skipper, attr);

            // return true if *any* of the parsers succeed
            return fusion::any(component.elements, f);
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            std::string result = "alternatives[";
            fusion::for_each(component.elements,
                spirit::detail::what_function<Context>(result, ctx));
            result += "]";
            return result;
        }
    };
}}}

#endif
