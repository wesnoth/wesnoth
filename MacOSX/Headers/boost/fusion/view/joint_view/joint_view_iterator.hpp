/*=============================================================================
    Copyright (c) 2001-2006 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(FUSION_JOINT_VIEW_ITERATOR_07162005_0140)
#define FUSION_JOINT_VIEW_ITERATOR_07162005_0140

#include <boost/fusion/support/iterator_base.hpp>
#include <boost/fusion/iterator/equal_to.hpp>
#include <boost/fusion/iterator/mpl/convert_iterator.hpp>
#include <boost/fusion/adapted/mpl/mpl_iterator.hpp>
#include <boost/fusion/view/joint_view/detail/deref_impl.hpp>
#include <boost/fusion/view/joint_view/detail/next_impl.hpp>
#include <boost/fusion/view/joint_view/detail/value_of_impl.hpp>
#include <boost/static_assert.hpp>

namespace boost { namespace fusion
{
    struct joint_view_iterator_tag;
    struct forward_traversal_tag;

    template <typename First, typename Last, typename Concat>
    struct joint_view_iterator
        : iterator_base<joint_view_iterator<First, Last, Concat> >
    {
        typedef convert_iterator<First> first_converter;
        typedef convert_iterator<Last> last_converter;
        typedef convert_iterator<Concat> concat_converter;

        typedef typename first_converter::type first_type;
        typedef typename last_converter::type last_type;
        typedef typename concat_converter::type concat_type;

        typedef joint_view_iterator_tag fusion_tag;
        typedef forward_traversal_tag category;
        BOOST_STATIC_ASSERT((!result_of::equal_to<first_type, last_type>::value));

        joint_view_iterator(First const& first, Concat const& concat)
            : first(first_converter::call(first))
            , concat(concat_converter::call(concat))
        {}

        first_type first;
        concat_type concat;
    };
}}

#endif


