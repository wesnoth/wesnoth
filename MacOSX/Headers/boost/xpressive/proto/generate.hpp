#ifndef BOOST_PP_IS_ITERATING
    ///////////////////////////////////////////////////////////////////////////////
    /// \file generate.hpp
    /// Contains definition of generate\<\> class template, which end users can
    /// specialize for generating domain-specific expression wrappers.
    //
    //  Copyright 2008 Eric Niebler. Distributed under the Boost
    //  Software License, Version 1.0. (See accompanying file
    //  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

    #ifndef BOOST_PROTO_GENERATE_HPP_EAN_02_13_2007
    #define BOOST_PROTO_GENERATE_HPP_EAN_02_13_2007

    #include <boost/xpressive/proto/detail/prefix.hpp>
    #include <boost/preprocessor/cat.hpp>
    #include <boost/preprocessor/selection/max.hpp>
    #include <boost/preprocessor/iteration/iterate.hpp>
    #include <boost/preprocessor/repetition/enum.hpp>
    #include <boost/utility/enable_if.hpp>
    #include <boost/xpressive/proto/proto_fwd.hpp>
    #include <boost/xpressive/proto/matches.hpp>
    #include <boost/xpressive/proto/detail/suffix.hpp>

    namespace boost { namespace proto
    {

        namespace detail
        {
            template<typename Domain, typename Expr>
            struct generate_if
              : lazy_enable_if<
                    matches<Expr, typename Domain::proto_grammar>
                  , typename Domain::template apply<Expr>
                >
            {};

            // Optimization, generate fewer templates...
            template<typename Expr>
            struct generate_if<proto::default_domain, Expr>
            {
                typedef Expr type;
            };

            template<typename Expr>
            struct expr_traits;

            template<typename Tag, typename Args, long N>
            struct expr_traits<proto::expr<Tag, Args, N> >
            {
                typedef Tag tag;
                typedef Args args;
                BOOST_STATIC_CONSTANT(long, arity = N);
            };

            template<typename Expr, long Arity = expr_traits<Expr>::arity>
            struct by_value_generator_;

        #define BOOST_PROTO_DEFINE_BY_VALUE_TYPE(Z, N, Expr)\
            typename result_of::unref<typename expr_traits<Expr>::args::BOOST_PP_CAT(arg, N) >::type

        #define BOOST_PROTO_DEFINE_BY_VALUE(Z, N, expr)\
            proto::unref(expr.BOOST_PP_CAT(arg, N))

        #define BOOST_PP_ITERATION_PARAMS_1 (3, (0, BOOST_PROTO_MAX_ARITY, <boost/xpressive/proto/generate.hpp>))
        #include BOOST_PP_ITERATE()

        #undef BOOST_PROTO_DEFINE_BY_VALUE
        #undef BOOST_PROTO_DEFINE_BY_VALUE_TYPE

        }

        namespace generatorns_
        {
            /// \brief A simple generator that passes an expression
            /// through unchanged.
            ///
            /// Generators are intended for use as the first template parameter
            /// to the \c domain\<\> class template and control if and how
            /// expressions within that domain are to be customized.
            /// The \c default_generator makes no modifications to the expressions
            /// passed to it.
            struct default_generator
            {
                template<typename Expr>
                struct apply
                {
                    typedef Expr type;
                };

                /// \param expr A Proto expression
                /// \return expr
                template<typename Expr>
                static Expr const &make(Expr const &expr)
                {
                    return expr;
                }
            };

            /// \brief A generator that wraps expressions passed
            /// to it in the specified extension wrapper.
            ///
            /// Generators are intended for use as the first template parameter
            /// to the \c domain\<\> class template and control if and how
            /// expressions within that domain are to be customized.
            /// \c generator\<\> wraps each expression passed to it in
            /// the \c Extends\<\> wrapper.
            template<template<typename> class Extends>
            struct generator
            {
                template<typename Expr>
                struct apply
                {
                    typedef Extends<Expr> type;
                };

                /// \param expr A Proto expression
                /// \return Extends<Expr>(expr)
                template<typename Expr>
                static Extends<Expr> make(Expr const &expr)
                {
                    return Extends<Expr>(expr);
                }
            };

            /// \brief A generator that wraps expressions passed
            /// to it in the specified extension wrapper and uses
            /// aggregate initialization for the wrapper.
            ///
            /// Generators are intended for use as the first template parameter
            /// to the \c domain\<\> class template and control if and how
            /// expressions within that domain are to be customized.
            /// \c pod_generator\<\> wraps each expression passed to it in
            /// the \c Extends\<\> wrapper, and uses aggregate initialzation
            /// for the wrapped object.
            template<template<typename> class Extends>
            struct pod_generator
            {
                template<typename Expr>
                struct apply
                {
                    typedef Extends<Expr> type;
                };

                /// \param expr The expression to wrap
                /// \return Extends<Expr> that = {expr}; return that;
                template<typename Expr>
                static Extends<Expr> make(Expr const &expr)
                {
                    Extends<Expr> that = {expr};
                    return that;
                }
            };

            /// \brief A composite generator that first replaces
            /// child nodes held by reference with ones held by value
            /// and then forwards the result on to another generator.
            ///
            /// Generators are intended for use as the first template parameter
            /// to the \c domain\<\> class template and control if and how
            /// expressions within that domain are to be customized.
            /// \c by_value_generator\<\> ensures all children nodes are
            /// held by value before forwarding the expression on to
            /// another generator for further processing. The \c Generator
            /// parameter defaults to \c default_generator.
            template<typename Generator BOOST_PROTO_FOR_DOXYGEN_ONLY(= default_generator)>
            struct by_value_generator
            {
                template<typename Expr>
                struct apply
                {
                    typedef
                        typename Generator::template apply<
                            typename detail::by_value_generator_<Expr>::type
                        >::type
                    type;
                };

                /// \param expr The expression to modify.
                /// \return Generator::make(deep_copy(expr))
                template<typename Expr>
                static typename apply<Expr>::type make(Expr const &expr)
                {
                    return Generator::make(detail::by_value_generator_<Expr>::make(expr));
                }
            };
        }

    }}

    #endif // BOOST_PROTO_GENERATE_HPP_EAN_02_13_2007

#else // BOOST_PP_IS_ITERATING

    #define N BOOST_PP_ITERATION()

            template<typename Expr>
            struct by_value_generator_<Expr, N>
            {
                typedef proto::expr<
                    typename expr_traits<Expr>::tag
                  , BOOST_PP_CAT(args, N)<
                        // typename result_of::unref<typename expr_traits<Expr>::args::arg0>::type, ...
                        BOOST_PP_ENUM(BOOST_PP_MAX(N, 1), BOOST_PROTO_DEFINE_BY_VALUE_TYPE, Expr)
                    >
                > type;

                static type const make(Expr const &expr)
                {
                    type that = {
                        // proto::unref(expr.arg0), ...
                        BOOST_PP_ENUM(BOOST_PP_MAX(N, 1), BOOST_PROTO_DEFINE_BY_VALUE, expr)
                    };
                    return that;
                }
            };

    #undef N

#endif
