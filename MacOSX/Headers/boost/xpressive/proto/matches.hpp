#ifndef BOOST_PP_IS_ITERATING
    ///////////////////////////////////////////////////////////////////////////////
    /// \file matches.hpp
    /// Contains definition of matches\<\> metafunction for determining if
    /// a given expression matches a given pattern.
    //
    //  Copyright 2008 Eric Niebler. Distributed under the Boost
    //  Software License, Version 1.0. (See accompanying file
    //  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

    #ifndef BOOST_PROTO_MATCHES_HPP_EAN_11_03_2006
    #define BOOST_PROTO_MATCHES_HPP_EAN_11_03_2006

    #include <boost/xpressive/proto/detail/prefix.hpp> // must be first include
    #include <boost/detail/workaround.hpp>
    #include <boost/preprocessor/cat.hpp>
    #include <boost/preprocessor/arithmetic/dec.hpp>
    #include <boost/preprocessor/arithmetic/sub.hpp>
    #include <boost/preprocessor/repetition/enum.hpp>
    #include <boost/preprocessor/iteration/iterate.hpp>
    #include <boost/preprocessor/facilities/intercept.hpp>
    #include <boost/preprocessor/punctuation/comma_if.hpp>
    #include <boost/preprocessor/repetition/enum_params.hpp>
    #include <boost/preprocessor/repetition/enum_shifted.hpp>
    #include <boost/preprocessor/repetition/enum_shifted_params.hpp>
    #include <boost/preprocessor/repetition/enum_trailing_params.hpp>
    #include <boost/preprocessor/repetition/enum_params_with_a_default.hpp>
    #include <boost/config.hpp>
    #include <boost/mpl/logical.hpp>
    #include <boost/mpl/apply.hpp>
    #include <boost/mpl/aux_/template_arity.hpp>
    #include <boost/mpl/aux_/lambda_arity_param.hpp>
    #include <boost/utility/enable_if.hpp>
    #include <boost/type_traits/is_array.hpp>
    #include <boost/type_traits/is_convertible.hpp>
    #include <boost/type_traits/is_reference.hpp>
    #include <boost/type_traits/is_pointer.hpp>
    #include <boost/xpressive/proto/proto_fwd.hpp>
    #include <boost/xpressive/proto/traits.hpp>
    #include <boost/xpressive/proto/transform/when.hpp>
    #include <boost/xpressive/proto/detail/suffix.hpp> // must be last include

    // Some compilers (like GCC) need extra help figuring out a template's arity.
    // I use MPL's BOOST_MPL_AUX_LAMBDA_ARITY_PARAM() macro to disambiguate, which
    // which is controlled by the BOOST_MPL_LIMIT_METAFUNCTION_ARITY macro. If
    // You define BOOST_PROTO_MAX_ARITY to be greater than
    // BOOST_MPL_LIMIT_METAFUNCTION_ARITY on these compilers, things don't work.
    // You must define BOOST_MPL_LIMIT_METAFUNCTION_ARITY to be greater.
    #ifdef BOOST_MPL_CFG_EXTENDED_TEMPLATE_PARAMETERS_MATCHING
    # if BOOST_PROTO_MAX_ARITY > BOOST_MPL_LIMIT_METAFUNCTION_ARITY
    #  error BOOST_MPL_LIMIT_METAFUNCTION_ARITY must be at least as large as BOOST_PROTO_MAX_ARITY
    # endif
    #endif

    #if defined(_MSC_VER) && (_MSC_VER >= 1020)
    # pragma warning(push)
    # pragma warning(disable:4305) // 'specialization' : truncation from 'const int' to 'bool'
    #endif

    namespace boost { namespace proto
    {

        namespace detail
        {
            struct ignore;

            template<typename Expr, typename Grammar>
            struct matches_;

            // and_ and or_ implementation
            template<bool B, typename Expr, typename G0>
            struct or1
              : mpl::bool_<B>
            {
                typedef G0 which;
            };

            template<bool B>
            struct and1
              : mpl::bool_<B>
            {};

            template<bool B, typename Pred>
            struct and2;

            template<typename And>
            struct last;

            template<typename T, typename U>
            struct array_matches
              : mpl::false_
            {};

            template<typename T, std::size_t M>
            struct array_matches<T[M], T *>
              : mpl::true_
            {};

            template<typename T, std::size_t M>
            struct array_matches<T[M], T const *>
              : mpl::true_
            {};

            template<typename T, std::size_t M>
            struct array_matches<T[M], T[proto::N]>
              : mpl::true_
            {};

            template<typename T, typename U
                BOOST_MPL_AUX_LAMBDA_ARITY_PARAM(long Arity = mpl::aux::template_arity<U>::value)
            >
            struct lambda_matches
              : mpl::false_
            {};

            template<typename T>
            struct lambda_matches<T, proto::_ BOOST_MPL_AUX_LAMBDA_ARITY_PARAM(-1)>
              : mpl::true_
            {};

            template<typename T>
            struct lambda_matches<T, T BOOST_MPL_AUX_LAMBDA_ARITY_PARAM(-1)>
              : mpl::true_
            {};

            template<typename T, std::size_t M, typename U>
            struct lambda_matches<T[M], U BOOST_MPL_AUX_LAMBDA_ARITY_PARAM(-1)>
              : array_matches<T[M], U>
            {};

            template<typename T, std::size_t M>
            struct lambda_matches<T[M], T[M] BOOST_MPL_AUX_LAMBDA_ARITY_PARAM(-1)>
              : mpl::true_
            {};

            template<template<typename> class T, typename Expr0, typename Grammar0>
            struct lambda_matches<T<Expr0>, T<Grammar0> BOOST_MPL_AUX_LAMBDA_ARITY_PARAM(1) >
              : lambda_matches<Expr0, Grammar0>
            {};

            // vararg_matches_impl
            template<typename Args1, typename Back, long From, long To>
            struct vararg_matches_impl;

            // vararg_matches
            template<typename Args1, typename Args2, typename Back, bool Can, bool Zero, typename Void = void>
            struct vararg_matches
              : mpl::false_
            {};

            template<typename Args1, typename Args2, typename Back>
            struct vararg_matches<Args1, Args2, Back, true, true, typename Back::proto_is_vararg_>
              : matches_<proto::expr<ignore, Args1, Args1::size>, proto::expr<ignore, Args2, Args1::size> >
            {};

            template<typename Args1, typename Args2, typename Back>
            struct vararg_matches<Args1, Args2, Back, true, false, typename Back::proto_is_vararg_>
              : and2<
                    matches_<proto::expr<ignore, Args1, Args2::size>, proto::expr<ignore, Args2, Args2::size> >::value
                  , vararg_matches_impl<Args1, typename Back::proto_base_expr, Args2::size + 1, Args1::size>
                >
            {};

            // How terminal_matches<> handles references and cv-qualifiers.
            // The cv and ref_ matter *only* if the grammar has a top-level ref_.
            //
            // Expr     |   Grammar  |  Match
            // ------------------------------
            // T            T           yes
            // T &          T           yes
            // T const &    T           yes
            // T            T &         no
            // T &          T &         yes
            // T const &    T &         no
            // T            T const &   no
            // T &          T const &   no
            // T const &    T const &   yes

            template<typename T, typename U>
            struct is_cv_ref_compatible
              : mpl::true_
            {};

            template<typename T, typename U>
            struct is_cv_ref_compatible<T, U &>
              : mpl::false_
            {};

            template<typename T, typename U>
            struct is_cv_ref_compatible<T &, U &>
              : mpl::bool_<is_const<T>::value == is_const<U>::value>
            {};

        #if BOOST_WORKAROUND(BOOST_MSVC, == 1310)
            // MSVC-7.1 has lots of problems with array types that have been
            // deduced. Partially specializing terminal_matches<> on array types
            // doesn't seem to work.
            template<
                typename T
              , typename U
              , bool B = is_array<BOOST_PROTO_UNCVREF(T)>::value
            >
            struct terminal_array_matches
              : mpl::false_
            {};

            template<typename T, typename U, std::size_t M>
            struct terminal_array_matches<T, U(&)[M], true>
              : is_convertible<T, U(&)[M]>
            {};

            template<typename T, typename U>
            struct terminal_array_matches<T, U(&)[proto::N], true>
              : is_convertible<T, U *>
            {};

            template<typename T, typename U>
            struct terminal_array_matches<T, U *, true>
              : is_convertible<T, U *>
            {};

            // terminal_matches
            template<typename T, typename U>
            struct terminal_matches
              : mpl::or_<
                    mpl::and_<
                        is_cv_ref_compatible<T, U>
                      , lambda_matches<
                            BOOST_PROTO_UNCVREF(T)
                          , BOOST_PROTO_UNCVREF(U)
                        >
                    >
                  , terminal_array_matches<T, U>
                >
            {};
        #else
            // terminal_matches
            template<typename T, typename U>
            struct terminal_matches
              : mpl::and_<
                    is_cv_ref_compatible<T, U>
                  , lambda_matches<
                        BOOST_PROTO_UNCVREF(T)
                      , BOOST_PROTO_UNCVREF(U)
                    >
                >
            {};

            template<typename T, std::size_t M>
            struct terminal_matches<T(&)[M], T(&)[proto::N]>
              : mpl::true_
            {};

            template<typename T, std::size_t M>
            struct terminal_matches<T(&)[M], T *>
              : mpl::true_
            {};
        #endif

            template<typename T>
            struct terminal_matches<T, T>
              : mpl::true_
            {};

            template<typename T>
            struct terminal_matches<T &, T>
              : mpl::true_
            {};

            template<typename T>
            struct terminal_matches<T const &, T>
              : mpl::true_
            {};

            template<typename T>
            struct terminal_matches<T, proto::_>
              : mpl::true_
            {};

            template<typename T>
            struct terminal_matches<T, exact<T> >
              : mpl::true_
            {};

            template<typename T, typename U>
            struct terminal_matches<T, proto::convertible_to<U> >
              : is_convertible<T, U>
            {};

            // matches_
            template<typename Expr, typename Grammar>
            struct matches_
              : mpl::false_
            {};

            template<typename Expr>
            struct matches_< Expr, proto::_ >
              : mpl::true_
            {};

            template<typename Tag, typename Args1, long N1, typename Args2, long N2>
            struct matches_< proto::expr<Tag, Args1, N1>, proto::expr<Tag, Args2, N2> >
              : vararg_matches< Args1, Args2, typename Args2::back_, (N1+2 > N2), (N2 > N1) >
            {};

            template<typename Tag, typename Args1, long N1, typename Args2, long N2>
            struct matches_< proto::expr<Tag, Args1, N1>, proto::expr<proto::_, Args2, N2> >
              : vararg_matches< Args1, Args2, typename Args2::back_, (N1+2 > N2), (N2 > N1) >
            {};

            template<typename Args1, typename Args2, long N2>
            struct matches_< proto::expr<tag::terminal, Args1, 0>, proto::expr<proto::_, Args2, N2> >
              : mpl::false_
            {};

            template<typename Tag, typename Args1, typename Args2>
            struct matches_< proto::expr<Tag, Args1, 1>, proto::expr<Tag, Args2, 1> >
              : matches_<typename Args1::arg0::proto_base_expr, typename Args2::arg0::proto_base_expr>
            {};

            template<typename Tag, typename Args1, typename Args2>
            struct matches_< proto::expr<Tag, Args1, 1>, proto::expr<proto::_, Args2, 1> >
              : matches_<typename Args1::arg0::proto_base_expr, typename Args2::arg0::proto_base_expr>
            {};

            template<typename Args1, typename Args2>
            struct matches_< proto::expr<tag::terminal, Args1, 0>, proto::expr<tag::terminal, Args2, 0> >
              : terminal_matches<typename Args1::arg0, typename Args2::arg0>
            {};

        #define BOOST_PROTO_MATCHES_N_FUN(z, n, data)\
            matches_<\
                typename Args1::BOOST_PP_CAT(arg, n)::proto_base_expr\
              , typename Args2::BOOST_PP_CAT(arg, n)::proto_base_expr\
            >

        #define BOOST_PROTO_DEFINE_MATCHES(z, n, data)\
            matches_<\
                typename Expr::proto_base_expr\
              , typename BOOST_PP_CAT(G, n)::proto_base_expr\
            >

        #define BOOST_PROTO_DEFINE_LAMBDA_MATCHES(z, n, data)\
            lambda_matches<\
                BOOST_PP_CAT(Expr, n)\
              , BOOST_PP_CAT(Grammar, n)\
            >

        #if BOOST_PROTO_MAX_LOGICAL_ARITY > BOOST_PROTO_MAX_ARITY
            #define BOOST_PP_ITERATION_PARAMS_1 (4, (2, BOOST_PROTO_MAX_LOGICAL_ARITY, <boost/xpressive/proto/matches.hpp>, 1))
        #else
            #define BOOST_PP_ITERATION_PARAMS_1 (4, (2, BOOST_PROTO_MAX_ARITY, <boost/xpressive/proto/matches.hpp>, 1))
        #endif
        #include BOOST_PP_ITERATE()

        #define BOOST_PP_ITERATION_PARAMS_1 (4, (2, BOOST_PROTO_MAX_ARITY, <boost/xpressive/proto/matches.hpp>, 2))
        #include BOOST_PP_ITERATE()

        #undef BOOST_PROTO_MATCHES_N_FUN
        #undef BOOST_PROTO_DEFINE_MATCHES
        #undef BOOST_PROTO_DEFINE_LAMBDA_MATCHES

            // handle proto::if_
            template<typename Expr, typename If, typename Then, typename Else>
            struct matches_<Expr, proto::if_<If, Then, Else> >
              : mpl::eval_if<
                    typename when<_, If>::template result<void(Expr, int, int)>::type
                  , matches_<Expr, typename Then::proto_base_expr>
                  , matches_<Expr, typename Else::proto_base_expr>
                >::type
            {};

            template<typename Expr, typename If>
            struct matches_<Expr, proto::if_<If> >
              : when<_, If>::template result<void(Expr, int, int)>::type
            {};

            // handle proto::not_
            template<typename Expr, typename Grammar>
            struct matches_<Expr, not_<Grammar> >
              : mpl::not_<matches_<Expr, typename Grammar::proto_base_expr> >
            {};

            // handle proto::switch_
            template<typename Expr, typename Cases>
            struct matches_<Expr, switch_<Cases> >
              : matches_<
                    Expr
                  , typename Cases::template case_<typename Expr::proto_tag>::proto_base_expr
                >
            {};
        }

        namespace result_of
        {
            /// \brief A Boolean metafunction that evaluates whether a given
            /// expression type matches a grammar.
            ///
            /// <tt>matches\<Expr,Grammar\></tt> inherits (indirectly) from
            /// \c mpl::true_ if <tt>Expr::proto_base_expr</tt> matches
            /// <tt>Grammar::proto_base_expr</tt>, and from \c mpl::false_
            /// otherwise.
            ///
            /// Non-terminal expressions are matched against a grammar
            /// according to the following rules:
            ///
            /// \li The wildcard pattern, \c _, matches any expression.
            /// \li An expression <tt>expr\<AT, argsN\<A0,A1,...An\> \></tt>
            ///     matches a grammar <tt>expr\<BT, argsN\<B0,B1,...Bn\> \></tt>
            ///     if \c BT is \c _ or \c AT, and if \c Ax matches \c Bx for
            ///     each \c x in <tt>[0,n)</tt>.
            /// \li An expression <tt>expr\<AT, argsN\<A0,...An,U0,...Um\> \></tt>
            ///     matches a grammar <tt>expr\<BT, argsM\<B0,...Bn,vararg\<V\> \> \></tt>
            ///     if \c BT is \c _ or \c AT, and if \c Ax matches \c Bx
            ///     for each \c x in <tt>[0,n)</tt> and if \c Ux matches \c V
            ///     for each \c x in <tt>[0,m)</tt>.
            /// \li An expression \c E matches <tt>or_\<B0,B1,...Bn\></tt> if \c E
            ///     matches some \c Bx for \c x in <tt>[0,n)</tt>.
            /// \li An expression \c E matches <tt>and_\<B0,B1,...Bn\></tt> if \c E
            ///     matches all \c Bx for \c x in <tt>[0,n)</tt>.
            /// \li An expression \c E matches <tt>if_\<T,U,V\></tt> if
            ///     <tt>when\<_,T\>::::result\<void(E,int,int)\>::::type::value</tt>
            ///     is \c true and \c E matches \c U; or, if
            ///     <tt>when\<_,T\>::::result\<void(E,int,int)\>::::type::value</tt>
            ///     is \c false and \c E matches \c V. (Note: \c U defaults to \c _
            ///     and \c V defaults to \c not_\<_\>.)
            /// \li An expression \c E matches <tt>not_\<T\></tt> if \c E does
            ///     not match \c T.
            /// \li An expression \c E matches <tt>switch_\<C\></tt> if
            ///     \c E matches <tt>C::case_\<E::proto_tag\></tt>.
            ///
            /// A terminal expression <tt>expr\<tag::terminal,args0\<A\> \></tt> matches
            /// a grammar <tt>expr\<BT,args0\<B\> \></tt> if \c BT is \c _ or
            /// \c tag::terminal and one of the following is true:
            ///
            /// \li \c B is the wildcard pattern, \c _
            /// \li \c A is \c B
            /// \li \c A is <tt>B &</tt>
            /// \li \c A is <tt>B const &</tt>
            /// \li \c B is <tt>exact\<A\></tt>
            /// \li \c B is <tt>convertible_to\<X\></tt> and
            ///     <tt>is_convertible\<A,X\>::::value</tt> is \c true.
            /// \li \c A is <tt>X[M]</tt> or <tt>X(&)[M]</tt> and
            ///     \c B is <tt>X[proto::N]</tt>.
            /// \li \c A is <tt>X(&)[M]</tt> and \c B is <tt>X(&)[proto::N]</tt>.
            /// \li \c A is <tt>X[M]</tt> or <tt>X(&)[M]</tt> and
            ///     \c B is <tt>X*</tt>.
            /// \li \c B lambda-matches \c A (see below).
            ///
            /// A type \c B lambda-matches \c A if one of the following is true:
            ///
            /// \li \c B is \c A
            /// \li \c B is the wildcard pattern, \c _
            /// \li \c B is <tt>T\<B0,B1,...Bn\></tt> and \c A is
            ///     <tt>T\<A0,A1,...An\></tt> and for each \c x in
            ///     <tt>[0,n)</tt>, \c Ax and \c Bx are types
            ///     such that \c Ax lambda-matches \c Bx 
            template<typename Expr, typename Grammar>
            struct matches
              : detail::matches_<
                    typename Expr::proto_base_expr
                  , typename Grammar::proto_base_expr
                >
            {};
        }

        namespace wildcardns_
        {
            /// \brief A wildcard grammar element that matches any expression,
            /// and a transform that returns the current expression unchanged.
            /// 
            /// The wildcard type, \c _, is a grammar element such that
            /// <tt>matches\<E,_\>::::value</tt> is \c true for any expression
            /// type \c E.
            ///
            /// The wildcard can also be used as a stand-in for a template
            /// argument when matching terminals. For instance, the following
            /// is a grammar that will match any <tt>std::complex\<\></tt>
            /// terminal:
            ///
            /// \code
            /// BOOST_MPL_ASSERT((
            ///     matches<
            ///         terminal<std::complex<double> >::type
            ///       , terminal<std::complex< _ > >
            ///     >
            /// ));
            /// \endcode
            ///
            /// When used as a transform, \c _ returns the current expression
            /// unchanged. For instance, in the following, \c _ is used with
            /// the \c fold\<\> transform to fold the children of a node:
            ///
            /// \code
            /// struct CountChildren
            ///   : or_<
            ///         // Terminals have no children
            ///         when<terminal<_>, mpl::int_<0>()>
            ///         // Use fold<> to count the children of non-terminals
            ///       , otherwise<
            ///             fold<
            ///                 _ // <-- fold the current expression
            ///               , mpl::int_<0>()
            ///               , mpl::plus<_state, mpl::int_<1> >()
            ///             >
            ///         >
            ///     >
            /// {};
            /// \endcode
            struct _ : proto::callable
            {
                typedef _ proto_base_expr;

                template<typename Sig>
                struct result;

                template<typename This, typename Expr, typename State, typename Visitor>
                struct result<This(Expr, State, Visitor)>
                {
                    typedef Expr type;
                };

                /// \param expr An expression
                /// \return \c expr
                template<typename Expr, typename State, typename Visitor>
                Expr const &operator ()(Expr const &expr, State const &, Visitor &) const
                {
                    return expr;
                }
            };
        }

        namespace control
        {
            /// \brief Inverts the set of expressions matched by a grammar. When
            /// used as a transform, \c not_\<\> returns the current expression
            /// unchanged.
            ///
            /// If an expression type \c E does not match a grammar \c G, then
            /// \c E \e does match <tt>not_\<G\></tt>. For example,
            /// <tt>not_\<terminal\<_\> \></tt> will match any non-terminal.
            template<typename Grammar>
            struct not_ : proto::callable
            {
                typedef not_ proto_base_expr;

                template<typename Sig>
                struct result;

                template<typename This, typename Expr, typename State, typename Visitor>
                struct result<This(Expr, State, Visitor)>
                {
                    typedef Expr type;
                };

                /// \param expr An expression
                /// \pre <tt>matches\<Expr,not_\>::::value</tt> is \c true.
                /// \return \c expr
                template<typename Expr, typename State, typename Visitor>
                Expr const &operator ()(Expr const &expr, State const &, Visitor &) const
                {
                    return expr;
                }
            };

            /// \brief Used to select one grammar or another based on the result
            /// of a compile-time Boolean. When used as a transform, \c if_\<\>
            /// selects between two transforms based on a compile-time Boolean.
            ///
            /// When <tt>if_\<If,Then,Else\></tt> is used as a grammar, \c If
            /// must be a Proto transform and \c Then and \c Else must be grammars.
            /// An expression type \c E matches <tt>if_\<If,Then,Else\></tt> if
            /// <tt>when\<_,If\>::::result\<void(E,int,int)\>::::type::value</tt>
            /// is \c true and \c E matches \c U; or, if
            /// <tt>when\<_,If\>::::result\<void(E,int,int)\>::::type::value</tt>
            /// is \c false and \c E matches \c V.
            ///
            /// The template parameter \c Then defaults to \c _
            /// and \c Else defaults to \c not\<_\>, so an expression type \c E
            /// will match <tt>if_\<If\></tt> if and only if
            /// <tt>when\<_,If\>::::result\<void(E,int,int)\>::::type::value</tt>
            /// is \c true.
            ///
            /// \code
            /// // A grammar that only matches integral terminals,
            /// // using is_integral<> from Boost.Type_traits.
            /// struct IsIntegral
            ///   : and_<
            ///         terminal<_>
            ///       , if_< is_integral<_arg>() >
            ///     >
            /// {};
            /// \endcode
            ///
            /// When <tt>if_\<If,Then,Else\></tt> is used as a transform, \c If,
            /// \c Then and \c Else must be Proto transforms. When applying
            /// the transform to an expression \c E, state \c S and visitor \c V,
            /// if <tt>when\<_,If\>::::result\<void(E,S,V)\>::::type::value</tt>
            /// is \c true then the \c Then transform is applied; otherwise
            /// the \c Else transform is applied.
            ///
            /// \code
            /// // Match a terminal. If the terminal is integral, return
            /// // mpl::true_; otherwise, return mpl::false_.
            /// struct IsIntegral2
            ///   : when<
            ///         terminal<_>
            ///       , if_<
            ///             is_integral<_arg>()
            ///           , mpl::true_()
            ///           , mpl::false_()
            ///         >
            ///     >
            /// {};
            /// \endcode
            template<
                typename If
              , typename Then   BOOST_PROTO_FOR_DOXYGEN_ONLY(= _)
              , typename Else   BOOST_PROTO_FOR_DOXYGEN_ONLY(= not_<_>)
            >
            struct if_ : proto::callable
            {
                typedef if_ proto_base_expr;

                template<typename Sig>
                struct result;

                template<typename This, typename Expr, typename State, typename Visitor>
                struct result<This(Expr, State, Visitor)>
                {
                    typedef
                        typename when<_, If>::template result<void(Expr, State, Visitor)>::type
                    condition;

                    typedef
                        typename mpl::if_<
                            condition
                          , when<_, Then>
                          , when<_, Else>
                        >::type
                    which;

                    typedef typename which::template result<void(Expr, State, Visitor)>::type type;
                };

                /// \param expr An expression
                /// \param state The current state
                /// \param visitor A visitor of arbitrary type
                /// \return <tt>result\<void(Expr, State, Visitor)\>::::which()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    typedef
                        typename result<void(Expr, State, Visitor)>::which
                    which;

                    return which()(expr, state, visitor);
                }
            };

            /// \brief For matching one of a set of alternate grammars. Alternates
            /// tried in order to avoid ambiguity. When used as a transform, \c or_\<\>
            /// applies the transform associated with the first grammar that matches
            /// the expression.
            ///
            /// An expression type \c E matches <tt>or_\<B0,B1,...Bn\></tt> if \c E
            /// matches any \c Bx for \c x in <tt>[0,n)</tt>.
            ///
            /// When applying <tt>or_\<B0,B1,...Bn\></tt> as a transform with an
            /// expression \c e of type \c E, state \c s and visitor \c v, it is
            /// equivalent to <tt>Bx()(e, s, v)</tt>, where \c x is the lowest
            /// number such that <tt>matches\<E,Bx\>::::value</tt> is \c true.
            template<BOOST_PP_ENUM_PARAMS(BOOST_PROTO_MAX_LOGICAL_ARITY, typename G)>
            struct or_ : proto::callable
            {
                typedef or_ proto_base_expr;

                template<typename Sig>
                struct result;

                template<typename This, typename Expr, typename State, typename Visitor>
                struct result<This(Expr, State, Visitor)>
                {
                    typedef typename detail::matches_<Expr, or_>::which which;
                    typedef typename which::template result<void(Expr, State, Visitor)>::type type;
                };

                /// \param expr An expression
                /// \param state The current state
                /// \param visitor A visitor of arbitrary type
                /// \pre <tt>matches\<Expr,or_\>::::value</tt> is \c true.
                /// \return <tt>result\<void(Expr, State, Visitor)\>::::which()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    typedef typename detail::matches_<Expr, or_>::which which;
                    return which()(expr, state, visitor);
                }
            };

            /// \brief For matching all of a set of grammars. When used as a
            /// transform, \c and_\<\> applies the transform associated with
            /// the last grammar in the set.
            ///
            /// An expression type \c E matches <tt>and_\<B0,B1,...Bn\></tt> if \c E
            /// matches all \c Bx for \c x in <tt>[0,n)</tt>.
            ///
            /// When applying <tt>and_\<B0,B1,...Bn\></tt> as a transform with an
            /// expression \c e, state \c s and visitor \c v, it is
            /// equivalent to <tt>Bn()(e, s, v)</tt>.
            template<BOOST_PP_ENUM_PARAMS(BOOST_PROTO_MAX_LOGICAL_ARITY, typename G)>
            struct and_ : proto::callable
            {
                typedef and_ proto_base_expr;

                template<typename Sig>
                struct result;

                template<typename This, typename Expr, typename State, typename Visitor>
                struct result<This(Expr, State, Visitor)>
                {
                    typedef typename detail::last<and_>::type which;
                    typedef typename which::template result<void(Expr, State, Visitor)>::type type;
                };

                /// \param expr An expression
                /// \param state The current state
                /// \param visitor A visitor of arbitrary type
                /// \pre <tt>matches\<Expr,and_\>::::value</tt> is \c true.
                /// \return <tt>result\<void(Expr, State, Visitor)\>::::which()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    typedef typename detail::last<and_>::type which;
                    return which()(expr, state, visitor);
                }
            };

            /// \brief For matching one of a set of alternate grammars, which
            /// are looked up based on an expression's tag type. When used as a
            /// transform, \c switch_\<\> applies the transform associated with
            /// the grammar that matches the expression.
            ///
            /// \note \c switch_\<\> is functionally identical to \c or_\<\> but
            /// is often more efficient. It does a fast, O(1) lookup based on an
            /// expression's tag type to find a sub-grammar that may potentially
            /// match the expression.
            ///
            /// An expression type \c E matches <tt>switch_\<C\></tt> if \c E
            /// matches <tt>C::case_\<E::proto_tag\></tt>.
            ///
            /// When applying <tt>switch_\<C\></tt> as a transform with an
            /// expression \c e of type \c E, state \c s and visitor \c v, it is
            /// equivalent to <tt>C::case_\<E::proto_tag\>()(e, s, v)</tt>.
            template<typename Cases>
            struct switch_ : proto::callable
            {
                typedef switch_ proto_base_expr;

                template<typename Sig>
                struct result;

                template<typename This, typename Expr, typename State, typename Visitor>
                struct result<This(Expr, State, Visitor)>
                {
                    typedef typename Cases::template case_<typename Expr::proto_tag> which;
                    typedef typename which::template result<void(Expr, State, Visitor)>::type type;
                };

                /// \param expr An expression
                /// \param state The current state
                /// \param visitor A visitor of arbitrary type
                /// \pre <tt>matches\<Expr,switch_\>::::value</tt> is \c true.
                /// \return <tt>result\<void(Expr, State, Visitor)\>::::which()(expr, state, visitor)</tt>
                template<typename Expr, typename State, typename Visitor>
                typename result<void(Expr, State, Visitor)>::type
                operator ()(Expr const &expr, State const &state, Visitor &visitor) const
                {
                    typedef typename Cases::template case_<typename Expr::proto_tag> which;
                    return which()(expr, state, visitor);
                }
            };

            /// \brief For forcing exact matches of terminal types.
            ///
            /// By default, matching terminals ignores references and
            /// cv-qualifiers. For instance, a terminal expression of
            /// type <tt>terminal\<int const &\>::::type</tt> will match
            /// the grammar <tt>terminal\<int\></tt>. If that is not
            /// desired, you can force an exact match with
            /// <tt>terminal\<exact\<int\> \></tt>. This will only
            /// match integer terminals where the terminal is held by
            /// value.
            template<typename T>
            struct exact
            {};

            /// \brief For matching terminals that are convertible to
            /// a type.
            ///
            /// Use \c convertible_to\<\> to match a terminal that is
            /// convertible to some type. For example, the grammar
            /// <tt>terminal\<convertible_to\<int\> \></tt> will match
            /// any terminal whose argument is convertible to an integer.
            ///
            /// \note The trait \c is_convertible\<\> from Boost.Type_traits
            /// is used to determinal convertibility.
            template<typename T>
            struct convertible_to
            {};

            /// \brief For matching a Grammar to a variable number of
            /// sub-expressions.
            ///
            /// An expression type <tt>expr\<AT, argsN\<A0,...An,U0,...Um\> \></tt>
            /// matches a grammar <tt>expr\<BT, argsM\<B0,...Bn,vararg\<V\> \> \></tt>
            /// if \c BT is \c _ or \c AT, and if \c Ax matches \c Bx
            /// for each \c x in <tt>[0,n)</tt> and if \c Ux matches \c V
            /// for each \c x in <tt>[0,m)</tt>.
            ///
            /// For example:
            ///
            /// \code
            /// // Match any function call expression, irregardless
            /// // of the number of function arguments:
            /// struct Function
            ///   : function< vararg<_> >
            /// {};
            /// \endcode
            ///
            /// When used as a transform, <tt>vararg\<G\></tt> applies
            /// <tt>G</tt>'s transform.
            template<typename Grammar>
            struct vararg
              : Grammar
            {
                typedef void proto_is_vararg_;
            };
        }

        /// INTERNAL ONLY
        ///
        template<BOOST_PP_ENUM_PARAMS(BOOST_PROTO_MAX_LOGICAL_ARITY, typename G)>
        struct is_callable<or_<BOOST_PP_ENUM_PARAMS(BOOST_PROTO_MAX_LOGICAL_ARITY, G)> >
          : mpl::true_
        {};

        /// INTERNAL ONLY
        ///
        template<BOOST_PP_ENUM_PARAMS(BOOST_PROTO_MAX_LOGICAL_ARITY, typename G)>
        struct is_callable<and_<BOOST_PP_ENUM_PARAMS(BOOST_PROTO_MAX_LOGICAL_ARITY, G)> >
          : mpl::true_
        {};

        /// INTERNAL ONLY
        ///
        template<typename Grammar>
        struct is_callable<not_<Grammar> >
          : mpl::true_
        {};

        /// INTERNAL ONLY
        ///
        template<typename If, typename Then, typename Else>
        struct is_callable<if_<If, Then, Else> >
          : mpl::true_
        {};

        /// INTERNAL ONLY
        ///
        template<typename Grammar>
        struct is_callable<vararg<Grammar> >
          : mpl::true_
        {};

    }}

    #if defined(_MSC_VER) && (_MSC_VER >= 1020)
    # pragma warning(pop)
    #endif

    #endif

