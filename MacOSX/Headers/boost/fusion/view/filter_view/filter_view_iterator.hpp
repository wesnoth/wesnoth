/*=============================================================================
    Copyright (c) 2001-2006 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(FUSION_FILTER_VIEW_ITERATOR_05062005_0849)
#define FUSION_FILTER_VIEW_ITERATOR_05062005_0849

#include <boost/fusion/iterator/mpl/convert_iterator.hpp>
#include <boost/fusion/adapted/mpl/mpl_iterator.hpp>
#include <boost/fusion/support/iterator_base.hpp>
#include <boost/fusion/view/filter_view/detail/deref_impl.hpp>
#include <boost/fusion/view/filter_view/detail/next_impl.hpp>
#include <boost/fusion/view/filter_view/detail/value_of_impl.hpp>
#include <boost/fusion/view/filter_view/detail/equal_to_impl.hpp>
#include <boost/fusion/algorithm/query/detail/find_if.hpp>

namespace boost { namespace fusion
{
    struct filter_view_iterator_tag;
    struct forward_traversal_tag;

    template <typename First, typename Last, typename Pred>
    struct filter_iterator : iterator_base<filter_iterator<First, Last, Pred> >
    {
        typedef convert_iterator<First> first_converter;
        typedef typename first_converter::type first_iter;
        typedef convert_iterator<Last> last_converter;
        typedef typename last_converter::type last_iter;

        typedef filter_view_iterator_tag fusion_tag;
        typedef forward_traversal_tag category;
        typedef detail::static_find_if<first_iter, last_iter, Pred> filter;
        typedef typename filter::type first_type;
        typedef last_iter last_type;
        typedef Pred pred_type;

        filter_iterator(First const& first)
            : first(filter::call(first_converter::call(first))) {}

        first_type first;
    };
}}

#endif


