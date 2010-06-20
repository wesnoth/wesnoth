#ifndef BOOST_PP_IS_ITERATING
    ///////////////////////////////////////////////////////////////////////////////
    /// \file fold.hpp
    /// Contains definition of the fold<> and reverse_fold<> transforms.
    //
    //  Copyright 2008 Eric Niebler. Distributed under the Boost
    //  Software License, Version 1.0. (See accompanying file
    //  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

    #ifndef BOOST_PROTO_TRANSFORM_FOLD_HPP_EAN_11_04_2007
    #define BOOST_PROTO_TRANSFORM_FOLD_HPP_EAN_11_04_2007

    #include <boost/xpressive/proto/detail/prefix.hpp>
    #include <boost/version.hpp>
    #include <boost/preprocessor/cat.hpp>
    #include <boost/preprocessor/iteration/iterate.hpp>
    #include <boost/preprocessor/arithmetic/inc.hpp>
    #include <boost/preprocessor/arithmetic/sub.hpp>
    #include <boost/preprocessor/repetition/repeat.hpp>
    #if BOOST_VERSION >= 103500
    #include <boost/fusion/include/fold.hpp>
    #else
    #include <boost/spirit/fusion/algorithm/fold.hpp>
    #endif
    #include <boost/xpressive/proto/proto_fwd.hpp>
    #include <boost/xpressive/proto/fusion.hpp>
    #include <boost/xpressive/proto/traits.hpp>
    #include <boost/xpressive/proto/transform/call.hpp>
    #include <boost/xpressive/proto/detail/suffix.hpp>

    namespace boost { namespace proto
    {
        namespace transform
        {

            namespace detail
            {

                template<typename Transform, typename Visitor>
                struct as_callable
                {
                    as_callable(Visitor &v)
                      : v_(v)
                    {}

                    template<typename Sig>
                    struct result;

                    template<typename This, typename Expr, typename State>
                    struct result<This(Expr, State)>
                    {
                        typedef
                            typename when<_, Transform>::template result<void(
                                BOOST_PROTO_UNCVREF(Expr)
                              , BOOST_PROTO_UNCVREF(State)
                              , Visitor
                            )>::type
                        type;
                    };

                    #if BOOST_VERSION < 103500
                    template<typename Expr, typename State>
                    struct apply : result<void(Expr, State)> {};
                    #endif

                    template<typename Expr, typename State>
                    typename when<_, Transform>::template result<void(Expr, State, Visitor)>::type
                    operator ()(Expr const &expr, State const &state) const
                    {
                        return when<_, Transform>()(expr, state, this->v_);
                    }

                private:
                    Visitor &v_;
                };

                #if BOOST_VERSION < 103500
                template<typename Sequence, typename Void = void>
                struct as_fusion_sequence_type
                {
                    typedef Sequence const type;
                };

                template<typename Sequence>
                Sequence const &as_fusion_sequence(Sequence const &sequence, ...)
                {
                    return sequence;
                }

                template<typename Sequence>
                struct as_fusion_sequence_type<Sequence, typename Sequence::proto_is_expr_>
                {
                    typedef typename Sequence::proto_base_expr const type;
                };

                template<typename Sequence>
                typename Sequence::proto_base_expr const &as_fusion_sequence(Sequence const &sequence, int)
                {
                    return sequence.proto_base();
                }

                #define BOOST_PROTO_AS_FUSION_SEQUENCE_TYPE(X) typename detail::as_fusion_sequence_type<X>::type
                #define BOOST_PROTO_AS_FUSION_SEQUENCE(X) detail::as_fusion_sequence(X, 0)
                #else
                #define BOOST_PROTO_AS_FUSION_SEQUENCE_TYPE(X) X
                #define BOOST_PROTO_AS_FUSION_SEQUENCE(X) X
                #endif

                template<typename Fun, typename Expr, typename State, typename Visitor, long Arity = Expr::proto_arity::value>
                struct fold_impl
                {};

                template<typename Fun, typename Expr, typename State, typename Visitor, long Arity = Expr::proto_arity::value>
                struct reverse_fold_impl
                {};

                #define BOOST_PROTO_ARG_N_TYPE(n)\
                    BOOST_PP_CAT(proto_arg, n)\
                    /**/

                #define BOOST_PROTO_FOLD_STATE_TYPE(z, n, data)\
                    typedef\
                        typename when<_, Fun>::template result<void(\
                            typename Expr::BOOST_PROTO_ARG_N_TYPE(n)::proto_base_expr\
                          , BOOST_PP_CAT(state, n)\
                          , Visitor\
                        )>::type\
                    BOOST_PP_CAT(state, BOOST_PP_INC(n));\
                    /**/

                #define BOOST_PROTO_FOLD_STATE(z, n, data)\
                    BOOST_PP_CAT(state, BOOST_PP_INC(n)) const &BOOST_PP_CAT(s, BOOST_PP_INC(n)) =\
                        when<_, Fun>()(expr.BOOST_PP_CAT(arg, n).proto_base(), BOOST_PP_CAT(s, n), visitor);\
                    /**/

                #define BOOST_PROTO_REVERSE_FOLD_STATE_TYPE(z, n, data)\
                    typedef\
                        typename when<_, Fun>::template result<void(\
                            typename Expr::BOOST_PROTO_ARG_N_TYPE(BOOST_PP_SUB(data, BOOST_PP_INC(n)))::proto_base_expr\
                          , BOOST_PP_CAT(state, BOOST_PP_SUB(data, n))\
                          , Visitor\
                        )>::type\
                    BOOST_PP_CAT(state, BOOST_PP_SUB(data, BOOST_PP_INC(n)));\
                    /**/

                #define BOOST_PROTO_REVERSE_FOLD_STATE(z, n, data)\
                    BOOST_PP_CAT(state, BOOST_PP_SUB(data, BOOST_PP_INC(n))) const &BOOST_PP_CAT(s, BOOST_PP_SUB(data, BOOST_PP_INC(n))) =\
                        when<_, Fun>()(expr.BOOST_PP_CAT(arg, BOOST_PP_SUB(data, BOOST_PP_INC(n))).proto_base(), BOOST_PP_CAT(s, BOOST_PP_SUB(data, n)), visitor);\
                    /**/

                #define BOOST_PP_ITERATION_PARAMS_1 (3, (1, BOOST_PROTO_MAX_ARITY, <boost/xpressive/proto/transform/fold.hpp>))
                #include BOOST_PP_ITERATE()

                #undef BOOST_PROTO_REVERSE_FOLD_STATE
                #undef BOOST_PROTO_REVERSE_FOLD_STATE_TYPE
                #undef BOOST_PROTO_FOLD_STATE
                #undef BOOST_PROTO_FOLD_STATE_TYPE
                #undef BOOST_PROTO_ARG_N_TYPE

            } // namespace detail

            /// \brief A PrimitiveTransform that invokes the <tt>fusion::fold\<\></tt>
            /// algorithm to accumulate 
            template<typename Sequence, typename State0, typename Fun>
            struct fold : proto::callable
            {
                template<typename Sig>
                struct result;

                template<typename This, typename Expr, typename State, typename Visitor>
                struct result<This(Expr, State, Visitor)>
                {
                    /// \brief A Fusion sequence.
                    typedef
                        typename when<_, Sequence>::template result<void(Expr, State, Visitor)>::type
                    sequence;

                    /// \brief An initial state for the fold.
                    typedef
                        typename when<_, State0>::template result<void(Expr, State, Visitor)>::type
                    state0;

                    /// \brief <tt>fun(v)(e,s) == when\<_,Fun\>()(e,s,v)</tt>
                    typedef
                        detail::as_callable<Fun, Visitor>
                    fun;

                    typedef
                        typename fusion::BOOST_PROTO_FUSION_RESULT_OF::fold<
                            BOOST_PROTO_AS_FUSION_SEQUENCE_TYPE(sequence)
                          , state0
                          , fun
                        >::type
                    type;
                };

                /// Let \c seq be <tt>when\<_, Sequence\>()(expr, state, visitor)</tt>, let
                /// \c state0 be <tt>when\<_, State0\>()(expr, state, visitor)</tt>, and
                /// let \c fun(visitor) be an object such that <tt>fun(visitor)(expr, state)</tt>
                /// is equivalent to <tt>when\<_, Fun\>()(expr, state, visitor)</tt>. Then, this
                /// function returns <tt>fusion::fold(seq, state0, fun(visitor))</tt>.
                ///
                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    when<_, Sequence> sequence;
                    detail::as_callable<Fun, Visitor> fun(visitor);
                    return fusion::fold(
                        BOOST_PROTO_AS_FUSION_SEQUENCE(sequence(expr, state, visitor))
                      , when<_, State0>()(expr, state, visitor)
                      , fun
                    );
                }
            };

            /// \brief A PrimitiveTransform that is the same as the
            /// <tt>fold\<\></tt> transform, except that it folds
            /// back-to-front instead of front-to-back. It uses
            /// the \c _reverse callable PolymorphicFunctionObject
            /// to create a <tt>fusion::reverse_view\<\></tt> of the
            /// sequence before invoking <tt>fusion::fold\<\></tt>.
            template<typename Sequence, typename State0, typename Fun>
            struct reverse_fold
              : fold<call<_reverse(Sequence)>, State0, Fun>
            {};

            // This specialization is only for improved compile-time performance
            // in the commom case when the Sequence transform is \c proto::_.
            //
            /// INTERNAL ONLY
            ///
            template<typename State0, typename Fun>
            struct fold<_, State0, Fun> : proto::callable
            {
                template<typename Sig>
                struct result;

                template<typename This, typename Expr, typename State, typename Visitor>
                struct result<This(Expr, State, Visitor)>
                {
                    typedef
                        typename detail::fold_impl<
                            Fun
                          , typename Expr::proto_base_expr
                          , typename when<_, State0>::template result<void(Expr, State, Visitor)>::type
                          , Visitor
                        >::type
                    type;
                };

                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    typedef
                        detail::fold_impl<
                            Fun
                          , typename Expr::proto_base_expr
                          , typename when<_, State0>::template result<void(Expr, State, Visitor)>::type
                          , Visitor
                        >
                    impl;

                    return impl::call(
                        expr.proto_base()
                      , when<_, State0>()(expr, state, visitor)
                      , visitor
                    );
                }
            };

            // This specialization is only for improved compile-time performance
            // in the commom case when the Sequence transform is \c proto::_.
            //
            /// INTERNAL ONLY
            ///
            template<typename State0, typename Fun>
            struct reverse_fold<_, State0, Fun> : proto::callable
            {
                template<typename Sig>
                struct result;

                template<typename This, typename Expr, typename State, typename Visitor>
                struct result<This(Expr, State, Visitor)>
                {
                    typedef
                        typename detail::reverse_fold_impl<
                            Fun
                          , typename Expr::proto_base_expr
                          , typename when<_, State0>::template result<void(Expr, State, Visitor)>::type
                          , Visitor
                        >::type
                    type;
                };

                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    typedef
                        detail::reverse_fold_impl<
                            Fun
                          , typename Expr::proto_base_expr
                          , typename when<_, State0>::template result<void(Expr, State, Visitor)>::type
                          , Visitor
                        >
                    impl;

                    return impl::call(
                        expr.proto_base()
                      , when<_, State0>()(expr, state, visitor)
                      , visitor
                    );
                }
            };
        }

        /// INTERNAL ONLY
        ///
        template<typename Sequence, typename State, typename Fun>
        struct is_callable<transform::fold<Sequence, State, Fun> >
          : mpl::true_
        {};

        /// INTERNAL ONLY
        ///
        template<typename Sequence, typename State, typename Fun>
        struct is_callable<transform::reverse_fold<Sequence, State, Fun> >
          : mpl::true_
        {};

    }}

    #endif

