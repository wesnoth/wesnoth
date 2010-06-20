/*=============================================================================
    Copyright (c) 2001-2006 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying 
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(FUSION_FIND_05052005_1107)
#define FUSION_FIND_05052005_1107

#include <boost/fusion/algorithm/query/detail/find_if.hpp>
#include <boost/fusion/algorithm/query/detail/assoc_find.hpp>
#include <boost/fusion/sequence/intrinsic/begin.hpp>
#include <boost/fusion/sequence/intrinsic/end.hpp>
#include <boost/fusion/support/category_of.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/is_const.hpp>
#include <boost/utility/enable_if.hpp>

namespace boost { namespace fusion
{
    struct associative_sequence_tag;

    namespace result_of
    {
        template <
            typename Sequence
          , typename T
          , bool is_associative_sequence = traits::is_associative<Sequence>::value >
        struct find;

        template <typename Sequence, typename T>
        struct find<Sequence, T, false>
        {
            typedef
                detail::static_seq_find_if<
                    typename result_of::begin<Sequence>::type
                  , typename result_of::end<Sequence>::type
                  , is_same<mpl::_, T>
                >
            filter;

            typedef typename filter::type type;
        };

        template <typename Sequence, typename T>
        struct find<Sequence, T, true>
        {
            typedef detail::assoc_find<Sequence, T> filter;
            typedef typename filter::type type;
        };
    }

    template <typename T, typename Sequence>
    inline typename 
        lazy_disable_if<
            is_const<Sequence>
          , result_of::find<Sequence, T>
        >::type const
    find(Sequence& seq)
    {
        typedef typename result_of::find<Sequence, T>::filter filter;
        return filter::call(seq);
    }

    template <typename T, typename Sequence>
    inline typename result_of::find<Sequence const, T>::type const
    find(Sequence const& seq)
    {
        typedef typename result_of::find<Sequence const, T>::filter filter;
        return filter::call(seq);
    }
}}

#endif

