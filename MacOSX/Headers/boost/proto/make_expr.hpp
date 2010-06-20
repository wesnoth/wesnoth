#ifndef BOOST_PP_IS_ITERATING
    ///////////////////////////////////////////////////////////////////////////////
    /// \file make_expr.hpp
    /// Definition of the \c make_expr() and \c unpack_expr() utilities for
    /// building Proto expression nodes from child nodes or from a Fusion
    /// sequence of child nodes, respectively.
    //
    //  Copyright 2008 Eric Niebler. Distributed under the Boost
    //  Software License, Version 1.0. (See accompanying file
    //  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

    #ifndef BOOST_PROTO_MAKE_EXPR_HPP_EAN_04_01_2005
    #define BOOST_PROTO_MAKE_EXPR_HPP_EAN_04_01_2005

    #include <boost/proto/detail/prefix.hpp>
    #include <boost/version.hpp>
    #include <boost/config.hpp>
    #include <boost/detail/workaround.hpp>
    #include <boost/preprocessor/cat.hpp>
    #include <boost/preprocessor/control/if.hpp>
    #include <boost/preprocessor/control/expr_if.hpp>
    #include <boost/preprocessor/arithmetic/inc.hpp>
    #include <boost/preprocessor/arithmetic/dec.hpp>
    #include <boost/preprocessor/arithmetic/sub.hpp>
    #include <boost/preprocessor/punctuation/comma_if.hpp>
    #include <boost/preprocessor/iteration/iterate.hpp>
    #include <boost/preprocessor/facilities/intercept.hpp>
    #include <boost/preprocessor/comparison/greater.hpp>
    #include <boost/preprocessor/tuple/elem.hpp>
    #include <boost/preprocessor/tuple/to_list.hpp>
    #include <boost/preprocessor/logical/and.hpp>
    #include <boost/preprocessor/repetition/enum.hpp>
    #include <boost/preprocessor/repetition/enum_params.hpp>
    #include <boost/preprocessor/repetition/enum_trailing.hpp>
    #include <boost/preprocessor/repetition/enum_binary_params.hpp>
    #include <boost/preprocessor/repetition/enum_shifted_params.hpp>
    #include <boost/preprocessor/repetition/enum_trailing_params.hpp>
    #include <boost/preprocessor/repetition/enum_shifted_binary_params.hpp>
    #include <boost/preprocessor/repetition/enum_trailing_binary_params.hpp>
    #include <boost/preprocessor/repetition/repeat.hpp>
    #include <boost/preprocessor/repetition/repeat_from_to.hpp>
    #include <boost/preprocessor/seq/size.hpp>
    #include <boost/preprocessor/seq/enum.hpp>
    #include <boost/preprocessor/seq/seq.hpp>
    #include <boost/preprocessor/seq/to_tuple.hpp>
    #include <boost/preprocessor/seq/for_each_i.hpp>
    #include <boost/preprocessor/seq/pop_back.hpp>
    #include <boost/preprocessor/seq/push_back.hpp>
    #include <boost/preprocessor/seq/push_front.hpp>
    #include <boost/preprocessor/list/for_each_i.hpp>
    #include <boost/ref.hpp>
    #include <boost/mpl/if.hpp>
    #include <boost/mpl/assert.hpp>
    #include <boost/mpl/eval_if.hpp>
    #include <boost/utility/enable_if.hpp>
    #include <boost/type_traits/is_same.hpp>
    #include <boost/type_traits/add_const.hpp>
    #include <boost/type_traits/add_reference.hpp>
    #include <boost/type_traits/remove_cv.hpp>
    #include <boost/type_traits/remove_const.hpp>
    #include <boost/type_traits/remove_reference.hpp>
    #include <boost/proto/proto_fwd.hpp>
    #include <boost/proto/traits.hpp>
    #include <boost/proto/domain.hpp>
    #include <boost/proto/generate.hpp>
    #if BOOST_VERSION >= 103500
    # include <boost/fusion/include/at.hpp>
    # include <boost/fusion/include/value_at.hpp>
    # include <boost/fusion/include/size.hpp>
    #else
    # include <boost/spirit/fusion/sequence/at.hpp>
    # include <boost/spirit/fusion/sequence/value_at.hpp>
    # include <boost/spirit/fusion/sequence/size.hpp>
    #endif
    #include <boost/proto/detail/poly_function.hpp>
    #include <boost/proto/detail/suffix.hpp>

    #ifdef _MSC_VER
    # pragma warning(push)
    # pragma warning(disable: 4180) // qualifier applied to function type has no meaning; ignored
    #endif

    namespace boost
    {
        /// INTERNAL ONLY
        ///
        namespace fusion
        {
            /// INTERNAL ONLY
            ///
            template<typename Function>
            class unfused_generic;
        }
    }

    namespace boost { namespace proto
    {
    /// INTERNAL ONLY
    ///
    #define BOOST_PROTO_AS_CHILD_TYPE(Z, N, DATA)                                                   \
        typename boost::proto::detail::protoify_<                                                   \
            BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(3, 0, DATA), N)                                        \
          , BOOST_PP_TUPLE_ELEM(3, 2, DATA)                                                         \
        >::type                                                                                     \
        /**/

    /// INTERNAL ONLY
    ///
    #define BOOST_PROTO_AS_CHILD(Z, N, DATA)                                                        \
        boost::proto::detail::protoify_<                                                            \
            BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(3, 0, DATA), N)                                        \
          , BOOST_PP_TUPLE_ELEM(3, 2, DATA)                                                         \
        >::call(BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(3, 1, DATA), N))                                   \
        /**/

    /// INTERNAL ONLY
    ///
    # define BOOST_PROTO_AT_TYPE(Z, N, DATA)                                                        \
        typename add_const<                                                                         \
            typename fusion::BOOST_PROTO_FUSION_RESULT_OF::value_at_c<                              \
                BOOST_PP_TUPLE_ELEM(3, 0, DATA)                                                     \
              , N                                                                                   \
            >::type                                                                                 \
        >::type                                                                                     \
        /**/

    /// INTERNAL ONLY
    ///
    # define BOOST_PROTO_AT(Z, N, DATA)                                                             \
        fusion::BOOST_PROTO_FUSION_AT_C(N, BOOST_PP_TUPLE_ELEM(3, 1, DATA))                         \
        /**/

    /// INTERNAL ONLY
    ///
    #define BOOST_PROTO_AS_CHILD_AT_TYPE(Z, N, DATA)                                                \
        typename boost::proto::detail::protoify_<                                                   \
            BOOST_PROTO_AT_TYPE(Z, N, DATA)                                                         \
          , BOOST_PP_TUPLE_ELEM(3, 2, DATA)                                                         \
        >::type                                                                                     \
        /**/

    /// INTERNAL ONLY
    ///
    #define BOOST_PROTO_AS_CHILD_AT(Z, N, DATA)                                                     \
        boost::proto::detail::protoify_<                                                            \
            BOOST_PROTO_AT_TYPE(Z, N, DATA)                                                         \
          , BOOST_PP_TUPLE_ELEM(3, 2, DATA)                                                         \
        >::call(BOOST_PROTO_AT(Z, N, DATA))                                                         \
        /**/

    /// INTERNAL ONLY
    ///
    #define BOOST_PROTO_VARARG_TEMPLATE_AUX_(R, DATA, I, ELEM)                                      \
        (ELEM BOOST_PP_CAT(BOOST_PP_CAT(X, DATA), BOOST_PP_CAT(_, I)))                              \
        /**/

    /// INTERNAL ONLY
    ///
    #define BOOST_PROTO_VARARG_TEMPLATE_YES_(R, DATA, I, ELEM)                                      \
        BOOST_PP_LIST_FOR_EACH_I_R(                                                                 \
            R                                                                                       \
          , BOOST_PROTO_VARARG_TEMPLATE_AUX_                                                        \
          , I                                                                                       \
          , BOOST_PP_TUPLE_TO_LIST(                                                                 \
                BOOST_PP_DEC(BOOST_PP_SEQ_SIZE(ELEM))                                               \
              , BOOST_PP_SEQ_TO_TUPLE(BOOST_PP_SEQ_TAIL(ELEM))                                      \
            )                                                                                       \
        )                                                                                           \
        /**/

    /// INTERNAL ONLY
    ///
    #define BOOST_PROTO_VARARG_TEMPLATE_NO_(R, DATA, I, ELEM)                                       \
        /**/

    /// INTERNAL ONLY
    ///
    #define BOOST_PROTO_VARARG_TEMPLATE_(R, DATA, I, ELEM)                                          \
        BOOST_PP_IF(                                                                                \
            BOOST_PP_DEC(BOOST_PP_SEQ_SIZE(ELEM))                                                   \
          , BOOST_PROTO_VARARG_TEMPLATE_YES_                                                        \
          , BOOST_PROTO_VARARG_TEMPLATE_NO_                                                         \
        )(R, DATA, I, ELEM)                                                                         \
        /**/

    /// INTERNAL ONLY
    ///
    #define BOOST_PROTO_VARARG_TYPE_AUX_(R, DATA, I, ELEM)                                          \
        (BOOST_PP_CAT(BOOST_PP_CAT(X, DATA), BOOST_PP_CAT(_, I)))                                   \
        /**/

    /// INTERNAL ONLY
    ///
    #define BOOST_PROTO_TEMPLATE_PARAMS_YES_(R, DATA, I, ELEM)                                      \
        <                                                                                           \
            BOOST_PP_SEQ_ENUM(                                                                      \
                BOOST_PP_LIST_FOR_EACH_I_R(                                                         \
                    R                                                                               \
                  , BOOST_PROTO_VARARG_TYPE_AUX_                                                    \
                  , I                                                                               \
                  , BOOST_PP_TUPLE_TO_LIST(                                                         \
                        BOOST_PP_DEC(BOOST_PP_SEQ_SIZE(ELEM))                                       \
                      , BOOST_PP_SEQ_TO_TUPLE(BOOST_PP_SEQ_TAIL(ELEM))                              \
                    )                                                                               \
                )                                                                                   \
            )                                                                                       \
        >                                                                                           \
        /**/

    /// INTERNAL ONLY
    ///
    #define BOOST_PROTO_TEMPLATE_PARAMS_NO_(R, DATA, I, ELEM)                                       \
        /**/

    /// INTERNAL ONLY
    ///
    #define BOOST_PROTO_VARARG_TYPE_(R, DATA, I, ELEM)                                              \
        BOOST_PP_COMMA_IF(I)                                                                        \
        BOOST_PP_SEQ_HEAD(ELEM)                                                                     \
        BOOST_PP_IF(                                                                                \
            BOOST_PP_DEC(BOOST_PP_SEQ_SIZE(ELEM))                                                   \
          , BOOST_PROTO_TEMPLATE_PARAMS_YES_                                                        \
          , BOOST_PROTO_TEMPLATE_PARAMS_NO_                                                         \
        )(R, DATA, I, ELEM) BOOST_PP_EXPR_IF(BOOST_PP_GREATER(I, 1), const)                         \
        /**/

    /// INTERNAL ONLY
    ///
    #define BOOST_PROTO_VARARG_AS_EXPR_(R, DATA, I, ELEM)                                           \
        BOOST_PP_EXPR_IF(                                                                           \
            BOOST_PP_GREATER(I, 1)                                                                  \
          , ((                                                                                      \
                BOOST_PP_SEQ_HEAD(ELEM)                                                             \
                BOOST_PP_IF(                                                                        \
                    BOOST_PP_DEC(BOOST_PP_SEQ_SIZE(ELEM))                                           \
                  , BOOST_PROTO_TEMPLATE_PARAMS_YES_                                                \
                  , BOOST_PROTO_TEMPLATE_PARAMS_NO_                                                 \
                )(R, DATA, I, ELEM)()                                                               \
            ))                                                                                      \
        )                                                                                           \
        /**/

    /// INTERNAL ONLY
    ///
    #define BOOST_PROTO_VARARG_AS_CHILD_(Z, N, DATA)                                                \
        (BOOST_PP_CAT(DATA, N))                                                                     \
        /**/

    /// INTERNAL ONLY
    ///
    #define BOOST_PROTO_SEQ_PUSH_FRONT(SEQ, ELEM)                                                   \
        BOOST_PP_SEQ_POP_BACK(BOOST_PP_SEQ_PUSH_FRONT(BOOST_PP_SEQ_PUSH_BACK(SEQ, _dummy_), ELEM))  \
        /**/

    /// INTERNAL ONLY
    ///
    #define BOOST_PROTO_VARARG_AS_PARAM_(Z, N, DATA)                                                \
        (BOOST_PP_CAT(DATA, N))                                                                     \
        /**/

    /// INTERNAL ONLY
    ///
    #define BOOST_PROTO_VARARG_FUN_(Z, N, DATA)                                                     \
        template<                                                                                   \
            BOOST_PP_SEQ_ENUM(                                                                      \
                BOOST_PP_SEQ_FOR_EACH_I(                                                            \
                    BOOST_PROTO_VARARG_TEMPLATE_, ~                                                 \
                  , BOOST_PP_SEQ_PUSH_FRONT(                                                        \
                        BOOST_PROTO_SEQ_PUSH_FRONT(                                                 \
                            BOOST_PP_TUPLE_ELEM(4, 2, DATA)                                         \
                          , (BOOST_PP_TUPLE_ELEM(4, 3, DATA))                                       \
                        )                                                                           \
                      , BOOST_PP_TUPLE_ELEM(4, 1, DATA)                                             \
                    )                                                                               \
                )                                                                                   \
                BOOST_PP_REPEAT_ ## Z(N, BOOST_PROTO_VARARG_AS_PARAM_, typename A)                  \
            )                                                                                       \
        >                                                                                           \
        typename boost::proto::result_of::make_expr<                                                \
            BOOST_PP_SEQ_FOR_EACH_I(                                                                \
                BOOST_PROTO_VARARG_TYPE_, ~                                                         \
              , BOOST_PP_SEQ_PUSH_FRONT(                                                            \
                    BOOST_PROTO_SEQ_PUSH_FRONT(                                                     \
                        BOOST_PP_TUPLE_ELEM(4, 2, DATA)                                             \
                      , (BOOST_PP_TUPLE_ELEM(4, 3, DATA))                                           \
                    )                                                                               \
                  , BOOST_PP_TUPLE_ELEM(4, 1, DATA)                                                 \
                )                                                                                   \
            )                                                                                       \
            BOOST_PP_ENUM_TRAILING_BINARY_PARAMS_Z(Z, N, A, const & BOOST_PP_INTERCEPT)             \
        >::type const                                                                               \
        BOOST_PP_TUPLE_ELEM(4, 0, DATA)(BOOST_PP_ENUM_BINARY_PARAMS_Z(Z, N, A, const &a))           \
        {                                                                                           \
            return boost::proto::detail::make_expr_<                                                \
                BOOST_PP_SEQ_FOR_EACH_I(                                                            \
                    BOOST_PROTO_VARARG_TYPE_, ~                                                     \
                  , BOOST_PP_SEQ_PUSH_FRONT(                                                        \
                        BOOST_PROTO_SEQ_PUSH_FRONT(                                                 \
                            BOOST_PP_TUPLE_ELEM(4, 2, DATA)                                         \
                          , (BOOST_PP_TUPLE_ELEM(4, 3, DATA))                                       \
                        )                                                                           \
                      , BOOST_PP_TUPLE_ELEM(4, 1, DATA)                                             \
                    )                                                                               \
                )                                                                                   \
                BOOST_PP_ENUM_TRAILING_BINARY_PARAMS_Z(Z, N, A, const & BOOST_PP_INTERCEPT)         \
            >()(                                                                                    \
                BOOST_PP_SEQ_ENUM(                                                                  \
                    BOOST_PP_SEQ_FOR_EACH_I(                                                        \
                        BOOST_PROTO_VARARG_AS_EXPR_, ~                                              \
                      , BOOST_PP_SEQ_PUSH_FRONT(                                                    \
                            BOOST_PROTO_SEQ_PUSH_FRONT(                                             \
                                BOOST_PP_TUPLE_ELEM(4, 2, DATA)                                     \
                              , (BOOST_PP_TUPLE_ELEM(4, 3, DATA))                                   \
                            )                                                                       \
                          , BOOST_PP_TUPLE_ELEM(4, 1, DATA)                                         \
                        )                                                                           \
                    )                                                                               \
                    BOOST_PP_REPEAT_ ## Z(N, BOOST_PROTO_VARARG_AS_CHILD_, a)                       \
                )                                                                                   \
            );                                                                                      \
        }                                                                                           \
        /**/

    /// \code
    /// BOOST_PROTO_DEFINE_FUNCTION_TEMPLATE(
    ///     1
    ///   , construct
    ///   , boost::proto::default_domain
    ///   , (boost::proto::tag::function)
    ///   , ((op::construct)(typename)(int))
    /// )
    /// \endcode
    #define BOOST_PROTO_DEFINE_FUNCTION_TEMPLATE(ARGCOUNT, NAME, DOMAIN, TAG, BOUNDARGS)            \
        BOOST_PP_REPEAT_FROM_TO(                                                                    \
            ARGCOUNT                                                                                \
          , BOOST_PP_INC(ARGCOUNT)                                                                  \
          , BOOST_PROTO_VARARG_FUN_                                                                 \
          , (NAME, TAG, BOUNDARGS, DOMAIN)                                                          \
        )\
        /**/

    /// \code
    /// BOOST_PROTO_DEFINE_VARARG_FUNCTION_TEMPLATE(
    ///     construct
    ///   , boost::proto::default_domain
    ///   , (boost::proto::tag::function)
    ///   , ((op::construct)(typename)(int))
    /// )
    /// \endcode
    #define BOOST_PROTO_DEFINE_VARARG_FUNCTION_TEMPLATE(NAME, DOMAIN, TAG, BOUNDARGS)               \
        BOOST_PP_REPEAT(                                                                            \
            BOOST_PP_SUB(BOOST_PP_INC(BOOST_PROTO_MAX_ARITY), BOOST_PP_SEQ_SIZE(BOUNDARGS))         \
          , BOOST_PROTO_VARARG_FUN_                                                                 \
          , (NAME, TAG, BOUNDARGS, DOMAIN)                                                          \
        )                                                                                           \
        /**/

        namespace detail
        {
            template<typename T, typename Domain>
            struct protoify_
            {
                typedef
                    typename boost::unwrap_reference<T>::type
                unref_type;

                typedef
                    typename mpl::eval_if<
                        boost::is_reference_wrapper<T>
                      , proto::result_of::as_child<unref_type, Domain>
                      , proto::result_of::as_expr<unref_type, Domain>
                    >::type
                type;

                static type call(T &t)
                {
                    return typename mpl::if_<
                        is_reference_wrapper<T>
                      , functional::as_child<Domain>
                      , functional::as_expr<Domain>
                    >::type()(static_cast<unref_type &>(t));
                }
            };

            template<typename T, typename Domain>
            struct protoify_<T &, Domain>
            {
                typedef
                    typename proto::result_of::as_child<T, Domain>::type
                type;

                static type call(T &t)
                {
                    return functional::as_child<Domain>()(t);
                }
            };

            template<
                int Index
                BOOST_PP_ENUM_TRAILING_BINARY_PARAMS(
                    BOOST_PROTO_MAX_ARITY
                  , typename D
                  , = void BOOST_PP_INTERCEPT
                )
            >
            struct select_nth
            {
                BOOST_MPL_ASSERT_MSG((false), PROTO_DOMAIN_MISMATCH, (select_nth));
                typedef default_domain type;
            };

            template<typename Void = void>
            struct deduce_domain0
            {
                typedef default_domain type;
            };

            template<int I>
            struct sized
            {
                char buffer[I];
            };

            template<typename T>
            struct nondeduced_domain
            {
                typedef nondeduced_domain type;
                nondeduced_domain(T);
                nondeduced_domain(default_domain);
            };

            template<>
            struct nondeduced_domain<default_domain>
            {
                typedef nondeduced_domain type;
                nondeduced_domain(default_domain);
            };

            template<typename Tag, typename Domain, typename Sequence, std::size_t Size>
            struct unpack_expr_
            {};

            template<typename Domain, typename Sequence>
            struct unpack_expr_<tag::terminal, Domain, Sequence, 1u>
            {
                typedef
                    typename add_const<
                        typename fusion::BOOST_PROTO_FUSION_RESULT_OF::value_at_c<
                            Sequence
                          , 0
                        >::type
                    >::type
                terminal_type;

                typedef
                    typename proto::detail::protoify_<
                        terminal_type
                      , Domain
                    >::type
                type;

                static type const call(Sequence const &sequence)
                {
                    return proto::detail::protoify_<terminal_type, Domain>::call(fusion::BOOST_PROTO_FUSION_AT_C(0, sequence));
                }
            };

            template<typename Sequence>
            struct unpack_expr_<tag::terminal, deduce_domain, Sequence, 1u>
              : unpack_expr_<tag::terminal, default_domain, Sequence, 1u>
            {};

            template<
                typename Tag
              , typename Domain
                BOOST_PP_ENUM_TRAILING_BINARY_PARAMS(
                    BOOST_PROTO_MAX_ARITY
                  , typename A
                  , = void BOOST_PP_INTERCEPT
                )
              , typename _ = void
            >
            struct make_expr_
            {};

            template<typename Domain, typename A>
            struct make_expr_<tag::terminal, Domain, A
                BOOST_PP_ENUM_TRAILING_PARAMS(BOOST_PROTO_MAX_ARITY, void BOOST_PP_INTERCEPT)>
            {
                typedef typename proto::detail::protoify_<A, Domain>::type result_type;

                result_type operator()(typename add_reference<A>::type a) const
                {
                    return proto::detail::protoify_<A, Domain>::call(a);
                }
            };

            template<typename A>
            struct make_expr_<tag::terminal, deduce_domain, A
                BOOST_PP_ENUM_TRAILING_PARAMS(BOOST_PROTO_MAX_ARITY, void BOOST_PP_INTERCEPT)>
              : make_expr_<tag::terminal, default_domain, A>
            {};

            template<typename Base, typename Expr>
            Expr implicit_expr_wrap(Base const &expr, mpl::false_, Expr *)
            {
                return Expr(expr);
            }

            template<typename Base, typename Expr>
            Expr implicit_expr_wrap(Base const &expr, mpl::true_, Expr *)
            {
                Expr that = {expr};
                return that;
            }

            template<typename A0, typename Void = void>
            struct implicit_expr_1
            {
                A0 &a0;

                template<typename Args>
                operator proto::expr<tag::terminal, Args, 0>() const
                {
                    proto::expr<tag::terminal, Args, 0> that = {this->a0};
                    return that;
                };

                template<typename Expr>
                operator Expr() const
                {
                    typename Expr::proto_base_expr that = *this;
                    return detail::implicit_expr_wrap(that, is_aggregate<Expr>(), static_cast<Expr *>(0));
                }
            };

            template<typename A0>
            struct implicit_expr_1<A0, typename A0::proto_is_expr_>
            {
                A0 &a0;

            #if BOOST_WORKAROUND(BOOST_INTEL_CXX_VERSION, BOOST_TESTED_AT(1010))
                typedef typename remove_cv<A0>::type uncv_a0_type;

                operator uncv_a0_type &() const
                {
                    return const_cast<uncv_a0_type &>(this->a0);
                }
            #else
                operator A0 &() const
                {
                    return this->a0;
                }
            #endif

                template<typename Tag, typename Args>
                operator proto::expr<Tag, Args, 1>() const
                {
                    proto::expr<Tag, Args, 1> that = {this->a0};
                    return that;
                };

                template<typename Expr>
                operator Expr() const
                {
                    typename Expr::proto_base_expr that = *this;
                    return detail::implicit_expr_wrap(that, is_aggregate<Expr>(), static_cast<Expr *>(0));
                }
            };

        #define BOOST_PP_ITERATION_PARAMS_1                                                         \
            (4, (1, BOOST_PROTO_MAX_ARITY, <boost/proto/make_expr.hpp>, 1))                         \
            /**/

        #include BOOST_PP_ITERATE()
        }

        namespace result_of
        {
            /// \brief Metafunction that computes the return type of the
            /// \c make_expr() function, with a domain deduced from the
            /// domains of the children.
            ///
            /// Use the <tt>result_of::make_expr\<\></tt> metafunction to
            /// compute the return type of the \c make_expr() function.
            ///
            /// In this specialization, the domain is deduced from the
            /// domains of the child types. (If
            /// <tt>is_domain\<A0\>::::value</tt> is \c true, then another
            /// specialization is selected.)
            template<
                typename Tag
              , typename A0
              , BOOST_PP_ENUM_SHIFTED_BINARY_PARAMS(
                    BOOST_PROTO_MAX_ARITY
                  , typename A
                  , BOOST_PROTO_WHEN_BUILDING_DOCS(= void) BOOST_PP_INTERCEPT
                )
              , typename Void1  BOOST_PROTO_WHEN_BUILDING_DOCS(= void)
              , typename Void2  BOOST_PROTO_WHEN_BUILDING_DOCS(= void)
            >
            struct make_expr
            {
                /// Same as <tt>result_of::make_expr\<Tag, D, A0, ... AN\>::::type</tt>
                /// where \c D is the deduced domain, which is calculated as follows:
                ///
                /// For each \c x in <tt>[0,N)</tt> (proceeding in order beginning with
                /// <tt>x=0</tt>), if <tt>domain_of\<Ax\>::::type</tt> is not
                /// \c default_domain, then \c D is <tt>domain_of\<Ax\>::::type</tt>.
                /// Otherwise, \c D is \c default_domain.
                typedef
                    typename detail::make_expr_<
                        Tag
                      , deduce_domain
                        BOOST_PP_ENUM_TRAILING_PARAMS(BOOST_PROTO_MAX_ARITY, A)
                    >::result_type
                type;
            };

            /// \brief Metafunction that computes the return type of the
            /// \c make_expr() function, within the specified domain.
            ///
            /// Use the <tt>result_of::make_expr\<\></tt> metafunction to compute
            /// the return type of the \c make_expr() function.
            template<
                typename Tag
              , typename Domain
                BOOST_PP_ENUM_TRAILING_PARAMS(BOOST_PROTO_MAX_ARITY, typename A)
            >
            struct make_expr<
                Tag
              , Domain
                BOOST_PP_ENUM_TRAILING_PARAMS(BOOST_PROTO_MAX_ARITY, A)
              , typename Domain::proto_is_domain_
            >
            {
                /// If \c Tag is <tt>tag::terminal</tt>, then \c type is a
                /// typedef for <tt>boost::result_of\<Domain(expr\<tag::terminal,
                /// term\<A0\> \>)\>::::type</tt>.
                ///
                /// Otherwise, \c type is a typedef for <tt>boost::result_of\<Domain(expr\<Tag,
                /// listN\< as_child\<A0\>::::type, ... as_child\<AN\>::::type\>)
                /// \>::::type</tt>, where \c N is the number of non-void template
                /// arguments, and <tt>as_child\<A\>::::type</tt> is evaluated as
                /// follows:
                ///
                /// \li If <tt>is_expr\<A\>::::value</tt> is \c true, then the
                /// child type is \c A.
                /// \li If \c A is <tt>B &</tt> or <tt>cv boost::reference_wrapper\<B\></tt>,
                /// and <tt>is_expr\<B\>::::value</tt> is \c true, then the
                /// child type is <tt>B &</tt>.
                /// \li If <tt>is_expr\<A\>::::value</tt> is \c false, then the
                /// child type is <tt>boost::result_of\<Domain(expr\<tag::terminal, term\<A\> \>
                /// )\>::::type</tt>.
                /// \li If \c A is <tt>B &</tt> or <tt>cv boost::reference_wrapper\<B\></tt>,
                /// and <tt>is_expr\<B\>::::value</tt> is \c false, then the
                /// child type is <tt>boost::result_of\<Domain(expr\<tag::terminal, term\<B &\> \>
                /// )\>::::type</tt>.
                typedef
                    typename detail::make_expr_<
                        Tag
                      , Domain
                        BOOST_PP_ENUM_TRAILING_PARAMS(BOOST_PROTO_MAX_ARITY, A)
                    >::result_type
                type;
            };

            /// \brief Metafunction that computes the return type of the
            /// \c unpack_expr() function, with a domain deduced from the
            /// domains of the children.
            ///
            /// Use the <tt>result_of::unpack_expr\<\></tt> metafunction to
            /// compute the return type of the \c unpack_expr() function.
            ///
            /// \c Sequence is a Fusion Random Access Sequence.
            ///
            /// In this specialization, the domain is deduced from the
            /// domains of the child types. (If
            /// <tt>is_domain\<Sequence>::::value</tt> is \c true, then another
            /// specialization is selected.)
            template<
                typename Tag
              , typename Sequence
              , typename Void1  BOOST_PROTO_WHEN_BUILDING_DOCS(= void)
              , typename Void2  BOOST_PROTO_WHEN_BUILDING_DOCS(= void)
            >
            struct unpack_expr
            {
                /// Same as <tt>result_of::make_expr\<Tag,
                /// fusion::value_at\<Sequence, 0\>::::type, ...
                /// fusion::value_at\<Sequence, N-1\>::::type\>::::type</tt>,
                /// where \c N is the size of \c Sequence.
                typedef
                    typename detail::unpack_expr_<
                        Tag
                      , deduce_domain
                      , Sequence
                      , fusion::BOOST_PROTO_FUSION_RESULT_OF::size<Sequence>::type::value
                    >::type
                type;
            };

            /// \brief Metafunction that computes the return type of the
            /// \c unpack_expr() function, within the specified domain.
            ///
            /// Use the <tt>result_of::make_expr\<\></tt> metafunction to compute
            /// the return type of the \c make_expr() function.
            template<typename Tag, typename Domain, typename Sequence>
            struct unpack_expr<Tag, Domain, Sequence, typename Domain::proto_is_domain_>
            {
                /// Same as <tt>result_of::make_expr\<Tag, Domain,
                /// fusion::value_at\<Sequence, 0\>::::type, ...
                /// fusion::value_at\<Sequence, N-1\>::::type\>::::type</tt>,
                /// where \c N is the size of \c Sequence.
                typedef
                    typename detail::unpack_expr_<
                        Tag
                      , Domain
                      , Sequence
                      , fusion::BOOST_PROTO_FUSION_RESULT_OF::size<Sequence>::type::value
                    >::type
                type;
            };
        }

        namespace functional
        {
            /// \brief A callable function object equivalent to the
            /// \c proto::make_expr() function.
            ///
            /// In all cases, <tt>functional::make_expr\<Tag, Domain\>()(a0, ... aN)</tt>
            /// is equivalent to <tt>proto::make_expr\<Tag, Domain\>(a0, ... aN)</tt>.
            ///
            /// <tt>functional::make_expr\<Tag\>()(a0, ... aN)</tt>
            /// is equivalent to <tt>proto::make_expr\<Tag\>(a0, ... aN)</tt>.
            template<typename Tag, typename Domain  BOOST_PROTO_WHEN_BUILDING_DOCS(= deduce_domain)>
            struct make_expr
            {
                BOOST_PROTO_CALLABLE()
                BOOST_PROTO_POLY_FUNCTION()

                template<typename Sig>
                struct result;

                template<typename This, typename A0>
                struct result<This(A0)>
                {
                    typedef
                        typename result_of::make_expr<
                            Tag
                          , Domain
                          , A0
                        >::type
                    type;
                };

                /// Construct an expression node with tag type \c Tag
                /// and in the domain \c Domain.
                ///
                /// \return <tt>proto::make_expr\<Tag, Domain\>(a0,...aN)</tt>
                template<typename A0>
                typename result_of::make_expr<
                    Tag
                  , Domain
                  , A0 const
                >::type
                operator ()(A0 const &a0) const
                {
                    return proto::detail::make_expr_<
                        Tag
                      , Domain
                      , A0 const
                    >()(a0);
                }

                // Additional overloads generated by the preprocessor ...

            #define BOOST_PP_ITERATION_PARAMS_1                                                     \
                (4, (2, BOOST_PROTO_MAX_ARITY, <boost/proto/make_expr.hpp>, 2))                     \
                /**/

            #include BOOST_PP_ITERATE()

                /// INTERNAL ONLY
                ///
                template<
                    BOOST_PP_ENUM_BINARY_PARAMS(
                        BOOST_PROTO_MAX_ARITY
                      , typename A
                      , = void BOOST_PP_INTERCEPT
                    )
                >
                struct impl
                  : detail::make_expr_<
                      Tag
                    , Domain
                      BOOST_PP_ENUM_TRAILING_PARAMS(BOOST_PROTO_MAX_ARITY, A)
                    >
                {};
            };

            /// \brief A callable function object equivalent to the
            /// \c proto::unpack_expr() function.
            ///
            /// In all cases, <tt>functional::unpack_expr\<Tag, Domain\>()(seq)</tt>
            /// is equivalent to <tt>proto::unpack_expr\<Tag, Domain\>(seq)</tt>.
            ///
            /// <tt>functional::unpack_expr\<Tag\>()(seq)</tt>
            /// is equivalent to <tt>proto::unpack_expr\<Tag\>(seq)</tt>.
            template<typename Tag, typename Domain  BOOST_PROTO_WHEN_BUILDING_DOCS(= deduce_domain)>
            struct unpack_expr
            {
                BOOST_PROTO_CALLABLE()

                template<typename Sig>
                struct result
                {};

                template<typename This, typename Sequence>
                struct result<This(Sequence)>
                {
                    typedef
                        typename result_of::unpack_expr<
                            Tag
                          , Domain
                          , typename remove_reference<Sequence>::type
                        >::type
                    type;
                };

                /// Construct an expression node with tag type \c Tag
                /// and in the domain \c Domain.
                ///
                /// \param sequence A Fusion Random Access Sequence
                /// \return <tt>proto::unpack_expr\<Tag, Domain\>(sequence)</tt>
                template<typename Sequence>
                typename result_of::unpack_expr<Tag, Domain, Sequence const>::type
                operator ()(Sequence const &sequence) const
                {
                    return proto::detail::unpack_expr_<
                        Tag
                      , Domain
                      , Sequence const
                      , fusion::BOOST_PROTO_FUSION_RESULT_OF::size<Sequence>::type::value
                    >::call(sequence);
                }
            };

            /// INTERNAL ONLY
            ///
            template<typename Tag, typename Domain>
            struct unfused_expr_fun
            {
                BOOST_PROTO_CALLABLE()

                template<typename Sig>
                struct result;

                template<typename This, typename Sequence>
                struct result<This(Sequence)>
                {
                    typedef
                        typename result_of::unpack_expr<
                            Tag
                          , Domain
                          , typename remove_reference<Sequence>::type
                        >::type
                    type;
                };

                template<typename Sequence>
                typename proto::result_of::unpack_expr<Tag, Domain, Sequence const>::type
                operator ()(Sequence const &sequence) const
                {
                    return proto::detail::unpack_expr_<
                        Tag
                      , Domain
                      , Sequence const
                      , fusion::BOOST_PROTO_FUSION_RESULT_OF::size<Sequence>::type::value
                    >::call(sequence);
                }
            };

            /// INTERNAL ONLY
            ///
            template<typename Tag, typename Domain>
            struct unfused_expr
              : fusion::unfused_generic<unfused_expr_fun<Tag, Domain> >
            {
                BOOST_PROTO_CALLABLE()
            };

        } // namespace functional

        /// \brief Construct an expression of the requested tag type
        /// with a domain and with the specified arguments as children.
        ///
        /// This function template may be invoked either with or without
        /// specifying a \c Domain argument. If no domain is specified,
        /// the domain is deduced by examining in order the domains of
        /// the given arguments and taking the first that is not
        /// \c default_domain, if any such domain exists, or
        /// \c default_domain otherwise.
        ///
        /// Let \c wrap_(x) be defined such that:
        /// \li If \c x is a <tt>boost::reference_wrapper\<\></tt>,
        /// \c wrap_(x) is equivalent to <tt>as_child\<Domain\>(x.get())</tt>.
        /// \li Otherwise, \c wrap_(x) is equivalent to
        /// <tt>as_expr\<Domain\>(x)</tt>.
        ///
        /// Let <tt>make_\<Tag\>(b0,...bN)</tt> be defined as
        /// <tt>expr\<Tag, listN\<B0,...BN\> \>::::make(b0,...bN)</tt>
        /// where \c Bx is the type of \c bx.
        ///
        /// \return <tt>Domain()(make_\<Tag\>(wrap_(a0),...wrap_(aN)))</tt>.
        template<typename Tag, typename A0>
        typename lazy_disable_if<
            is_domain<A0>
          , result_of::make_expr<
                Tag
              , A0 const
            >
        >::type const
        make_expr(A0 const &a0)
        {
            return proto::detail::make_expr_<
                Tag
              , deduce_domain
              , A0 const
            >()(a0);
        }

        /// \overload
        ///
        template<typename Tag, typename Domain, typename B0>
        typename result_of::make_expr<
            Tag
          , Domain
          , B0 const
        >::type const
        make_expr(B0 const &b0)
        {
            return proto::detail::make_expr_<
                Tag
              , Domain
              , B0 const
            >()(b0);
        }

        // Additional overloads generated by the preprocessor...

    #define BOOST_PP_ITERATION_PARAMS_1                                                             \
        (4, (2, BOOST_PROTO_MAX_ARITY, <boost/proto/make_expr.hpp>, 3))                             \
        /**/

    #include BOOST_PP_ITERATE()

        /// \brief Construct an expression of the requested tag type
        /// with a domain and with childres from the specified Fusion
        /// Random Access Sequence.
        ///
        /// This function template may be invoked either with or without
        /// specifying a \c Domain argument. If no domain is specified,
        /// the domain is deduced by examining in order the domains of the
        /// elements of \c sequence and taking the first that is not
        /// \c default_domain, if any such domain exists, or
        /// \c default_domain otherwise.
        ///
        /// Let <tt>wrap_\<N\>(s)</tt>, where \c s has type \c S, be defined
        /// such that:
        /// \li If <tt>fusion::value_at\<S,N\>::::type</tt> is a reference,
        /// <tt>wrap_\<N\>(s)</tt> is equivalent to
        /// <tt>as_child\<Domain\>(fusion::at_c\<N\>(s))</tt>.
        /// \li Otherwise, <tt>wrap_\<N\>(s)</tt> is equivalent to
        /// <tt>as_expr\<Domain\>(fusion::at_c\<N\>(s))</tt>.
        ///
        /// Let <tt>make_\<Tag\>(b0,...bN)</tt> be defined as
        /// <tt>expr\<Tag, listN\<B0,...BN\> \>::::make(b0,...bN)</tt>
        /// where \c Bx is the type of \c bx.
        ///
        /// \param sequence a Fusion Random Access Sequence.
        /// \return <tt>Domain()(make_\<Tag\>(wrap_\<0\>(s),...wrap_\<N-1\>(S)))</tt>,
        /// where N is the size of \c Sequence.
        template<typename Tag, typename Sequence>
        typename lazy_disable_if<
            is_domain<Sequence>
          , result_of::unpack_expr<Tag, Sequence const>
        >::type const
        unpack_expr(Sequence const &sequence)
        {
            return proto::detail::unpack_expr_<
                Tag
              , deduce_domain
              , Sequence const
              , fusion::BOOST_PROTO_FUSION_RESULT_OF::size<Sequence>::type::value
            >::call(sequence);
        }

        /// \overload
        ///
        template<typename Tag, typename Domain, typename Sequence2>
        typename result_of::unpack_expr<Tag, Domain, Sequence2 const>::type const
        unpack_expr(Sequence2 const &sequence2)
        {
            return proto::detail::unpack_expr_<
                Tag
              , Domain
              , Sequence2 const
              , fusion::BOOST_PROTO_FUSION_RESULT_OF::size<Sequence2>::type::value
            >::call(sequence2);
        }

        /// \brief Return a proxy object that holds its arguments by reference
        /// and is implicitly convertible to an expression.
        template<typename A0>
        detail::implicit_expr_1<A0> const
        implicit_expr(A0 &a0)
        {
            detail::implicit_expr_1<A0> that = {a0};
            return that;
        }

        // Additional overloads generated by the preprocessor...

    #define BOOST_PP_ITERATION_PARAMS_1                                                             \
        (4, (2, BOOST_PROTO_MAX_ARITY, <boost/proto/make_expr.hpp>, 4))                             \
        /**/

    #include BOOST_PP_ITERATE()

        /// INTERNAL ONLY
        ///
        template<typename Tag, typename Domain>
        struct is_callable<functional::make_expr<Tag, Domain> >
          : mpl::true_
        {};

        /// INTERNAL ONLY
        ///
        template<typename Tag, typename Domain>
        struct is_callable<functional::unpack_expr<Tag, Domain> >
          : mpl::true_
        {};

        /// INTERNAL ONLY
        ///
        template<typename Tag, typename Domain>
        struct is_callable<functional::unfused_expr<Tag, Domain> >
          : mpl::true_
        {};

    }}

    #ifdef _MSC_VER
    # pragma warning(pop)
    #endif

    #undef BOOST_PROTO_AT
    #undef BOOST_PROTO_AT_TYPE
    #undef BOOST_PROTO_AS_CHILD_AT
    #undef BOOST_PROTO_AS_CHILD_AT_TYPE

    #endif // BOOST_PROTO_MAKE_EXPR_HPP_EAN_04_01_2005

