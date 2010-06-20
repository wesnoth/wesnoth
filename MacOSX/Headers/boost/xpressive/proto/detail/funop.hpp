///////////////////////////////////////////////////////////////////////////////
// funop.hpp
// Contains definition of funop[n]\<\> class template.
//
//  Copyright 2008 Eric Niebler. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PP_IS_ITERATING
#error Do not include this file directly
#endif

    /// \brief A helper metafunction for computing the
    /// return type of \c proto::expr\<\>::operator().
    template<typename Expr BOOST_PP_ENUM_TRAILING_PARAMS(BOOST_PP_ITERATION(), typename A)>
    struct BOOST_PP_CAT(funop, BOOST_PP_ITERATION())
    {
        typedef proto::expr<
            tag::function
          , BOOST_PP_CAT(args, BOOST_PP_INC(BOOST_PP_ITERATION()))<
                ref_<Expr>
                BOOST_PP_ENUM_TRAILING_BINARY_PARAMS(
                    BOOST_PP_ITERATION()
                  , typename result_of::as_arg<A
                  , >::type BOOST_PP_INTERCEPT
                )
            >
        > type;

        static type const call(
            Expr &expr
            BOOST_PP_ENUM_TRAILING_BINARY_PARAMS(BOOST_PP_ITERATION(), A, &a)
        )
        {
            type that = {
                {expr}
                BOOST_PP_ENUM_TRAILING(BOOST_PP_ITERATION(), BOOST_PROTO_AS_OP, _)
            };
            return that;
        }
    };

    /// \brief A helper metafunction for computing the
    /// return type of \c proto::expr\<\>::operator().
    template<typename Expr BOOST_PP_ENUM_TRAILING_PARAMS(BOOST_PP_ITERATION(), typename A), typename This>
    struct funop<Expr(BOOST_PP_ENUM_PARAMS(BOOST_PP_ITERATION(), A)), This>
      : BOOST_PP_CAT(funop, BOOST_PP_ITERATION())<
            This
            BOOST_PP_ENUM_TRAILING_BINARY_PARAMS(
                BOOST_PP_ITERATION()
              , typename remove_reference<A
              , >::type BOOST_PP_INTERCEPT
            )
        >
    {};

    /// \brief A helper metafunction for computing the
    /// return type of \c proto::expr\<\>::operator().
    template<typename Expr BOOST_PP_ENUM_TRAILING_PARAMS(BOOST_PP_ITERATION(), typename A), typename This>
    struct funop<Expr const(BOOST_PP_ENUM_PARAMS(BOOST_PP_ITERATION(), A)), This>
      : BOOST_PP_CAT(funop, BOOST_PP_ITERATION())<
            This const
            BOOST_PP_ENUM_TRAILING_BINARY_PARAMS(
                BOOST_PP_ITERATION()
              , typename remove_reference<A
              , >::type BOOST_PP_INTERCEPT
            )
        >
    {};
