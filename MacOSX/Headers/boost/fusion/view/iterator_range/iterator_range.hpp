/*=============================================================================
    Copyright (c) 2001-2006 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(FUSION_ITERATOR_RANGE_05062005_1224)
#define FUSION_ITERATOR_RANGE_05062005_1224

#include <boost/fusion/support/detail/access.hpp>
#include <boost/fusion/support/sequence_base.hpp>
#include <boost/fusion/support/category_of.hpp>
#include <boost/fusion/iterator/distance.hpp>
#include <boost/fusion/iterator/mpl/convert_iterator.hpp>
#include <boost/fusion/view/iterator_range/detail/begin_impl.hpp>
#include <boost/fusion/view/iterator_range/detail/end_impl.hpp>
#include <boost/fusion/view/iterator_range/detail/at_impl.hpp>
#include <boost/fusion/view/iterator_range/detail/value_at_impl.hpp>
#include <boost/fusion/adapted/mpl/mpl_iterator.hpp>
#include <boost/mpl/bool.hpp>

namespace boost { namespace fusion
{
    struct iterator_range_tag;
    struct fusion_sequence_tag;

    template <typename First, typename Last>
    struct iterator_range : sequence_base<iterator_range<First, Last> >
    {
        typedef typename convert_iterator<First>::type begin_type;
        typedef typename convert_iterator<Last>::type end_type;
        typedef iterator_range_tag fusion_tag;
        typedef fusion_sequence_tag tag; // this gets picked up by MPL
        typedef typename result_of::distance<begin_type, end_type>::type size;
        typedef mpl::true_ is_view;

        typedef typename traits::category_of<begin_type>::type category;

        iterator_range(First const& first, Last const& last)
            : first(convert_iterator<First>::call(first))
            , last(convert_iterator<Last>::call(last)) {}

        begin_type first;
        end_type last;
    };
}}

#endif


