#ifndef BOOST_PP_IS_ITERATING
    ///////////////////////////////////////////////////////////////////////////////
    /// \file callable.hpp
    /// Definintion of callable_context\<\>, an evaluation context for
    /// proto::eval() that explodes each node and calls the derived context
    /// type with the expressions constituents. If the derived context doesn't
    /// have an overload that handles this node, fall back to some other
    /// context.
    //
    //  Copyright 2008 Eric Niebler. Distributed under the Boost
    //  Software License, Version 1.0. (See accompanying file
    //  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

    #ifndef BOOST_PROTO_CONTEXT_CALLABLE_HPP_EAN_06_23_2007
    #define BOOST_PROTO_CONTEXT_CALLABLE_HPP_EAN_06_23_2007

    #include <boost/xpressive/proto/detail/prefix.hpp> // must be first include
    #include <boost/config.hpp>
    #include <boost/detail/workaround.hpp>
    #include <boost/preprocessor/cat.hpp>
    #include <boost/preprocessor/iteration/iterate.hpp>
    #include <boost/preprocessor/facilities/intercept.hpp>
    #include <boost/preprocessor/repetition/repeat.hpp>
    #include <boost/preprocessor/repetition/enum_params.hpp>
    #include <boost/preprocessor/repetition/enum_trailing.hpp>
    #include <boost/preprocessor/repetition/enum_trailing_params.hpp>
    #include <boost/preprocessor/arithmetic/inc.hpp>
    #include <boost/preprocessor/selection/max.hpp>
    #include <boost/mpl/if.hpp>
    #include <boost/mpl/bool.hpp>
    #include <boost/utility/result_of.hpp>
    #include <boost/type_traits/remove_cv.hpp>
    #include <boost/xpressive/proto/proto_fwd.hpp>
    #include <boost/xpressive/proto/traits.hpp> // for arg_c
    #include <boost/xpressive/proto/detail/suffix.hpp> // must be last include

    namespace boost { namespace proto
    {
        namespace detail
        {
            struct private_type_
            {
                private_type_ const &operator,(int) const;
            };

            template<typename T>
            yes_type check_is_expr_handled(T const &);

            no_type check_is_expr_handled(private_type_ const &);

            template<typename Context, long Arity>
            struct callable_context_wrapper;

            template<typename Expr, typename Context, long Arity = Expr::proto_arity::value>
            struct is_expr_handled;
        }

        namespace context
        {
            /// \brief A BinaryFunction that accepts a Proto expression and a
            /// callable context and calls the context with the expression tag
            /// and children as arguments, effectively fanning the expression
            /// out.
            ///
            /// <tt>callable_eval\<\></tt> requires that \c Context is a
            /// PolymorphicFunctionObject that can be invoked with \c Expr's
            /// tag and children as expressions, as follows:
            ///
            /// \code
            /// context(Expr::proto_tag(), arg_c<0>(expr), arg_c<1>(expr), ...)
            /// \endcode
            template<
                typename Expr
              , typename Context
              , long Arity          BOOST_PROTO_FOR_DOXYGEN_ONLY(= Expr::proto_arity::value)
            >
            struct callable_eval
            {};

            /// \brief An evaluation context adaptor that makes authoring a
            /// context a simple matter of writing function overloads, rather
            /// then writing template specializations.
            ///
            /// <tt>callable_context\<\></tt> is a base class that implements
            /// the context protocol by passing fanned-out expression nodes to
            /// the derived context, making it easy to customize the handling
            /// of expression types by writing function overloads. Only those
            /// expression types needing special handling require explicit
            /// handling. All others are dispatched to a user-specified
            /// default context, \c DefaultCtx.
            ///
            /// <tt>callable_context\<\></tt> is defined simply as:
            ///
            /// \code
            /// template<typename Context, typename DefaultCtx = default_context>
            /// struct callable_context
            /// {
            ///    template<typename Expr, typename ThisContext = Context>
            ///     struct eval
            ///       : mpl::if_<
            ///             is_expr_handled_<Expr, Context> // For exposition
            ///           , callable_eval<Expr, ThisContext>
            ///           , typename DefaultCtx::template eval<Expr, Context>
            ///         >::type
            ///     {};
            /// };
            /// \endcode
            ///
            /// The Boolean metafunction <tt>is_expr_handled_\<\></tt> uses
            /// metaprogramming tricks to determine whether \c Context has
            /// an overloaded function call operator that accepts the
            /// fanned-out constituents of an expression of type \c Expr.
            /// If so, the handling of the expression is dispatched to
            /// <tt>callable_eval\<\></tt>. If not, it is dispatched to
            /// the user-specified \c DefaultCtx.
            ///
            /// Below is an example of how to use <tt>callable_context\<\></tt>:
            ///
            /// \code
            /// // An evaluation context that increments all
            /// // integer terminals in-place.
            /// struct increment_ints
            ///  : callable_context<
            ///         increment_ints const    // derived context
            ///       , null_context const      // fall-back context
            ///     >
            /// {
            ///     typedef void result_type;
            ///
            ///     // Handle int terminals here:
            ///     void operator()(proto::tag::terminal, int &i) const
            ///     {
            ///        ++i;
            ///     }
            /// };
            /// \endcode
            ///
            /// With \c increment_ints, we can do the following:
            ///
            /// \code
            /// literal<int> i = 0, j = 10;
            /// proto::eval( i - j * 3.14, increment_ints() );
            ///
            /// assert( i.get() == 1 && j.get() == 11 );
            /// \endcode
            template<
                typename Context
              , typename DefaultCtx BOOST_PROTO_FOR_DOXYGEN_ONLY(= default_context)
            >
            struct callable_context
            {
                /// A BinaryFunction that accepts an \c Expr and a
                /// \c Context, and either fans out the expression and passes
                /// it to the context, or else hands off the expression to
                /// \c DefaultCtx.
                ///
                /// If \c Context is a PolymorphicFunctionObject such that
                /// it can be invoked with the tag and children of \c Expr,
                /// as <tt>ctx(Expr::proto_tag(), arg_c\<0\>(expr), arg_c\<1\>(expr)...)</tt>,
                /// then <tt>eval\<Expr, ThisContext\></tt> inherits from
                /// <tt>callable_eval\<Expr, ThisContext\></tt>. Otherwise,
                /// <tt>eval\<Expr, ThisContext\></tt> inherits from
                /// <tt>DefaultCtx::eval\<Expr, Context\></tt>.
                template<typename Expr, typename ThisContext = Context>
                struct eval
                  : mpl::if_<
                        detail::is_expr_handled<Expr, Context>
                      , callable_eval<Expr, ThisContext>
                      , typename DefaultCtx::template eval<Expr, Context>
                    >::type
                {};
            };
        }

    #define BOOST_PROTO_ARG_N_TYPE(Z, N, Expr)                                                      \
        typedef typename proto::result_of::arg_c<Expr, N>::const_reference BOOST_PP_CAT(arg, N);    \
        /**/

    #define BOOST_PROTO_ARG_N(Z, N, expr)                                                           \
        proto::arg_c<N>(expr)                                                                       \
        /**/

    #define BOOST_PP_ITERATION_PARAMS_1                                                             \
        (3, (0, BOOST_PROTO_MAX_ARITY, <boost/xpressive/proto/context/callable.hpp>))               \
        /**/

    #include BOOST_PP_ITERATE()

    #undef BOOST_PROTO_ARG_N_TYPE
    #undef BOOST_PROTO_ARG_N

    }}

    #endif

