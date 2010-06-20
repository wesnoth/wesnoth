//  Copyright (c) 2001-2008 Hartmut Kaiser
//  Copyright (c) 2001-2007 Joel de Guzman
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(SPIRIT_KARMA_SEQUENCE_FEB_28_2007_0247PM)
#define SPIRIT_KARMA_SEQUENCE_FEB_28_2007_0247PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/karma/domain.hpp>
#include <boost/spirit/home/karma/operator/detail/sequence.hpp>
#include <boost/spirit/home/support/attribute_transform.hpp>
#include <boost/spirit/home/support/detail/what_function.hpp>
#include <boost/spirit/home/support/algorithm/any_if.hpp>
#include <boost/spirit/home/support/unused.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/include/as_vector.hpp>
#include <boost/fusion/include/transform.hpp>
#include <boost/fusion/include/filter_if.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/mpl/not.hpp>

namespace boost { namespace spirit { namespace karma
{
    struct sequence
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
                sequence, Component, Iterator, Context, mpl::true_
            >
        {
        };

        template <typename Context>
        struct attribute_not_unused
        {
            template <typename Component>
            struct apply
              : spirit::traits::is_not_unused<typename
                  traits::attribute_of<karma::domain, Component, Context>::type>
            {};
        };

        template <typename Component, typename OutputIterator,
            typename Context, typename Delimiter, typename Parameter>
        static bool
        generate(Component const& component, OutputIterator& sink,
            Context& ctx, Delimiter const& d, Parameter const& param)
        {
            detail::sequence_generate<OutputIterator, Context, Delimiter>
                f (sink, ctx, d);

            typedef attribute_not_unused<Context> predicate;

            // f returns true if *any* of the generators fail
            return !spirit::any_if(component.elements, param, f, predicate());
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            std::string result = "sequence[";
            fusion::for_each(component.elements,
                spirit::detail::what_function<Context>(result, ctx));
            result += "]";
            return result;
        }
    };

}}} // namespace boost::spirit::karma

#endif