#else

    #define N BOOST_PP_ITERATION()

            template<typename Fun, typename Expr, typename state0, typename Visitor>
            struct fold_impl<Fun, Expr, state0, Visitor, N>
            {
                BOOST_PP_REPEAT(N, BOOST_PROTO_FOLD_STATE_TYPE, N)
                typedef BOOST_PP_CAT(state, N) type;

                static type call(Expr const &expr, state0 const &s0, Visitor &visitor)
                {
                    BOOST_PP_REPEAT(N, BOOST_PROTO_FOLD_STATE, N)
                    return BOOST_PP_CAT(s, N);
                }
            };

            template<typename Fun, typename Expr, typename BOOST_PP_CAT(state, N), typename Visitor>
            struct reverse_fold_impl<Fun, Expr, BOOST_PP_CAT(state, N), Visitor, N>
            {
                BOOST_PP_REPEAT(N, BOOST_PROTO_REVERSE_FOLD_STATE_TYPE, N)
                typedef state0 type;

                static type call(Expr const &expr, BOOST_PP_CAT(state, N) const &BOOST_PP_CAT(s, N), Visitor &visitor)
                {
                    BOOST_PP_REPEAT(N, BOOST_PROTO_REVERSE_FOLD_STATE, N)
                    return s0;
                }
            };

    #undef N

#endif
