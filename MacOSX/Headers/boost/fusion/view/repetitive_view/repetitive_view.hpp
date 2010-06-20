/*=============================================================================
    Copyright (c) 2007 Tobias Schwinger

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if !defined(BOOST_FUSION_REPETITIVE_REPETITIVE_VIEW_VIEW_HPP_INCLUDED)
#define BOOST_FUSION_REPETITIVE_VIEW_REPETITIVE_VIEW_HPP_INCLUDED

#include <boost/type_traits/remove_reference.hpp>
#include <boost/mpl/if.hpp>

#include <boost/fusion/support/is_view.hpp>
#include <boost/fusion/support/category_of.hpp>

#include <boost/fusion/view/repetitive_view/detail/begin_impl.hpp>
#include <boost/fusion/view/repetitive_view/detail/end_impl.hpp>


namespace boost { namespace fusion
{
    struct repetitive_view_tag;
    struct fusion_sequence_tag;

    template<typename Sequence> struct repetitive_view 
        : sequence_base< repetitive_view<Sequence> >
    {
        typedef repetitive_view_tag fusion_tag;
        typedef fusion_sequence_tag tag; // this gets picked up by MPL
        typedef mpl::true_ is_view;

        typedef single_pass_traversal_tag category;

        typedef typename boost::remove_reference<Sequence>::type sequence_type;
        typedef typename 
            mpl::if_<traits::is_view<Sequence>, Sequence, sequence_type&>::type
        stored_seq_type;

        repetitive_view(Sequence& seq)
            : seq(seq) {}

        stored_seq_type seq;
    };

}}

#endif
