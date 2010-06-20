///////////////////////////////////////////////////////////////////////////////
/// \file extends.hpp
/// Macros and a base class for defining end-user expression types
//
//  Copyright 2008 Eric Niebler. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PROTO_EXTENDS_HPP_EAN_11_1_2006
#define BOOST_PROTO_EXTENDS_HPP_EAN_11_1_2006

#include <boost/xpressive/proto/detail/prefix.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/control/if.hpp>
#include <boost/preprocessor/arithmetic/inc.hpp>
#include <boost/preprocessor/arithmetic/dec.hpp>
#include <boost/preprocessor/iteration/local.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/repeat_from_to.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>
#include <boost/preprocessor/repetition/enum_trailing_params.hpp>
#include <boost/preprocessor/repetition/enum_trailing_binary_params.hpp>
#include <boost/mpl/apply_wrap.hpp>
#include <boost/xpressive/proto/proto_fwd.hpp>
#include <boost/xpressive/proto/traits.hpp>
#include <boost/xpressive/proto/expr.hpp>
#include <boost/xpressive/proto/args.hpp>
#include <boost/xpressive/proto/traits.hpp>
#include <boost/xpressive/proto/generate.hpp>
#include <boost/xpressive/proto/detail/suffix.hpp>

namespace boost { namespace proto
{
    /// INTERNAL ONLY
    ///
    #define BOOST_PROTO_CONST0

    /// INTERNAL ONLY
    ///
    #define BOOST_PROTO_CONST1 const

    /// INTERNAL ONLY
    ///
    #define BOOST_PROTO_TEMPLATE_YES_(Z, N) template<BOOST_PP_ENUM_PARAMS_Z(Z, N, typename A)>

    /// INTERNAL ONLY
    ///
    #define BOOST_PROTO_TEMPLATE_NO_(Z, N)

    /// INTERNAL ONLY
    ///
    #define BOOST_PROTO_DEFINE_FUN_OP_IMPL_(Z, N, Data, Const)                                      \
        BOOST_PP_IF(N, BOOST_PROTO_TEMPLATE_YES_, BOOST_PROTO_TEMPLATE_NO_)(Z, N)                   \
        typename boost::mpl::apply_wrap1<                                                           \
            BOOST_PP_TUPLE_ELEM(3, 2, Data)                                                         \
          , typename boost::proto::result_of::BOOST_PP_CAT(funop, N)<                               \
                BOOST_PP_TUPLE_ELEM(3, 1, Data) BOOST_PROTO_CONST ## Const                          \
                BOOST_PP_ENUM_TRAILING_PARAMS_Z(Z, N, const A)                                      \
            >::type                                                                                 \
        >::type const                                                                               \
        operator ()(BOOST_PP_ENUM_BINARY_PARAMS_Z(Z, N, A, const &a)) BOOST_PROTO_CONST ## Const    \
        {                                                                                           \
            typedef boost::proto::result_of::BOOST_PP_CAT(funop, N)<                                \
                BOOST_PP_TUPLE_ELEM(3, 1, Data) BOOST_PROTO_CONST ## Const                          \
                BOOST_PP_ENUM_TRAILING_PARAMS_Z(Z, N, const A)                                      \
            > funop;                                                                                \
            return BOOST_PP_TUPLE_ELEM(3, 2, Data)::make(                                           \
                funop::call(                                                                        \
                    *static_cast<BOOST_PP_TUPLE_ELEM(3, 1, Data) BOOST_PROTO_CONST ## Const *>(this)\
                    BOOST_PP_ENUM_TRAILING_PARAMS_Z(Z, N, a)                                        \
                )                                                                                   \
            );                                                                                      \
        }                                                                                           \
        /**/

    /// INTERNAL ONLY
    ///
    #define BOOST_PROTO_DEFINE_FUN_OP_CONST(Z, N, Data)                                             \
        BOOST_PROTO_DEFINE_FUN_OP_IMPL_(Z, N, Data, 1)

