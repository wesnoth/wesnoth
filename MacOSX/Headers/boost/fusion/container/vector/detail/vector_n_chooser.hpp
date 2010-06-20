/*=============================================================================
    Copyright (c) 2001-2006 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying 
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#ifndef BOOST_PP_IS_ITERATING
#if !defined(FUSION_VECTOR_N_CHOOSER_07072005_1248)
#define FUSION_VECTOR_N_CHOOSER_07072005_1248

#include <boost/fusion/container/vector/limits.hpp>

//  include vector0..N where N is FUSION_MAX_VECTOR_SIZE
#include <boost/fusion/container/vector/vector10.hpp>
#if (FUSION_MAX_VECTOR_SIZE > 10)
#include <boost/fusion/container/vector/vector20.hpp>
#endif
#if (FUSION_MAX_VECTOR_SIZE > 20)
#include <boost/fusion/container/vector/vector30.hpp>
#endif
#if (FUSION_MAX_VECTOR_SIZE > 30)
#include <boost/fusion/container/vector/vector40.hpp>
#endif
#if (FUSION_MAX_VECTOR_SIZE > 40)
#include <boost/fusion/container/vector/vector50.hpp>
#endif

#include <boost/mpl/distance.hpp>
#include <boost/mpl/find.hpp>
#include <boost/mpl/begin_end.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>

namespace boost { namespace fusion
{
    struct void_;
}}

namespace boost { namespace fusion { namespace detail
{
    template <int N>
    struct get_vector_n;

    template <>
    struct get_vector_n<0>
    {
        template <BOOST_PP_ENUM_PARAMS(FUSION_MAX_VECTOR_SIZE, typename T)>
        struct call
        {
            typedef vector0 type;
        };
    };

#define BOOST_PP_FILENAME_1 \
    <boost/fusion/container/vector/detail/vector_n_chooser.hpp>
#define BOOST_PP_ITERATION_LIMITS (1, FUSION_MAX_VECTOR_SIZE)
#include BOOST_PP_ITERATE()

    template <BOOST_PP_ENUM_PARAMS(FUSION_MAX_VECTOR_SIZE, typename T)>
    struct vector_n_chooser
    {
        typedef
            mpl::BOOST_PP_CAT(vector, FUSION_MAX_VECTOR_SIZE)
                <BOOST_PP_ENUM_PARAMS(FUSION_MAX_VECTOR_SIZE, T)>
        input;

        typedef typename mpl::begin<input>::type begin;
        typedef typename mpl::find<input, void_>::type end;
        typedef typename mpl::distance<begin, end>::type size;

        typedef typename get_vector_n<size::value>::template
            call<BOOST_PP_ENUM_PARAMS(FUSION_MAX_VECTOR_SIZE, T)>::type
        type;
    };
}}}

#endif

///////////////////////////////////////////////////////////////////////////////
//
//  Preprocessor vertical repetition code
//
///////////////////////////////////////////////////////////////////////////////
#else // defined(BOOST_PP_IS_ITERATING)

#define N BOOST_PP_ITERATION()

    template <>
    struct get_vector_n<N>
    {
        template <BOOST_PP_ENUM_PARAMS(FUSION_MAX_VECTOR_SIZE, typename T)>
        struct call
        {
            typedef BOOST_PP_CAT(vector, N)<BOOST_PP_ENUM_PARAMS(N, T)> type;
        };
    };

#undef N
#endif // defined(BOOST_PP_IS_ITERATING)