#else

    #define N BOOST_PP_ITERATION()
    #define ARG_COUNT BOOST_PP_MAX(1, N)

        namespace detail
        {
            #if N > 0
            template<typename Context>
            struct callable_context_wrapper<Context, N>
              : remove_cv<Context>::type
            {
                callable_context_wrapper();
                typedef
                    private_type_ const &fun_type(
                        BOOST_PP_ENUM_PARAMS(
                            BOOST_PP_INC(ARG_COUNT)
                          , detail::dont_care BOOST_PP_INTERCEPT
                        )
                    );
                operator fun_type *() const;
            };
            #endif

            template<typename Expr, typename Context>
            struct is_expr_handled<Expr, Context, N>
            {
                static callable_context_wrapper<Context, ARG_COUNT> &sctx_;
                static Expr &sexpr_;
                static typename Expr::proto_tag &stag_;

                BOOST_STATIC_CONSTANT(bool, value =
                (
                    sizeof(yes_type) ==
                    sizeof(
                        detail::check_is_expr_handled(
                            (sctx_(
                                stag_
                                BOOST_PP_ENUM_TRAILING(ARG_COUNT, BOOST_PROTO_ARG_N, sexpr_)
                            ), 0)
                        )
                )));

                typedef mpl::bool_<value> type;
            };
        }

        namespace context
        {
            /// \brief A BinaryFunction that accepts a Proto expression and a
            /// callable context and calls the context with the expression tag
            /// and children as arguments, effectively fanning the expression
            /// out.
            ///
            /// <tt>callable_eval\<\></tt> requires that \c Context is a
            /// PolymorphicFunctionObject that can be invoked with \c Expr's
            /// tag and children as expressions, as follows:
            ///
            /// \code
            /// context(Expr::proto_tag(), arg_c\<0\>(expr), arg_c\<1\>(expr), ...)
            /// \endcode
            template<typename Expr, typename Context>
            struct callable_eval<Expr, Context, N>
            {
                BOOST_PP_REPEAT(ARG_COUNT, BOOST_PROTO_ARG_N_TYPE, Expr)

                typedef
                    typename boost::result_of<
                        Context(
                            typename Expr::proto_tag
                            BOOST_PP_ENUM_TRAILING_PARAMS(ARG_COUNT, arg)
                        )
                    >::type
                result_type;

                /// \param expr The current expression
                /// \param context The callable evaluation context
                /// \return <tt>context(Expr::proto_tag(), arg_c\<0\>(expr), arg_c\<1\>(expr), ...)</tt>
                result_type operator ()(Expr &expr, Context &context) const
                {
                    return context(
                        typename Expr::proto_tag()
                        BOOST_PP_ENUM_TRAILING(ARG_COUNT, BOOST_PROTO_ARG_N, expr)
                    );
                }
            };
        }

    #undef N
    #undef ARG_COUNT

#endif