#elif BOOST_PP_ITERATION_FLAGS() == 1

    #define N BOOST_PP_ITERATION()
    #define M BOOST_PP_SUB(BOOST_PROTO_MAX_ARITY, N)

    #if N > 1
        template<BOOST_PP_ENUM_PARAMS(N, typename A)>
        struct BOOST_PP_CAT(implicit_expr_, N)
        {
            #define M0(Z, N, DATA) BOOST_PP_CAT(A, N) &BOOST_PP_CAT(a, N);
            BOOST_PP_REPEAT(N, M0, ~)
            #undef M0

            template<typename Tag, typename Args>
            operator proto::expr<Tag, Args, N>() const
            {
                #define M0(Z, N, DATA)                                                              \
                    implicit_expr_1<BOOST_PP_CAT(A, N)> BOOST_PP_CAT(b, N)                          \
                        = {this->BOOST_PP_CAT(a, N)};                                               \
                    typename Args::BOOST_PP_CAT(child, N) BOOST_PP_CAT(c, N) = BOOST_PP_CAT(b, N);  \
                    /**/
                BOOST_PP_REPEAT(N, M0, ~)
                #undef M0
                proto::expr<Tag, Args, N> that = {BOOST_PP_ENUM_PARAMS(N, c)};
                return that;
            };

            template<typename Expr>
            operator Expr() const
            {
                typename Expr::proto_base_expr that = *this;
                return detail::implicit_expr_wrap(that, is_aggregate<Expr>(), static_cast<Expr *>(0));
            }
        };
    #endif

        template<BOOST_PP_ENUM_PARAMS(BOOST_PROTO_MAX_ARITY, typename T)>
        struct select_nth<BOOST_PP_DEC(N), BOOST_PP_ENUM_PARAMS(BOOST_PROTO_MAX_ARITY, T)>
        {
            typedef BOOST_PP_CAT(T, BOOST_PP_DEC(N)) type;
        };

        // Use function overloading as an efficient mechanism for
        // calculating the domain shared by a bunch of proto expressions
        // (or non-expressions, assumed to be in the default_domain).
        // The domain of a set of domains S is deduced as follows:
        // - If S contains only default_domain, the deduced domain is
        //   default_domain.
        // - If S contains only X and default_domain, the deduced domain
        //   is X.
        // - If S contains different domains X and Y, neither of which is
        //   default_domain, it is an error.
        template<BOOST_PP_ENUM_PARAMS(N, typename A)>
        struct BOOST_PP_CAT(deduce_domain, N)
        {
            #if BOOST_WORKAROUND(BOOST_MSVC, == 1310)
            // The function overloading trick doesn't work on MSVC-7.1, so
            // do it the hard (expensive) way.
            typedef
                typename mpl::eval_if_c<
                    is_same<typename domain_of<A0>::type, default_domain>::value
                  , BOOST_PP_CAT(deduce_domain, BOOST_PP_DEC(N))<BOOST_PP_ENUM_SHIFTED_PARAMS(N, A)>
                  , domain_of<A0>
                >::type
            type;
            #else
            #define M0(N, F) char (&F)[BOOST_PP_INC(N)]
            static M0(BOOST_PROTO_MAX_ARITY, deducer(
                BOOST_PP_ENUM_PARAMS(N, dont_care BOOST_PP_INTERCEPT)));
            #define M1(Z, X, DATA)                                                                  \
            typedef typename domain_of<BOOST_PP_CAT(A, X)>::type BOOST_PP_CAT(D, X);                \
            static BOOST_PP_CAT(D, X) &BOOST_PP_CAT(d, X);                                          \
            template<typename T>                                                                    \
            static M0(X, deducer(                                                                   \
                BOOST_PP_ENUM_PARAMS_Z(Z, X, default_domain BOOST_PP_INTERCEPT)                     \
                BOOST_PP_COMMA_IF(X) T                                                              \
                BOOST_PP_ENUM_TRAILING_PARAMS_Z(                                                    \
                    Z                                                                               \
                  , BOOST_PP_DEC(BOOST_PP_SUB(N, X))                                                \
                  , typename nondeduced_domain<T>::type BOOST_PP_INTERCEPT                          \
                )                                                                                   \
            ));
            BOOST_PP_REPEAT(N, M1, ~)
            #undef M0
            #undef M1
            BOOST_STATIC_CONSTANT(int, value = sizeof(deducer(BOOST_PP_ENUM_PARAMS(N, d))) - 1);
            typedef typename select_nth<value, BOOST_PP_ENUM_PARAMS(N, D)>::type type;
            #endif
        };

        template<typename Tag, typename Domain BOOST_PP_ENUM_TRAILING_PARAMS(N, typename A)>
        struct make_expr_<Tag, Domain BOOST_PP_ENUM_TRAILING_PARAMS(N, A)
            BOOST_PP_ENUM_TRAILING_PARAMS(M, void BOOST_PP_INTERCEPT), void>
        {
            typedef proto::expr<
                Tag
              , BOOST_PP_CAT(list, N)<BOOST_PP_ENUM(N, BOOST_PROTO_AS_CHILD_TYPE, (A, ~, Domain)) >
            > expr_type;

            typedef typename Domain::template result<void(expr_type)>::type result_type;

            result_type operator()(BOOST_PP_ENUM_BINARY_PARAMS(N, typename add_reference<A, >::type a)) const
            {
                expr_type that = {
                    BOOST_PP_ENUM(N, BOOST_PROTO_AS_CHILD, (A, a, Domain))
                };
                return Domain()(that);
            }
        };

        template<typename Tag BOOST_PP_ENUM_TRAILING_PARAMS(N, typename A)>
        struct make_expr_<Tag, deduce_domain BOOST_PP_ENUM_TRAILING_PARAMS(N, A)
            BOOST_PP_ENUM_TRAILING_PARAMS(M, void BOOST_PP_INTERCEPT), void>
          : make_expr_<
                Tag
              , typename BOOST_PP_CAT(deduce_domain, N)<BOOST_PP_ENUM_PARAMS(N, A)>::type
                BOOST_PP_ENUM_TRAILING_PARAMS(N, A)
            >
        {};

        template<typename Tag, typename Domain, typename Sequence>
        struct unpack_expr_<Tag, Domain, Sequence, N>
        {
            typedef proto::expr<
                Tag
              , BOOST_PP_CAT(list, N)<
                    BOOST_PP_ENUM(N, BOOST_PROTO_AS_CHILD_AT_TYPE, (Sequence const, ~, Domain))
                >
            > expr_type;

            typedef typename Domain::template result<void(expr_type)>::type type;

            static type const call(Sequence const &sequence)
            {
                expr_type that = {
                    BOOST_PP_ENUM(N, BOOST_PROTO_AS_CHILD_AT, (Sequence const, sequence, Domain))
                };
                return Domain()(that);
            }
        };

        template<typename Tag, typename Sequence>
        struct unpack_expr_<Tag, deduce_domain, Sequence, N>
          : unpack_expr_<
                Tag
              , typename BOOST_PP_CAT(deduce_domain, N)<
                    BOOST_PP_ENUM(N, BOOST_PROTO_AT_TYPE, (Sequence const, ~, ~))
                >::type
              , Sequence
              , N
            >
        {};

    #undef N
    #undef M

