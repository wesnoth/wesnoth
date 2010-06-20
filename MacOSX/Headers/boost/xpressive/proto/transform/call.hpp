#ifndef BOOST_PP_IS_ITERATING
    ///////////////////////////////////////////////////////////////////////////////
    /// \file call.hpp
    /// Contains definition of the call<> transform.
    //
    //  Copyright 2008 Eric Niebler. Distributed under the Boost
    //  Software License, Version 1.0. (See accompanying file
    //  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

    #ifndef BOOST_PROTO_TRANSFORM_CALL_HPP_EAN_11_02_2007
    #define BOOST_PROTO_TRANSFORM_CALL_HPP_EAN_11_02_2007

    #include <boost/xpressive/proto/detail/prefix.hpp>
    #include <boost/preprocessor/cat.hpp>
    #include <boost/preprocessor/iteration/iterate.hpp>
    #include <boost/preprocessor/repetition/enum.hpp>
    #include <boost/preprocessor/repetition/repeat.hpp>
    #include <boost/preprocessor/repetition/enum_params.hpp>
    #include <boost/preprocessor/repetition/enum_trailing_params.hpp>
    #include <boost/utility/result_of.hpp>
    #include <boost/xpressive/proto/proto_fwd.hpp>
    #include <boost/xpressive/proto/traits.hpp>
    #include <boost/xpressive/proto/detail/dont_care.hpp>
    #include <boost/xpressive/proto/detail/as_lvalue.hpp>
    #include <boost/xpressive/proto/detail/suffix.hpp>

    namespace boost { namespace proto
    {

        namespace transform
        {
            namespace detail
            {
                using proto::detail::uncv;
                using proto::detail::as_lvalue;
                using proto::detail::dont_care;
                typedef char (&yes_type)[2];
                typedef char no_type;

                struct private_type_
                {
                    private_type_ const &operator ,(int) const;
                };

                template<typename T>
                yes_type check_fun_arity(T const &);

                no_type check_fun_arity(private_type_ const &);

                template<typename Fun>
                struct callable0_wrap : Fun
                {
                    callable0_wrap();
                    typedef private_type_ const &(*pfun0)();
                    operator pfun0() const;
                };

                template<typename Fun>
                struct callable1_wrap : Fun
                {
                    callable1_wrap();
                    typedef private_type_ const &(*pfun1)(dont_care);
                    operator pfun1() const;
                };

                template<typename Fun>
                struct callable2_wrap : Fun
                {
                    callable2_wrap();
                    typedef private_type_ const &(*pfun2)(dont_care, dont_care);
                    operator pfun2() const;
                };

                template<typename Fun>
                struct arity0
                {
                    static callable0_wrap<Fun> &fun;

                    static int const value =
                        sizeof(yes_type) == sizeof(check_fun_arity((fun(), 0)))
                      ? 0
                      : 3;
                };

                template<typename Fun, typename A0>
                struct arity1
                {
                    static callable1_wrap<Fun> &fun;
                    static A0 &a0;

                    static int const value =
                        sizeof(yes_type) == sizeof(check_fun_arity((fun(a0), 0)))
                      ? 1
                      : 3;
                };

                template<typename Fun, typename A0, typename A1>
                struct arity2
                {
                    static callable2_wrap<Fun> &fun;
                    static A0 &a0;
                    static A1 &a1;

                    static int const value =
                        sizeof(yes_type) == sizeof(check_fun_arity((fun(a0, a1), 0)))
                      ? 2
                      : 3;
                };

                template<typename Fun, typename Expr, typename State, typename Visitor>
                struct call3
                {
                    typedef typename boost::result_of<Fun(Expr, State, Visitor)>::type type;

                    template<typename Expr2, typename State2, typename Visitor2>
                    static type call(Expr2 &expr, State2 &state, Visitor2 &visitor)
                    {
                        Fun f;
                        return f(expr, state, visitor);
                    }
                };

                template<typename Fun, typename Expr, typename State, typename Visitor
                  , int Arity = arity0<Fun>::value>
                struct call0
                  : call3<Fun, Expr, State, Visitor>
                {};

                template<typename Fun, typename Expr, typename State, typename Visitor>
                struct call0<Fun, Expr, State, Visitor, 0>
                {
                    typedef typename boost::result_of<Fun()>::type type;

                    template<typename Expr2, typename State2, typename Visitor2>
                    static type call(Expr2 &, State2 &, Visitor2 &)
                    {
                        Fun f;
                        return f();
                    }
                };

                template<typename Fun, typename Expr, typename State, typename Visitor
                  , int Arity = arity1<Fun, Expr>::value>
                struct call1
                  : call3<Fun, Expr, State, Visitor>
                {};

                template<typename Fun, typename Expr, typename State, typename Visitor>
                struct call1<Fun, Expr, State, Visitor, 1>
                {
                    typedef typename boost::result_of<Fun(Expr)>::type type;

                    template<typename Expr2, typename State2, typename Visitor2>
                    static type call(Expr2 &expr, State2 &, Visitor2 &)
                    {
                        Fun f;
                        return f(expr);
                    }
                };

                template<typename Fun, typename Expr, typename State, typename Visitor
                  , int Arity = arity2<Fun, Expr, State>::value>
                struct call2
                  : call3<Fun, Expr, State, Visitor>
                {};

                template<typename Fun, typename Expr, typename State, typename Visitor>
                struct call2<Fun, Expr, State, Visitor, 2>
                {
                    typedef typename boost::result_of<Fun(Expr, State)>::type type;

                    template<typename Expr2, typename State2, typename Visitor2>
                    static type call(Expr2 &expr, State2 &state, Visitor2 &)
                    {
                        Fun f;
                        return f(expr, state);
                    }
                };
            } // namespace detail

            /// \brief Wrap \c PrimitiveTransform so that <tt>when\<\></tt> knows
            /// it is callable. Requires that the parameter is actually a 
            /// PrimitiveTransform.
            ///
            /// This form of <tt>call\<\></tt> is useful for annotating an
            /// arbitrary PrimitiveTransform as callable when using it with
            /// <tt>when\<\></tt>. Consider the following transform, which
            /// is parameterized with another transform.
            ///
            /// \code
            /// template<typename Grammar>
            /// struct Foo
            ///   : when< 
            ///         posit<Grammar>
            ///       , Grammar(_arg)   // May or may not work.
            ///     >
            /// {};
            /// \endcode
            ///
            /// The problem with the above is that <tt>when\<\></tt> may or
            /// may not recognize \c Grammar as callable, depending on how
            /// \c Grammar is implemented. (See <tt>is_callable\<\></tt> for
            /// a discussion of this issue.) The above code can guard against
            /// the issue by wrapping \c Grammar in <tt>call\<\></tt>, such
            /// as:
            ///
            /// \code
            /// template<typename Grammar>
            /// struct Foo
            ///   : when<
            ///         posit<Grammar>
            ///       , call<Grammar>(_arg)   // OK, this works
            ///     >
            /// {};
            /// \endcode
            ///
            /// The above could also have been written as:
            ///
            /// \code
            /// template<typename Grammar>
            /// struct Foo
            ///   : when<
            ///         posit<Grammar>
            ///       , call<Grammar(_arg)>   // OK, this works, too
            ///     >
            /// {};
            /// \endcode
            template<typename PrimitiveTransform>
            struct call : PrimitiveTransform
            {
                BOOST_PROTO_CALLABLE()
            };

            /// \brief Either call the PolymorphicFunctionObject with 0
            /// arguments, or invoke the PrimitiveTransform with 3
            /// arguments.
            template<typename Fun>
            struct call<Fun()> : proto::callable
            {
                template<typename Sig>
                struct result;

                template<typename This, typename Expr, typename State, typename Visitor>
                struct result<This(Expr, State, Visitor)>
                {
                    /// If \c Fun is a nullary PolymorphicFunctionObject, \c type is a typedef
                    /// for <tt>boost::result_of\<Fun()\>::::type</tt>. Otherwise, it is
                    /// a typedef for <tt>boost::result_of\<Fun(Expr, State, Visitor)\>::::type</tt>.
                    typedef
                        typename detail::call0<
                            Fun
                          , Expr
                          , State
                          , Visitor
                        >::type
                    type;
                };

                /// Either call the PolymorphicFunctionObject \c Fun with 0 arguments; or
                /// invoke the PrimitiveTransform \c Fun with 3 arguments: the current
                /// expression, state, and visitor.
                ///
                /// If \c Fun is a nullary PolymorphicFunctionObject, return <tt>Fun()()</tt>.
                /// Otherwise, return <tt>Fun()(expr, state, visitor)</tt>.
                ///
                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    typedef
                        detail::call0<
                            Fun
                          , Expr
                          , State
                          , Visitor
                        >
                    impl;

                    return impl::call(expr, state, visitor);
                }
            };

            /// \brief Either call the PolymorphicFunctionObject with 1
            /// argument, or invoke the PrimitiveTransform with 3
            /// arguments.
            template<typename Fun, typename A0>
            struct call<Fun(A0)> : proto::callable
            {
                template<typename Sig>
                struct result;

                template<typename This, typename Expr, typename State, typename Visitor>
                struct result<This(Expr, State, Visitor)>
                {
                    /// Let \c x be <tt>when\<_, A0\>()(expr, state, visitor)</tt> and \c X
                    /// be the type of \c x.
                    /// If \c Fun is a unary PolymorphicFunctionObject that accepts \c x,
                    /// then \c type is a typedef for <tt>boost::result_of\<Fun(X)\>::::type</tt>.
                    /// Otherwise, it is a typedef for <tt>boost::result_of\<Fun(X, State, Visitor)\>::::type</tt>.
                    typedef
                        typename detail::call1<
                            Fun
                          , typename when<_, A0>::template result<void(Expr, State, Visitor)>::type
                          , State
                          , Visitor
                        >::type
                    type;
                };

                /// Either call the PolymorphicFunctionObject with 1 argument:
                /// the result of applying the \c A0 transform; or
                /// invoke the PrimitiveTransform with 3 arguments:
                /// result of applying the \c A0 transform, the state, and the
                /// visitor.
                ///
                /// Let \c x be <tt>when\<_, A0\>()(expr, state, visitor)</tt>.
                /// If \c Fun is a unary PolymorphicFunctionObject that accepts \c x,
                /// then return <tt>Fun()(x)</tt>. Otherwise, return
                /// <tt>Fun()(x, state, visitor)</tt>.
                ///
                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    typedef
                        detail::call1<
                            Fun
                          , typename when<_, A0>::template result<void(Expr, State, Visitor)>::type
                          , State
                          , Visitor
                        >
                    impl;

                    return impl::call(
                        detail::as_lvalue(when<_, A0>()(expr, state, visitor))
                      , state
                      , visitor
                    );
                }
            };

            /// \brief Either call the PolymorphicFunctionObject with 2
            /// arguments, or invoke the PrimitiveTransform with 3
            /// arguments.
            template<typename Fun, typename A0, typename A1>
            struct call<Fun(A0, A1)> : proto::callable
            {
                template<typename Sig>
                struct result;

                template<typename This, typename Expr, typename State, typename Visitor>
                struct result<This(Expr, State, Visitor)>
                {
                    /// Let \c x be <tt>when\<_, A0\>()(expr, state, visitor)</tt> and \c X
                    /// be the type of \c x.
                    /// Let \c y be <tt>when\<_, A1\>()(expr, state, visitor)</tt> and \c Y
                    /// be the type of \c y.
                    /// If \c Fun is a binary PolymorphicFunction object that accepts \c x
                    /// and \c y, then \c type is a typedef for
                    /// <tt>boost::result_of\<Fun(X, Y)\>::::type</tt>. Otherwise, it is
                    /// a typedef for <tt>boost::result_of\<Fun(X, Y, Visitor)\>::::type</tt>.
                    typedef
                        typename detail::call2<
                            Fun
                          , typename when<_, A0>::template result<void(Expr, State, Visitor)>::type
                          , typename when<_, A1>::template result<void(Expr, State, Visitor)>::type
                          , Visitor
                        >::type
                    type;
                };

                /// Either call the PolymorphicFunctionObject with 2 arguments:
                /// the result of applying the \c A0 transform, and the
                /// result of applying the \c A1 transform; or invoke the
                /// PrimitiveTransform with 3 arguments: the result of applying
                /// the \c A0 transform, the result of applying the \c A1
                /// transform, and the visitor.
                ///
                /// Let \c x be <tt>when\<_, A0\>()(expr, state, visitor)</tt>.
                /// Let \c y be <tt>when\<_, A1\>()(expr, state, visitor)</tt>.
                /// If \c Fun is a binary PolymorphicFunction object that accepts \c x
                /// and \c y, return <tt>Fun()(x, y)</tt>. Otherwise, return
                /// <tt>Fun()(x, y, visitor)</tt>.
                ///
                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    typedef
                        detail::call2<
                            Fun
                          , typename when<_, A0>::template result<void(Expr, State, Visitor)>::type
                          , typename when<_, A1>::template result<void(Expr, State, Visitor)>::type
                          , Visitor
                        >
                    impl;

                    return impl::call(
                        detail::as_lvalue(when<_, A0>()(expr, state, visitor))
                      , detail::as_lvalue(when<_, A1>()(expr, state, visitor))
                      , visitor
                    );
                }
            };

            /// \brief Call the PolymorphicFunctionObject or the
            /// PrimitiveTransform with the current expression, state
            /// and visitor, transformed according to \c A0, \c A1, and
            /// \c A2, respectively.
            template<typename Fun, typename A0, typename A1, typename A2>
            struct call<Fun(A0, A1, A2)> : proto::callable
            {
                template<typename Sig>
                struct result;

                template<typename This, typename Expr, typename State, typename Visitor>
                struct result<This(Expr, State, Visitor)>
                {
                    typedef typename when<_, A0>::template result<void(Expr, State, Visitor)>::type a0;
                    typedef typename when<_, A1>::template result<void(Expr, State, Visitor)>::type a1;
                    typedef typename when<_, A2>::template result<void(Expr, State, Visitor)>::type a2;
                    typedef typename boost::result_of<Fun(a0, a1, a2)>::type type;
                };

                /// Let \c x be <tt>when\<_, A0\>()(expr, state, visitor)</tt>.
                /// Let \c y be <tt>when\<_, A1\>()(expr, state, visitor)</tt>.
                /// Let \c z be <tt>when\<_, A2\>()(expr, state, visitor)</tt>.
                /// Return <tt>Fun()(x, y, z)</tt>.
                ///
                /// \param expr The current expression
                /// \param state The current state
                /// \param visitor An arbitrary visitor
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    Fun f;
                    return f(
                        detail::as_lvalue(when<_, A0>()(expr, state, visitor))
                      , detail::as_lvalue(when<_, A1>()(expr, state, visitor))
                      , detail::uncv(when<_, A2>()(expr, state, visitor)) // HACK
                    );
                }
            };

            #if BOOST_PROTO_MAX_ARITY > 3
            #define BOOST_PP_ITERATION_PARAMS_1 (3, (4, BOOST_PROTO_MAX_ARITY, <boost/xpressive/proto/transform/call.hpp>))
            #include BOOST_PP_ITERATE()
            #endif
        }

        /// INTERNAL ONLY
        ///
        template<typename Fun>
        struct is_callable<transform::call<Fun> >
          : mpl::true_
        {};

    }}

    #endif