    /// INTERNAL ONLY
    ///
    #define BOOST_PROTO_DEFINE_FUN_OP_NON_CONST(Z, N, Data)                                         \
        BOOST_PROTO_DEFINE_FUN_OP_IMPL_(Z, N, Data, 0)

    /// INTERNAL ONLY
    ///
    #define BOOST_PROTO_DEFINE_FUN_OP(Z, N, Data)                                                   \
        BOOST_PROTO_DEFINE_FUN_OP_CONST(Z, N, Data)                                                 \
        BOOST_PROTO_DEFINE_FUN_OP_NON_CONST(Z, N, Data)                                             \
        /**/

    /// INTERNAL ONLY
    ///
    #define BOOST_PROTO_EXTENDS_ARG(z, n, Expr)                                                     \
        typedef                                                                                     \
            typename Expr::BOOST_PP_CAT(proto_arg, n)                                               \
        BOOST_PP_CAT(proto_arg, n);                                                                 \
        /**/

    #define BOOST_PROTO_EXTENDS(Expr, Derived, Domain)                                              \
        Expr expr;                                                                                  \
                                                                                                    \
        typedef Expr proto_base_expr;                                                               \
        typedef Domain proto_domain;                                                                \
        typedef Derived proto_derived_expr;                                                         \
        typedef typename Expr::proto_tag proto_tag;                                                 \
        typedef typename Expr::proto_args proto_args;                                               \
        typedef typename Expr::proto_arity proto_arity;                                             \
        typedef void proto_is_expr_;                                                                \
        BOOST_PROTO_DEFINE_FUSION_TAG(boost::proto::tag::proto_expr)                                \
        BOOST_PP_REPEAT(BOOST_PROTO_MAX_ARITY, BOOST_PROTO_EXTENDS_ARG, Expr)                       \
                                                                                                    \
        static Derived const make(Expr const &expr)                                                 \
        {                                                                                           \
            Derived that = {expr};                                                                  \
            return that;                                                                            \
        }                                                                                           \
                                                                                                    \
        Expr &proto_base()                                                                          \
        {                                                                                           \
            return this->expr;                                                                      \
        }                                                                                           \
                                                                                                    \
        Expr const &proto_base() const                                                              \
        {                                                                                           \
            return this->expr;                                                                      \
        }                                                                                           \
        /**/

