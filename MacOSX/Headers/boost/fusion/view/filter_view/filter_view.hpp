/*=============================================================================
    Copyright (c) 2001-2006 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying 
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(FUSION_SEQUENCE_FILTER_VIEW_HPP)
#define FUSION_SEQUENCE_FILTER_VIEW_HPP

#include <boost/fusion/support/detail/access.hpp>
#include <boost/fusion/support/sequence_base.hpp>
#include <boost/fusion/support/is_view.hpp>
#include <boost/fusion/view/filter_view/filter_view_iterator.hpp>
#include <boost/fusion/view/filter_view/detail/begin_impl.hpp>
#include <boost/fusion/view/filter_view/detail/end_impl.hpp>
#include <boost/fusion/view/filter_view/detail/size_impl.hpp>
#include <boost/fusion/sequence/intrinsic/begin.hpp>
#include <boost/fusion/sequence/intrinsic/end.hpp>
#include <boost/mpl/bool.hpp>

namespace boost { namespace fusion
{
    struct filter_view_tag;
    struct forward_traversal_tag;
    struct fusion_sequence_tag;

    template <typename Sequence, typename Pred>
    struct filter_view : sequence_base<filter_view<Sequence, Pred> >
    {
        typedef filter_view_tag fusion_tag;
        typedef fusion_sequence_tag tag; // this gets picked up by MPL
        typedef forward_traversal_tag category;
        typedef mpl::true_ is_view;

        typedef typename result_of::begin<Sequence>::type first_type;
        typedef typename result_of::end<Sequence>::type last_type;
        typedef Pred pred_type;

        filter_view(Sequence& seq)
            : seq(seq)
        {}

        first_type first() const { return fusion::begin(seq); }
        last_type last() const { return fusion::end(seq); }
        typename mpl::if_<traits::is_view<Sequence>, Sequence, Sequence&>::type seq;
    };
}}

#endif