#else

    #define N BOOST_PP_ITERATION()

        /// \brief Call the PolymorphicFunctionObject \c Fun with the
        /// current expression, state and visitor, transformed according
        /// to \c A0 through \c AN.
        template<typename Fun BOOST_PP_ENUM_TRAILING_PARAMS(N, typename A)>
        struct call<Fun(BOOST_PP_ENUM_PARAMS(N, A))> : proto::callable
        {
            template<typename Sig>
            struct result;

            template<typename This, typename Expr, typename State, typename Visitor>
            struct result<This(Expr, State, Visitor)>
            {
                #define TMP(Z, M, DATA)                                                             \
                    typedef                                                                         \
                        typename when<_, BOOST_PP_CAT(A, M)>                                        \
                            ::template result<void(Expr, State, Visitor)>                           \
                        ::type                                                                      \
                    BOOST_PP_CAT(a, M);                                                             \
                    /**/
                BOOST_PP_REPEAT(N, TMP, ~)
                #undef TMP

                typedef
                    typename boost::result_of<
                        Fun(BOOST_PP_ENUM_PARAMS(N, a))
                    >::type
                type;
            };

            /// Let \c ax be <tt>when\<_, Ax\>()(expr, state, visitor)</tt>
            /// for each \c x in <tt>[0,N]</tt>.
            /// Return <tt>Fun()(a0, a1,... aN)</tt>.
            ///
            /// \param expr The current expression
            /// \param state The current state
            /// \param visitor An arbitrary visitor
            template<typename Expr, typename State, typename Visitor>
            typename result<void(Expr, State, Visitor)>::type
            operator ()(Expr const &expr, State const &state, Visitor &visitor) const
            {
                Fun f;
                #define TMP(Z, M, DATA) when<_, BOOST_PP_CAT(A, M)>()(expr, state, visitor)
                return f(BOOST_PP_ENUM(N, TMP, ~));
                #undef TMP
            }
        };

    #undef N

#endif
