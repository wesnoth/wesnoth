/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying 
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#ifndef BOOST_PP_IS_ITERATING

#include <boost/preprocessor/iterate.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>

#define BOOST_PP_FILENAME_1 \
    <boost/spirit/home/support/nonterminal/detail/nonterminal_fcall.hpp>
#define BOOST_PP_ITERATION_LIMITS (1, SPIRIT_ARG_LIMIT)
#include BOOST_PP_ITERATE()

///////////////////////////////////////////////////////////////////////////////
//
//  Preprocessor vertical repetition code
//
///////////////////////////////////////////////////////////////////////////////
#else // defined(BOOST_PP_IS_ITERATING)

#define N BOOST_PP_ITERATION()

    template <BOOST_PP_ENUM_PARAMS(N, typename A)>
    typename lazy_enable_if_c<
        (mpl::size<param_types>::value == N)
      , make_nonterminal_holder<
            parameterized_nonterminal<
                Derived
              , fusion::vector<BOOST_PP_ENUM_PARAMS(N, A)>
            >
          , Derived
        >
    >::type
    operator()(BOOST_PP_ENUM_BINARY_PARAMS(N, A, const& f)) const
    {
        typename
            make_nonterminal_holder<
                parameterized_nonterminal<
                    Derived
                  , fusion::vector<BOOST_PP_ENUM_PARAMS(N, A)>
                >
              , Derived
            >::type
        result =
        {{
            static_cast<Derived const*>(this)
          , fusion::vector<BOOST_PP_ENUM_PARAMS(N, A)>(
                BOOST_PP_ENUM_PARAMS(N, f))
        }};
        return result;
    }

#undef N
#endif // defined(BOOST_PP_IS_ITERATING)


