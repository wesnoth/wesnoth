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

    #include <boost/proto/detail/prefix.hpp>
    #include <boost/preprocessor/cat.hpp>
    #include <boost/preprocessor/iteration/iterate.hpp>
    #include <boost/preprocessor/repetition/enum.hpp>
    #include <boost/utility/enable_if.hpp>
    #include <boost/proto/proto_fwd.hpp>
    #include <boost/proto/matches.hpp>
    #include <boost/proto/detail/suffix.hpp>

    namespace boost { namespace proto
    {

        namespace detail
        {
            template<typename Domain, typename Expr>
            struct generate_if
              : lazy_enable_if<
                    matches<Expr, typename Domain::proto_grammar>
                  , typename Domain::template result<void(Expr)>
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

        #define BOOST_PROTO_DEFINE_BY_VALUE_TYPE(Z, N, Expr)                                        \
            typename uncvref<typename expr_traits<Expr>::args::BOOST_PP_CAT(child, N)>::type        \
            /**/

        #define BOOST_PROTO_DEFINE_BY_VALUE(Z, N, expr)                                             \
            expr.BOOST_PP_CAT(child, N)                                                             \
            /**/

            template<typename Expr>
            struct by_value_generator_<Expr, 0>
            {
                typedef
                    proto::expr<
                        typename expr_traits<Expr>::tag
                      , term<typename expr_traits<Expr>::args::child_ref0::value_type>
                    >
                type;

                static type const make(Expr const &expr)
                {
                    type that = {expr.child0};
                    return that;
                }
            };

        #define BOOST_PP_ITERATION_PARAMS_1 (3, (1, BOOST_PROTO_MAX_ARITY, <boost/proto/generate.hpp>))
        #include BOOST_PP_ITERATE()

        #undef BOOST_PROTO_DEFINE_BY_VALUE
        #undef BOOST_PROTO_DEFINE_BY_VALUE_TYPE

        }

        BOOST_PROTO_BEGIN_ADL_NAMESPACE(generatorns_)

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
            BOOST_PROTO_CALLABLE()

            template<typename Sig>
            struct result;

            template<typename This, typename Expr>
            struct result<This(Expr)>
            {
                typedef Expr type;
            };

            /// \param expr A Proto expression
            /// \return expr
            template<typename Expr>
            Expr const &operator ()(Expr const &expr) const
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
            BOOST_PROTO_CALLABLE()

            template<typename Sig>
            struct result;

            template<typename This, typename Expr>
            struct result<This(Expr)>
            {
                typedef Extends<Expr> type;
            };

            template<typename This, typename Expr>
            struct result<This(Expr &)>
            {
                typedef Extends<Expr> type;
            };

            template<typename This, typename Expr>
            struct result<This(Expr const &)>
            {
                typedef Extends<Expr> type;
            };

            /// \param expr A Proto expression
            /// \return Extends<Expr>(expr)
            template<typename Expr>
            Extends<Expr> operator ()(Expr const &expr) const
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
            BOOST_PROTO_CALLABLE()

            template<typename Sig>
            struct result;

            template<typename This, typename Expr>
            struct result<This(Expr)>
            {
                typedef Extends<Expr> type;
            };

            template<typename This, typename Expr>
            struct result<This(Expr &)>
            {
                typedef Extends<Expr> type;
            };

            template<typename This, typename Expr>
            struct result<This(Expr const &)>
            {
                typedef Extends<Expr> type;
            };

            /// \param expr The expression to wrap
            /// \return <tt>Extends\<Expr\> that = {expr}; return that;</tt>
            template<typename Expr>
            Extends<Expr> operator ()(Expr const &expr) const
            {
                Extends<Expr> that = {expr};
                return that;
            }
        };

        /// \brief A generator that replaces child nodes held by
        /// reference with ones held by value. Use with
        /// \c compose_generators to forward that result to another
        /// generator.
        ///
        /// Generators are intended for use as the first template parameter
        /// to the \c domain\<\> class template and control if and how
        /// expressions within that domain are to be customized.
        /// \c by_value_generator ensures all child nodes are
        /// held by value. This generator is typically composed with a
        /// second generator for further processing, as
        /// <tt>compose_generators\<by_value_generator, MyGenerator\></tt>.
        struct by_value_generator
        {
            BOOST_PROTO_CALLABLE()

            template<typename Sig>
            struct result;

            template<typename This, typename Expr>
            struct result<This(Expr)>
            {
                typedef
                    typename detail::by_value_generator_<Expr>::type
                type;
            };

            template<typename This, typename Expr>
            struct result<This(Expr &)>
            {
                typedef
                    typename detail::by_value_generator_<Expr>::type
                type;
            };

            template<typename This, typename Expr>
            struct result<This(Expr const &)>
            {
                typedef
                    typename detail::by_value_generator_<Expr>::type
                type;
            };

            /// \param expr The expression to modify.
            /// \return <tt>deep_copy(expr)</tt>
            template<typename Expr>
            typename result<void(Expr)>::type operator ()(Expr const &expr) const
            {
                return detail::by_value_generator_<Expr>::make(expr);
            }
        };

        /// \brief A composite generator that first applies one
        /// transform to an expression and then forwards the result
        /// on to another generator for further transformation.
        ///
        /// Generators are intended for use as the first template parameter
        /// to the \c domain\<\> class template and control if and how
        /// expressions within that domain are to be customized.
        /// \c compose_generators\<\> is a composite generator that first
        /// applies one transform to an expression and then forwards the
        /// result on to another generator for further transformation.
        template<typename First, typename Second>
        struct compose_generators
        {
            BOOST_PROTO_CALLABLE()

            template<typename Sig>
            struct result;

            template<typename This, typename Expr>
            struct result<This(Expr)>
            {
                typedef
                    typename Second::template result<
                        void(typename First::template result<void(Expr)>::type)
                    >::type
                type;
            };

            template<typename This, typename Expr>
            struct result<This(Expr &)>
            {
                typedef
                    typename Second::template result<
                        void(typename First::template result<void(Expr)>::type)
                    >::type
                type;
            };

            template<typename This, typename Expr>
            struct result<This(Expr const &)>
            {
                typedef
                    typename Second::template result<
                        void(typename First::template result<void(Expr)>::type)
                    >::type
                type;
            };

            /// \param expr The expression to modify.
            /// \return Second()(First()(expr))
            template<typename Expr>
            typename result<void(Expr)>::type operator ()(Expr const &expr) const
            {
                return Second()(First()(expr));
            }
        };

        BOOST_PROTO_END_ADL_NAMESPACE(generatorns_)

        /// INTERNAL ONLY
        template<>
        struct is_callable<default_generator>
          : mpl::true_
        {};

        /// INTERNAL ONLY
        template<template<typename> class Extends>
        struct is_callable<generator<Extends> >
          : mpl::true_
        {};

        /// INTERNAL ONLY
        template<template<typename> class Extends>
        struct is_callable<pod_generator<Extends> >
          : mpl::true_
        {};

        /// INTERNAL ONLY
        template<>
        struct is_callable<by_value_generator>
          : mpl::true_
        {};

        /// INTERNAL ONLY
        template<typename First, typename Second>
        struct is_callable<compose_generators<First, Second> >
          : mpl::true_
        {};

    }}

    #endif // BOOST_PROTO_GENERATE_HPP_EAN_02_13_2007

#else // BOOST_PP_IS_ITERATING

    #define N BOOST_PP_ITERATION()

            template<typename Expr>
            struct by_value_generator_<Expr, N>
            {
                typedef
                    proto::expr<
                        typename expr_traits<Expr>::tag
                      , BOOST_PP_CAT(list, N)<
                            // typename expr_traits<Expr>::args::child_ref0::proto_derived_expr, ...
                            BOOST_PP_ENUM(N, BOOST_PROTO_DEFINE_BY_VALUE_TYPE, Expr)
                        >
                    >
                type;

                static type const make(Expr const &expr)
                {
                    type that = {
                        // expr.child0, ...
                        BOOST_PP_ENUM(N, BOOST_PROTO_DEFINE_BY_VALUE, expr)
                    };
                    return that;
                }
            };

    #undef N

#endif