#elif BOOST_PP_ITERATION_FLAGS() == 1

    #define N BOOST_PP_ITERATION()

            template<bool B, BOOST_PP_ENUM_PARAMS(BOOST_PP_DEC(N), typename P)>
            struct BOOST_PP_CAT(and, N)
              : BOOST_PP_CAT(and, BOOST_PP_DEC(N))<
                    P0::value BOOST_PP_COMMA_IF(BOOST_PP_SUB(N,2))
                    BOOST_PP_ENUM_SHIFTED_PARAMS(BOOST_PP_DEC(N), P)
                >
            {};

            template<BOOST_PP_ENUM_PARAMS(BOOST_PP_DEC(N), typename P)>
            struct BOOST_PP_CAT(and, N)<false, BOOST_PP_ENUM_PARAMS(BOOST_PP_DEC(N), P)>
              : mpl::false_
            {};

        #if N <= BOOST_PROTO_MAX_LOGICAL_ARITY
            template<BOOST_PP_ENUM_PARAMS(N, typename G)>
            struct last<proto::and_<BOOST_PP_ENUM_PARAMS(N, G)> >
            {
                typedef BOOST_PP_CAT(G, BOOST_PP_DEC(N)) type;
            };

            template<bool B, typename Expr, BOOST_PP_ENUM_PARAMS(N, typename G)>
            struct BOOST_PP_CAT(or, N)
              : BOOST_PP_CAT(or, BOOST_PP_DEC(N))<
                    matches_<Expr, typename G1::proto_base_expr>::value
                  , Expr, BOOST_PP_ENUM_SHIFTED_PARAMS(N, G)
                >
            {};

            template<typename Expr BOOST_PP_ENUM_TRAILING_PARAMS(N, typename G)>
            struct BOOST_PP_CAT(or, N)<true, Expr, BOOST_PP_ENUM_PARAMS(N, G)>
              : mpl::true_
            {
                typedef G0 which;
            };

            // handle proto::or_
            template<typename Expr, BOOST_PP_ENUM_PARAMS(N, typename G)>
            struct matches_<Expr, proto::or_<BOOST_PP_ENUM_PARAMS(N, G)> >
              : BOOST_PP_CAT(or, N)<
                    matches_<typename Expr::proto_base_expr, typename G0::proto_base_expr>::value,
                    typename Expr::proto_base_expr, BOOST_PP_ENUM_PARAMS(N, G)
                >
            {};

            // handle proto::and_
            template<typename Expr, BOOST_PP_ENUM_PARAMS(N, typename G)>
            struct matches_<Expr, proto::and_<BOOST_PP_ENUM_PARAMS(N, G)> >
              : detail::BOOST_PP_CAT(and, N)<
                    BOOST_PROTO_DEFINE_MATCHES(~, 0, ~)::value,
                    BOOST_PP_ENUM_SHIFTED(N, BOOST_PROTO_DEFINE_MATCHES, ~)
                >
            {};
        #endif

    #undef N

