#ifndef BOOST_PP_IS_ITERATING
    ///////////////////////////////////////////////////////////////////////////////
    /// \file deep_copy.hpp
    /// Replace all nodes stored by reference by nodes stored by value.
    //
    //  Copyright 2008 Eric Niebler. Distributed under the Boost
    //  Software License, Version 1.0. (See accompanying file
    //  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

    #ifndef BOOST_PROTO_DEEP_COPY_HPP_EAN_11_21_2006
    #define BOOST_PROTO_DEEP_COPY_HPP_EAN_11_21_2006

    #include <boost/xpressive/proto/detail/prefix.hpp>
    #include <boost/preprocessor/cat.hpp>
    #include <boost/preprocessor/repetition/enum.hpp>
    #include <boost/preprocessor/iteration/iterate.hpp>
    #include <boost/mpl/if.hpp>
    #include <boost/type_traits/is_function.hpp>
    #include <boost/xpressive/proto/proto_fwd.hpp>
    #include <boost/xpressive/proto/expr.hpp>
    #include <boost/xpressive/proto/detail/suffix.hpp>

    namespace boost { namespace proto
    {
        namespace detail
        {
            template<typename Expr, long Arity = Expr::proto_arity::value>
            struct deep_copy_impl;

            template<typename Expr>
            struct deep_copy_impl<Expr, 0>
            {
                typedef BOOST_PROTO_UNCVREF(typename Expr::proto_arg0) raw_terminal_type;
                // can't store a function type in a terminal.
                typedef
                    typename mpl::if_<
                        is_function<raw_terminal_type>
                      , typename Expr::proto_arg0
                      , raw_terminal_type
                    >::type
                actual_terminal_type;
                typedef typename terminal<actual_terminal_type>::type expr_type;
                typedef typename Expr::proto_domain::template apply<expr_type>::type type;

                template<typename Expr2>
                static type call(Expr2 const &expr)
                {
                    return Expr::proto_domain::make(expr_type::make(proto::arg(expr)));
                }
            };
        }

        namespace result_of
        {
            /// \brief A metafunction for calculating the return type
            /// of \c proto::deep_copy().
            ///
            /// A metafunction for calculating the return type
            /// of \c proto::deep_copy(). The type parameter \c Expr
            /// should be the type of a Proto expression tree.
            /// It should not be a reference type, nor should it
            /// be cv-qualified.
            template<typename Expr>
            struct deep_copy
            {
                typedef typename detail::deep_copy_impl<Expr>::type type;
            };
        }

        namespace functional
        {
            /// \brief A PolymorphicFunctionObject type for deep-copying
            /// Proto expression trees.
            ///
            /// A PolymorphicFunctionObject type for deep-copying
            /// Proto expression trees. When a tree is deep-copied,
            /// all internal nodes and most terminals held by reference
            /// are instead held by value.
            ///
            /// \attention Terminals of reference-to-function type are
            /// left unchanged. Terminals of reference-to-array type are
            /// stored by value, which can cause a large amount of data
            /// to be passed by value and stored on the stack.
            struct deep_copy
            {
                BOOST_PROTO_CALLABLE()

                template<typename Sig>
                struct result;

                template<typename This, typename Expr>
                struct result<This(Expr)>
                {
                    typedef
                        typename result_of::deep_copy<BOOST_PROTO_UNCVREF(Expr)>::type
                    type;
                };

                /// \brief Deep-copies a Proto expression tree, turning all
                /// nodes and terminals held by reference into ones held by
                /// value.
                template<typename Expr>
                typename result_of::deep_copy<Expr>::type
                operator()(Expr const &expr) const
                {
                    return proto::detail::deep_copy_impl<Expr>::call(expr);
                }
            };
        }

        /// \brief A PolymorphicFunctionObject for deep-copying
        /// Proto expression trees.
        ///
        /// A PolymorphicFunctionObject for deep-copying
        /// Proto expression trees. When a tree is deep-copied,
        /// all internal nodes and most terminals held by reference
        /// are instead held by value.
        ///
        /// \attention Terminals of reference-to-array type and of
        /// reference-to-function type are left unchanged.
        ///
        /// \sa proto::functional::deep_copy.
        functional::deep_copy const deep_copy = {};

        namespace detail
        {
        #define BOOST_PROTO_DEFINE_DEEP_COPY_TYPE(z, n, data)\
            typename deep_copy_impl<typename Expr::BOOST_PP_CAT(proto_arg, n)>::type

        #define BOOST_PROTO_DEFINE_DEEP_COPY_FUN(z, n, data)\
            proto::deep_copy(expr.proto_base().BOOST_PP_CAT(arg, n))

        #define BOOST_PP_ITERATION_PARAMS_1 (3, (1, BOOST_PROTO_MAX_ARITY, <boost/xpressive/proto/deep_copy.hpp>))
        #include BOOST_PP_ITERATE()

        #undef BOOST_PROTO_DEFINE_DEEP_COPY_FUN
        #undef BOOST_PROTO_DEFINE_DEEP_COPY_TYPE
        }

    }}

    #endif // BOOST_PROTO_COMPILER_DEEP_COPY_HPP_EAN_11_21_2006

#else

    #define N BOOST_PP_ITERATION()

            template<typename Expr>
            struct deep_copy_impl<Expr, N>
            {
                typedef
                    proto::expr<
                        typename Expr::proto_tag
                      , BOOST_PP_CAT(args, N)<
                            BOOST_PP_ENUM(N, BOOST_PROTO_DEFINE_DEEP_COPY_TYPE, ~)
                        >
                    >
                expr_type;
                
                typedef typename Expr::proto_domain::template apply<expr_type>::type type;

                template<typename Expr2>
                static type call(Expr2 const &expr)
                {
                    expr_type that = {
                        BOOST_PP_ENUM(N, BOOST_PROTO_DEFINE_DEEP_COPY_FUN, ~)
                    };

                    return Expr::proto_domain::make(that);
                }
            };

    #undef N

#endif
