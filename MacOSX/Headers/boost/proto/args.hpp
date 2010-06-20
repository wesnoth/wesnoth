#ifndef BOOST_PP_IS_ITERATING
    ///////////////////////////////////////////////////////////////////////////////
    /// \file args.hpp
    /// Contains definition of args\<\> class template.
    //
    //  Copyright 2008 Eric Niebler. Distributed under the Boost
    //  Software License, Version 1.0. (See accompanying file
    //  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

    #ifndef BOOST_PROTO_ARGS_HPP_EAN_04_01_2005
    #define BOOST_PROTO_ARGS_HPP_EAN_04_01_2005

    #include <boost/proto/detail/prefix.hpp>
    #include <boost/config.hpp>
    #include <boost/detail/workaround.hpp>
    #include <boost/preprocessor/cat.hpp>
    #include <boost/preprocessor/arithmetic/dec.hpp>
    #include <boost/preprocessor/iteration/iterate.hpp>
    #include <boost/preprocessor/repetition/enum_params.hpp>
    #include <boost/preprocessor/repetition/repeat.hpp>
    #include <boost/preprocessor/repetition/repeat_from_to.hpp>
    #include <boost/type_traits/is_function.hpp>
    #include <boost/mpl/if.hpp>
    #include <boost/mpl/void.hpp>
    #include <boost/proto/proto_fwd.hpp>
    #include <boost/proto/detail/suffix.hpp>

    namespace boost { namespace proto
    {
        ////////////////////////////////////////////////////////////////////////////////////////////
        BOOST_PROTO_BEGIN_ADL_NAMESPACE(argsns_)

        #define BOOST_PROTO_DEFINE_CHILD_N(Z, N, DATA)                                              \
            typedef BOOST_PP_CAT(Arg, N) BOOST_PP_CAT(child, N);                                    \
            typedef expr_ref<BOOST_PP_CAT(Arg, N)> BOOST_PP_CAT(child_ref, N);                      \

        #define BOOST_PROTO_DEFINE_VOID_N(z, n, data)                                               \
            typedef mpl::void_ BOOST_PP_CAT(child, n);                                              \
            typedef mpl::void_ BOOST_PP_CAT(child_ref, n);                                          \
            /**/

        /// INTERNAL ONLY
        template<typename Expr>
        struct expr_ref
        {
            typedef typename Expr::proto_base_expr proto_base_expr;
            typedef typename Expr::proto_derived_expr proto_derived_expr;
            typedef typename Expr::proto_tag proto_tag;
            typedef typename Expr::proto_args proto_args;
            typedef typename Expr::proto_arity proto_arity;
            typedef typename Expr::proto_domain proto_domain;
            typedef proto_derived_expr value_type;
            typedef Expr &reference;
            typedef Expr const &const_reference;
        };

        /// INTERNAL ONLY
        template<typename Expr>
        struct expr_ref<Expr &>
        {
            typedef typename Expr::proto_base_expr proto_base_expr;
            typedef typename Expr::proto_derived_expr proto_derived_expr;
            typedef typename Expr::proto_tag proto_tag;
            typedef typename Expr::proto_args proto_args;
            typedef typename Expr::proto_arity proto_arity;
            typedef typename Expr::proto_domain proto_domain;
            typedef proto_derived_expr value_type;
            typedef Expr &reference;
            typedef Expr &const_reference;
        };

        /// INTERNAL ONLY
        template<typename T>
        struct term_ref
        {
            typedef T value_type;
            typedef T &reference;
            typedef T const &const_reference;
        };

        /// INTERNAL ONLY
        template<typename T>
        struct term_ref<T &>
        {
            typedef typename mpl::if_c<is_function<T>::value, T &, T>::type value_type;
            typedef T &reference;
            typedef T &const_reference;
        };

        /// INTERNAL ONLY
        template<typename T>
        struct term_ref<T const &>
        {
            typedef T value_type;
            typedef T const &reference;
            typedef T const &const_reference;
        };

        /// INTERNAL ONLY
        template<typename T, std::size_t N>
        struct term_ref<T (&)[N]>
        {
            typedef T (&value_type)[N];
            typedef T (&reference)[N];
            typedef T (&const_reference)[N];
        };

        /// INTERNAL ONLY
        template<typename T, std::size_t N>
        struct term_ref<T const (&)[N]>
        {
            typedef T const (&value_type)[N];
            typedef T const (&reference)[N];
            typedef T const (&const_reference)[N];
        };

        /// INTERNAL ONLY
        template<typename T, std::size_t N>
        struct term_ref<T[N]>
        {
            typedef T (&value_type)[N];
            typedef T (&reference)[N];
            typedef T const (&const_reference)[N];
        };

        /// INTERNAL ONLY
        template<typename T, std::size_t N>
        struct term_ref<T const[N]>
        {
            typedef T const (&value_type)[N];
            typedef T const (&reference)[N];
            typedef T const (&const_reference)[N];
        };

        /// \brief A type sequence, for use as the 2nd parameter to the \c expr\<\> class template.
        ///
        /// A type sequence, for use as the 2nd parameter to the \c expr\<\> class template.
        /// The types in the sequence correspond to the children of a node in an expression tree.
        template< typename Arg0 >
        struct term
        {
            BOOST_STATIC_CONSTANT(long, arity = 0);
            typedef Arg0 child0;
            typedef term_ref<Arg0> child_ref0;

            #if BOOST_WORKAROUND(BOOST_MSVC, BOOST_TESTED_AT(1500))
            BOOST_PP_REPEAT_FROM_TO(1, BOOST_PROTO_MAX_ARITY, BOOST_PROTO_DEFINE_VOID_N, ~)
            #endif

            /// INTERNAL ONLY
            ///
            typedef Arg0 back_;
        };

        #define BOOST_PP_ITERATION_PARAMS_1 (3, (1, BOOST_PROTO_MAX_ARITY, <boost/proto/args.hpp>))
        #include BOOST_PP_ITERATE()

        #undef BOOST_PROTO_DEFINE_CHILD_N

        BOOST_PROTO_END_ADL_NAMESPACE(argsns_)
        ////////////////////////////////////////////////////////////////////////////////////////////

        #ifndef BOOST_PROTO_BUILDING_DOCS
        using namespace argsns_;
        #endif
    }}
    #endif

#else

    #define N BOOST_PP_ITERATION()

        /// \brief A type sequence, for use as the 2nd parameter to the \c expr\<\> class template.
        ///
        /// A type sequence, for use as the 2nd parameter to the \c expr\<\> class template.
        /// The types in the sequence correspond to the children of a node in an expression tree.
        template< BOOST_PP_ENUM_PARAMS(N, typename Arg) >
        struct BOOST_PP_CAT(list, N)
        {
            BOOST_STATIC_CONSTANT(long, arity = N);
            BOOST_PP_REPEAT(N, BOOST_PROTO_DEFINE_CHILD_N, ~)

            #if BOOST_WORKAROUND(BOOST_MSVC, BOOST_TESTED_AT(1500))
            BOOST_PP_REPEAT_FROM_TO(N, BOOST_PROTO_MAX_ARITY, BOOST_PROTO_DEFINE_VOID_N, ~)
            #endif

            /// INTERNAL ONLY
            ///
            typedef BOOST_PP_CAT(Arg, BOOST_PP_DEC(N)) back_;
        };

    #undef N

#endif