#elif BOOST_PP_ITERATION_FLAGS() == 2

    #define N BOOST_PP_ITERATION()

            template<typename Args, typename Back, long To>
            struct vararg_matches_impl<Args, Back, N, To>
              : and2<
                    matches_<typename Args::BOOST_PP_CAT(arg, BOOST_PP_DEC(N))::proto_base_expr, Back>::value
                  , vararg_matches_impl<Args, Back, N + 1, To>
                >
            {};

            template<typename Args, typename Back>
            struct vararg_matches_impl<Args, Back, N, N>
              : matches_<typename Args::BOOST_PP_CAT(arg, BOOST_PP_DEC(N))::proto_base_expr, Back>
            {};

            template<
                template<BOOST_PP_ENUM_PARAMS(N, typename BOOST_PP_INTERCEPT)> class T
                BOOST_PP_ENUM_TRAILING_PARAMS(N, typename Expr)
                BOOST_PP_ENUM_TRAILING_PARAMS(N, typename Grammar)
            >
            struct lambda_matches<T<BOOST_PP_ENUM_PARAMS(N, Expr)>, T<BOOST_PP_ENUM_PARAMS(N, Grammar)> BOOST_MPL_AUX_LAMBDA_ARITY_PARAM(N) >
              : BOOST_PP_CAT(and, N)<
                    BOOST_PROTO_DEFINE_LAMBDA_MATCHES(~, 0, ~)::value,
                    BOOST_PP_ENUM_SHIFTED(N, BOOST_PROTO_DEFINE_LAMBDA_MATCHES, ~)
                >
            {};

            template<typename Tag, typename Args1, typename Args2>
            struct matches_< proto::expr<Tag, Args1, N>, proto::expr<Tag, Args2, N> >
              : BOOST_PP_CAT(and, N)<
                    BOOST_PROTO_MATCHES_N_FUN(~, 0, ~)::value,
                    BOOST_PP_ENUM_SHIFTED(N, BOOST_PROTO_MATCHES_N_FUN, ~)
                >
            {};

            template<typename Tag, typename Args1, typename Args2>
            struct matches_< proto::expr<Tag, Args1, N>, proto::expr<proto::_, Args2, N> >
              : BOOST_PP_CAT(and, N)<
                    BOOST_PROTO_MATCHES_N_FUN(~, 0, ~)::value,
                    BOOST_PP_ENUM_SHIFTED(N, BOOST_PROTO_MATCHES_N_FUN, ~)
                >
            {};

    #undef N

#endif

