/*=============================================================================
    Copyright (c) 2001-2006 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(FUSION_TRANSFORM_VIEW_ITERATOR_07162005_1033)
#define FUSION_TRANSFORM_VIEW_ITERATOR_07162005_1033

#include <boost/fusion/support/iterator_base.hpp>
#include <boost/fusion/support/category_of.hpp>
#include <boost/fusion/iterator/mpl/convert_iterator.hpp>
#include <boost/fusion/adapted/mpl/mpl_iterator.hpp>
#include <boost/fusion/view/transform_view/detail/deref_impl.hpp>
#include <boost/fusion/view/transform_view/detail/next_impl.hpp>
#include <boost/fusion/view/transform_view/detail/prior_impl.hpp>
#include <boost/fusion/view/transform_view/detail/value_of_impl.hpp>
#include <boost/fusion/view/transform_view/detail/advance_impl.hpp>
#include <boost/fusion/view/transform_view/detail/distance_impl.hpp>
#include <boost/fusion/view/transform_view/detail/equal_to_impl.hpp>

namespace boost { namespace fusion
{
    // Unary Version
    struct transform_view_iterator_tag;

    template <typename First, typename F>
    struct transform_view_iterator
        : iterator_base<transform_view_iterator<First, F> >
    {
        typedef transform_view_iterator_tag fusion_tag;
        typedef convert_iterator<First> converter;
        typedef typename converter::type first_type;
        typedef typename traits::category_of<first_type>::type category;
        typedef F transform_type;

        transform_view_iterator(First const& first, F const& f)
            : first(converter::call(first)), f(f) {}

        first_type first;
        transform_type f;
    };

    // Binary Version
    struct transform_view_iterator2_tag;

    template <typename First1, typename First2, typename F>
    struct transform_view_iterator2
        : iterator_base<transform_view_iterator2<First1, First2, F> >
    {
        typedef transform_view_iterator2_tag fusion_tag;
        typedef convert_iterator<First1> converter1;
        typedef convert_iterator<First2> converter2;
        typedef typename converter1::type first1_type;
        typedef typename converter2::type first2_type;
        typedef typename traits::category_of<first1_type>::type category;
        typedef F transform_type;

        transform_view_iterator2(First1 const& first1, First2 const& first2, F const& f)
            : first1(converter1::call(first1)), first2(converter2::call(first2)), f(f) {}

        first1_type first1;
        first2_type first2;
        transform_type f;
    };
}}

#endif