#elif BOOST_PP_ITERATION_FLAGS() == 2

    #define N BOOST_PP_ITERATION()

        template<typename This BOOST_PP_ENUM_TRAILING_PARAMS(N, typename A)>
        struct result<This(BOOST_PP_ENUM_PARAMS(N, A))>
        {
            typedef
                typename result_of::make_expr<
                    Tag
                  , Domain
                    BOOST_PP_ENUM_TRAILING_PARAMS(N, A)
                >::type
            type;
        };

        /// \overload
        ///
        template<BOOST_PP_ENUM_PARAMS(N, typename A)>
        typename result_of::make_expr<
            Tag
          , Domain
            BOOST_PP_ENUM_TRAILING_PARAMS(N, const A)
        >::type
        operator ()(BOOST_PP_ENUM_BINARY_PARAMS(N, const A, &a)) const
        {
            return proto::detail::make_expr_<
                Tag
              , Domain
                BOOST_PP_ENUM_TRAILING_PARAMS(N, const A)
            >()(BOOST_PP_ENUM_PARAMS(N, a));
        }

    #undef N

#elif BOOST_PP_ITERATION_FLAGS() == 3

    #define N BOOST_PP_ITERATION()

        /// \overload
        ///
        template<typename Tag BOOST_PP_ENUM_TRAILING_PARAMS(N, typename A)>
        typename lazy_disable_if<
            is_domain<A0>
          , result_of::make_expr<
                Tag
                BOOST_PP_ENUM_TRAILING_PARAMS(N, const A)
            >
        >::type const
        make_expr(BOOST_PP_ENUM_BINARY_PARAMS(N, const A, &a))
        {
            return proto::detail::make_expr_<
                Tag
              , deduce_domain
                BOOST_PP_ENUM_TRAILING_PARAMS(N, const A)
            >()(BOOST_PP_ENUM_PARAMS(N, a));
        }

        /// \overload
        ///
        template<typename Tag, typename Domain BOOST_PP_ENUM_TRAILING_PARAMS(N, typename B)>
        typename result_of::make_expr<
            Tag
          , Domain
            BOOST_PP_ENUM_TRAILING_PARAMS(N, const B)
        >::type const
        make_expr(BOOST_PP_ENUM_BINARY_PARAMS(N, const B, &b))
        {
            return proto::detail::make_expr_<
                Tag
              , Domain
                BOOST_PP_ENUM_TRAILING_PARAMS(N, const B)
            >()(BOOST_PP_ENUM_PARAMS(N, b));
        }

    #undef N

#elif BOOST_PP_ITERATION_FLAGS() == 4

    #define N BOOST_PP_ITERATION()

        /// \overload
        ///
        template<BOOST_PP_ENUM_PARAMS(N, typename A)>
        detail::BOOST_PP_CAT(implicit_expr_, N)<BOOST_PP_ENUM_PARAMS(N, A)> const
        implicit_expr(BOOST_PP_ENUM_BINARY_PARAMS(N, A, &a))
        {
            detail::BOOST_PP_CAT(implicit_expr_, N)<BOOST_PP_ENUM_PARAMS(N, A)> that
                = {BOOST_PP_ENUM_PARAMS(N, a)};
            return that;
        }

    #undef N

#endif // BOOST_PP_IS_ITERATING