        /// INTERNAL ONLY
        ///
    #define BOOST_PROTO_EXTENDS_ASSIGN_IMPL_(Expr, Derived, Domain, Const)                          \
        template<typename A>                                                                        \
        typename boost::mpl::apply_wrap1<                                                           \
            Domain                                                                                  \
          , boost::proto::expr<                                                                     \
                boost::proto::tag::assign                                                           \
              , boost::proto::args2<                                                                \
                    boost::proto::ref_<Derived BOOST_PROTO_CONST ## Const>                          \
                  , typename boost::proto::result_of::as_arg<A>::type                               \
                >                                                                                   \
            >                                                                                       \
        >::type const                                                                               \
        operator =(A &a) BOOST_PROTO_CONST ## Const                                                 \
        {                                                                                           \
            typedef boost::proto::expr<                                                             \
                boost::proto::tag::assign                                                           \
              , boost::proto::args2<                                                                \
                    boost::proto::ref_<Derived BOOST_PROTO_CONST ## Const>                          \
                  , typename boost::proto::result_of::as_arg<A>::type                               \
                >                                                                                   \
            > that_type;                                                                            \
            that_type that = {                                                                      \
                {*static_cast<Derived BOOST_PROTO_CONST ## Const *>(this)}                          \
              , boost::proto::as_arg(a)                                                             \
            };                                                                                      \
            return Domain::make(that);                                                              \
        }                                                                                           \
                                                                                                    \
        template<typename A>                                                                        \
        typename boost::mpl::apply_wrap1<                                                           \
            Domain                                                                                  \
          , boost::proto::expr<                                                                     \
                boost::proto::tag::assign                                                           \
              , boost::proto::args2<                                                                \
                    boost::proto::ref_<Derived BOOST_PROTO_CONST ## Const>                          \
                  , typename boost::proto::result_of::as_arg<A const>::type                         \
                >                                                                                   \
            >                                                                                       \
        >::type const                                                                               \
        operator =(A const &a) BOOST_PROTO_CONST ## Const                                           \
        {                                                                                           \
            typedef boost::proto::expr<                                                             \
                boost::proto::tag::assign                                                           \
              , boost::proto::args2<                                                                \
                    boost::proto::ref_<Derived BOOST_PROTO_CONST ## Const>                          \
                  , typename boost::proto::result_of::as_arg<A const>::type                         \
                >                                                                                   \
            > that_type;                                                                            \
            that_type that = {                                                                      \
                {*static_cast<Derived BOOST_PROTO_CONST ## Const *>(this)}                          \
              , boost::proto::as_arg(a)                                                             \
            };                                                                                      \
            return Domain::make(that);                                                              \
        }                                                                                           \
        /**/

    #define BOOST_PROTO_EXTENDS_ASSIGN_CONST(Expr, Derived, Domain)                                 \
        BOOST_PROTO_EXTENDS_ASSIGN_IMPL_(Expr, Derived, Domain, 1)

    #define BOOST_PROTO_EXTENDS_ASSIGN_NON_CONST(Expr, Derived, Domain)                             \
        BOOST_PROTO_EXTENDS_ASSIGN_IMPL_(Expr, Derived, Domain, 0)

    #define BOOST_PROTO_EXTENDS_ASSIGN(Expr, Derived, Domain)                                       \
        BOOST_PROTO_EXTENDS_ASSIGN_CONST(Expr, Derived, Domain)                                     \
        BOOST_PROTO_EXTENDS_ASSIGN_NON_CONST(Expr, Derived, Domain)                                 \
        /**/

        /// INTERNAL ONLY
        ///
    #define BOOST_PROTO_EXTENDS_SUBSCRIPT_IMPL_(Expr, Derived, Domain, Const)                       \
        template<typename A>                                                                        \
        typename boost::mpl::apply_wrap1<                                                           \
            Domain                                                                                  \
          , boost::proto::expr<                                                                     \
                boost::proto::tag::subscript                                                        \
              , boost::proto::args2<                                                                \
                    boost::proto::ref_<Derived BOOST_PROTO_CONST ## Const>                          \
                  , typename boost::proto::result_of::as_arg<A>::type                               \
                >                                                                                   \
            >                                                                                       \
        >::type const                                                                               \
        operator [](A &a) BOOST_PROTO_CONST ## Const                                                \
        {                                                                                           \
            typedef boost::proto::expr<                                                             \
                boost::proto::tag::subscript                                                        \
              , boost::proto::args2<                                                                \
                    boost::proto::ref_<Derived BOOST_PROTO_CONST ## Const>                          \
                  , typename boost::proto::result_of::as_arg<A>::type                               \
                >                                                                                   \
            > that_type;                                                                            \
            that_type that = {                                                                      \
                {*static_cast<Derived BOOST_PROTO_CONST ## Const *>(this)}                          \
              , boost::proto::as_arg(a)                                                             \
            };                                                                                      \
            return Domain::make(that);                                                              \
        }                                                                                           \
                                                                                                    \
        template<typename A>                                                                        \
        typename boost::mpl::apply_wrap1<                                                           \
            Domain                                                                                  \
          , boost::proto::expr<                                                                     \
                boost::proto::tag::subscript                                                        \
              , boost::proto::args2<                                                                \
                    boost::proto::ref_<Derived BOOST_PROTO_CONST ## Const>                          \
                  , typename boost::proto::result_of::as_arg<A const>::type                         \
                >                                                                                   \
            >                                                                                       \
        >::type const                                                                               \
        operator [](A const &a) BOOST_PROTO_CONST ## Const                                          \
        {                                                                                           \
            typedef boost::proto::expr<                                                             \
                boost::proto::tag::subscript                                                        \
              , boost::proto::args2<                                                                \
                    boost::proto::ref_<Derived BOOST_PROTO_CONST ## Const>                          \
                  , typename boost::proto::result_of::as_arg<A const>::type                         \
                >                                                                                   \
            > that_type;                                                                            \
            that_type that = {                                                                      \
                {*static_cast<Derived BOOST_PROTO_CONST ## Const *>(this)}                          \
              , boost::proto::as_arg(a)                                                             \
            };                                                                                      \
            return Domain::make(that);                                                              \
        }                                                                                           \
        /**/

    #define BOOST_PROTO_EXTENDS_SUBSCRIPT_CONST(Expr, Derived, Domain)                              \
        BOOST_PROTO_EXTENDS_SUBSCRIPT_IMPL_(Expr, Derived, Domain, 1)

    #define BOOST_PROTO_EXTENDS_SUBSCRIPT_NON_CONST(Expr, Derived, Domain)                          \
        BOOST_PROTO_EXTENDS_SUBSCRIPT_IMPL_(Expr, Derived, Domain, 0)

    #define BOOST_PROTO_EXTENDS_SUBSCRIPT(Expr, Derived, Domain)                                    \
        BOOST_PROTO_EXTENDS_SUBSCRIPT_CONST(Expr, Derived, Domain)                                  \
        BOOST_PROTO_EXTENDS_SUBSCRIPT_NON_CONST(Expr, Derived, Domain)                              \
        /**/

        /// INTERNAL ONLY
        ///
    #define BOOST_PROTO_EXTENDS_FUNCTION_(Expr, Derived, Domain)                                    \
        template<typename Sig>                                                                      \
        struct result                                                                               \
        {                                                                                           \
            typedef                                                                                 \
                typename boost::mpl::apply_wrap1<                                                   \
                    Domain                                                                          \
                  , typename boost::proto::result_of::funop<Sig, Derived >::type                    \
                >::type                                                                             \
            type;                                                                                   \
        };                                                                                          \
        /**/

    #define BOOST_PROTO_EXTENDS_FUNCTION_CONST(Expr, Derived, Domain)                               \
        BOOST_PROTO_EXTENDS_FUNCTION_(Expr, Derived, Domain)                                        \
        BOOST_PP_REPEAT_FROM_TO(                                                                    \
            0                                                                                       \
          , BOOST_PP_DEC(BOOST_PROTO_MAX_FUNCTION_CALL_ARITY)                                       \
          , BOOST_PROTO_DEFINE_FUN_OP_CONST                                                         \
          , (Expr, Derived, Domain)                                                                 \
        )                                                                                           \
        /**/

    #define BOOST_PROTO_EXTENDS_FUNCTION_NON_CONST(Expr, Derived, Domain)                           \
        BOOST_PROTO_EXTENDS_FUNCTION_(Expr, Derived, Domain)                                        \
        BOOST_PP_REPEAT_FROM_TO(                                                                    \
            0                                                                                       \
          , BOOST_PP_DEC(BOOST_PROTO_MAX_FUNCTION_CALL_ARITY)                                       \
          , BOOST_PROTO_DEFINE_FUN_OP_NON_CONST                                                     \
          , (Expr, Derived, Domain)                                                                 \
        )                                                                                           \
        /**/

    #define BOOST_PROTO_EXTENDS_FUNCTION(Expr, Derived, Domain)                                     \
        BOOST_PROTO_EXTENDS_FUNCTION_(Expr, Derived, Domain)                                        \
        BOOST_PP_REPEAT_FROM_TO(                                                                    \
            0                                                                                       \
          , BOOST_PP_DEC(BOOST_PROTO_MAX_FUNCTION_CALL_ARITY)                                       \
          , BOOST_PROTO_DEFINE_FUN_OP                                                               \
          , (Expr, Derived, Domain)                                                                 \
        )                                                                                           \
        /**/

    namespace exprns_
    {
        /// \brief Empty type to be used as a dummy template parameter of
        ///     POD expression wrappers. It allows argument-dependent lookup
        ///     to find Proto's operator overloads.
        ///
        /// \c proto::is_proto_expr allows argument-dependent lookup
        ///     to find Proto's operator overloads. For example:
        ///
        /// \code
        /// template<typename T, typename Dummy = proto::is_proto_expr>
        /// struct my_terminal
        /// {
        ///     BOOST_PROTO_EXTENDS(
        ///         typename proto::terminal<T>::type
        ///       , my_terminal<T>
        ///       , default_domain
        ///     )
        /// };
        ///
        /// // ...
        /// my_terminal<int> _1, _2;
        /// _1 + _2; // OK, uses proto::operator+
        /// \endcode
        ///
        /// Without the second \c Dummy template parameter, Proto's operator
        /// overloads would not be considered by name lookup.
        struct is_proto_expr
        {};

        /// \brief extends\<\> class template for adding behaviors to a Proto expression template
        ///
        template<
            typename Expr
          , typename Derived
          , typename Domain     BOOST_PROTO_FOR_DOXYGEN_ONLY(= default_domain)
          , typename Tag        BOOST_PROTO_FOR_DOXYGEN_ONLY(= typename Expr::proto_tag)
        >
        struct extends
        {
            extends()
              : expr()
            {}

            extends(extends const &that)
              : expr(that.expr)
            {}

            extends(Expr const &expr_)
              : expr(expr_)
            {}

            BOOST_PROTO_EXTENDS(Expr, Derived, Domain)
            BOOST_PROTO_EXTENDS_ASSIGN_CONST(Expr, Derived, Domain)
            BOOST_PROTO_EXTENDS_SUBSCRIPT_CONST(Expr, Derived, Domain)

            // Instead of using BOOST_PROTO_EXTENDS_FUNCTION, which uses
            // nested preprocessor loops, use file iteration here to generate
            // the operator() overloads, which is more efficient.
            BOOST_PROTO_EXTENDS_FUNCTION_(Expr, Derived, Domain)

            /// INTERNAL ONLY
            ///
        #define BOOST_PP_LOCAL_MACRO(N)                                                             \
            BOOST_PROTO_DEFINE_FUN_OP_CONST(1, N, (Expr, Derived, Domain))                          \
            /**/

            /// INTERNAL ONLY
            ///
        #define BOOST_PP_LOCAL_LIMITS (0, BOOST_PP_DEC(BOOST_PROTO_MAX_FUNCTION_CALL_ARITY))
        #include BOOST_PP_LOCAL_ITERATE()
        };

        /// \brief extends\<\> class template for adding behaviors to a Proto expression template
        ///
        template<typename Expr, typename Derived, typename Domain>
        struct extends<Expr, Derived, Domain, tag::terminal>
        {
            extends()
              : expr()
            {}

            extends(extends const &that)
              : expr(that.expr)
            {}

            extends(Expr const &expr_)
              : expr(expr_)
            {}

            BOOST_PROTO_EXTENDS(Expr, Derived, Domain)
            BOOST_PROTO_EXTENDS_ASSIGN(Expr, Derived, Domain)
            BOOST_PROTO_EXTENDS_SUBSCRIPT(Expr, Derived, Domain)

            // Instead of using BOOST_PROTO_EXTENDS_FUNCTION, which uses
            // nested preprocessor loops, use file iteration here to generate
            // the operator() overloads, which is more efficient.
            BOOST_PROTO_EXTENDS_FUNCTION_(Expr, Derived, Domain)

            /// INTERNAL ONLY
            ///
        #define BOOST_PP_LOCAL_MACRO(N)                                                             \
            BOOST_PROTO_DEFINE_FUN_OP(1, N, (Expr, Derived, Domain))                                \
            /**/

            /// INTERNAL ONLY
            ///
        #define BOOST_PP_LOCAL_LIMITS (0, BOOST_PP_DEC(BOOST_PROTO_MAX_FUNCTION_CALL_ARITY))
        #include BOOST_PP_LOCAL_ITERATE()
        };

    } // namespace exprns_

}}

#endif
