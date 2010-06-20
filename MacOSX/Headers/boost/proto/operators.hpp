///////////////////////////////////////////////////////////////////////////////
/// \file operators.hpp
/// Contains all the overloaded operators that make it possible to build
/// Proto expression trees.
//
//  Copyright 2008 Eric Niebler. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PROTO_OPERATORS_HPP_EAN_04_01_2005
#define BOOST_PROTO_OPERATORS_HPP_EAN_04_01_2005

#include <boost/proto/detail/prefix.hpp>
#include <boost/preprocessor/punctuation/comma.hpp>
#include <boost/preprocessor/seq/seq.hpp>
#include <boost/mpl/or.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/proto/proto_fwd.hpp>
#include <boost/proto/tags.hpp>
#include <boost/proto/expr.hpp>
#include <boost/proto/generate.hpp>
#include <boost/proto/make_expr.hpp>
#include <boost/proto/detail/suffix.hpp>

namespace boost { namespace proto
{
    namespace detail
    {
        template<typename Tag, typename Left, typename Right, typename Enable1 = void, typename Enable2 = void>
        struct as_expr_if2
        {};

        template<typename Tag, typename Left, typename Right>
        struct as_expr_if2<Tag, Left, Right, typename Left::proto_is_expr_, void>
          : generate_if<
                typename Left::proto_domain
              , proto::expr<
                    Tag
                  , list2<
                        Left &
                      , typename Left::proto_domain::template result<void(proto::expr<tag::terminal, term<Right &> >)>::type
                    >
                >
            >
        {
            typedef proto::expr<tag::terminal, term<Right &> > term_type;
            typedef proto::expr<Tag, list2<Left &, typename Left::proto_domain::template result<void(term_type)>::type> > expr_type;

            static typename Left::proto_domain::template result<void(expr_type)>::type
            make(Left &left, Right &right)
            {
                term_type term = {right};
                expr_type that = {left, typename Left::proto_domain()(term)};
                return typename Left::proto_domain()(that);
            }
        };

        template<typename Tag, typename Left, typename Right>
        struct as_expr_if2<Tag, Left, Right, void, typename Right::proto_is_expr_>
          : generate_if<
                typename Right::proto_domain
              , proto::expr<
                    Tag
                  , list2<
                        typename Right::proto_domain::template result<void(proto::expr<tag::terminal, term<Left &> >)>::type
                      , Right &
                    >
                >
            >
        {
            typedef proto::expr<tag::terminal, term<Left &> > term_type;
            typedef proto::expr<Tag, list2<typename Right::proto_domain::template result<void(term_type)>::type, Right &> > expr_type;

            static typename Right::proto_domain::template result<void(expr_type)>::type
            make(Left &left, Right &right)
            {
                term_type term = {left};
                expr_type that = {typename Right::proto_domain()(term), right};
                return typename Right::proto_domain()(that);
            }
        };

        template<typename Tag, typename Left, typename Right, typename Enable1 = void, typename Enable2 = void>
        struct as_expr_if
          : as_expr_if2<Tag, Left, Right>
        {};

        template<typename Tag, typename Left, typename Right>
        struct as_expr_if<Tag, Left, Right, typename Left::proto_is_expr_, typename Right::proto_is_expr_>
          : generate_if<
                typename Left::proto_domain
              , proto::expr<Tag, list2<Left &, Right &> >
            >
        {
            typedef proto::expr<Tag, list2<Left &, Right &> > expr_type;
            BOOST_MPL_ASSERT((is_same<typename Left::proto_domain, typename Right::proto_domain>));

            static typename Left::proto_domain::template result<void(expr_type)>::type
            make(Left &left, Right &right)
            {
                expr_type that = {left, right};
                return typename Left::proto_domain()(that);
            }
        };

        template<typename Arg, typename Trait, typename Enable = void>
        struct arg_weight
        {
            BOOST_STATIC_CONSTANT(int, value = 1 + Trait::value);
        };

        template<typename Arg, typename Trait>
        struct arg_weight<Arg, Trait, typename Arg::proto_is_expr_>
        {
            BOOST_STATIC_CONSTANT(int, value = 0);
        };

        template<typename Domain, typename Trait, typename Arg, typename Expr>
        struct enable_unary
          : boost::enable_if<
                boost::mpl::and_<Trait, boost::proto::matches<Expr, typename Domain::proto_grammar> >
              , Expr
            >
        {};

        template<typename Trait, typename Arg, typename Expr>
        struct enable_unary<deduce_domain, Trait, Arg, Expr>
          : boost::enable_if<
                boost::mpl::and_<
                    Trait
                  , boost::proto::matches<Expr, typename domain_of<Arg>::type::proto_grammar>
                >
              , Expr
            >
        {};

        template<typename Trait, typename Arg, typename Expr>
        struct enable_unary<default_domain, Trait, Arg, Expr>
          : boost::enable_if<Trait, Expr>
        {};

        template<typename Domain, typename Trait1, typename Arg1, typename Trait2, typename Arg2, typename Expr>
        struct enable_binary
          : boost::enable_if<
                boost::mpl::and_<
                    mpl::bool_<(3 <= (arg_weight<Arg1, Trait1>::value + arg_weight<Arg2, Trait2>::value))>
                  , boost::proto::matches<Expr, typename Domain::proto_grammar>
                >
              , Expr
            >
        {};

        template<typename Trait1, typename Arg1, typename Trait2, typename Arg2, typename Expr>
        struct enable_binary<deduce_domain, Trait1, Arg1, Trait2, Arg2, Expr>
          : boost::enable_if<
                boost::mpl::and_<
                    mpl::bool_<(3 <= (arg_weight<Arg1, Trait1>::value + arg_weight<Arg2, Trait2>::value))>
                  , boost::proto::matches<Expr, typename deduce_domain2<Arg1, Arg2>::type::proto_grammar>
                >
              , Expr
            >
        {};

        template<typename Trait1, typename Arg1, typename Trait2, typename Arg2, typename Expr>
        struct enable_binary<default_domain, Trait1, Arg1, Trait2, Arg2, Expr>
          : boost::enable_if_c<
                (3 <= (arg_weight<Arg1, Trait1>::value + arg_weight<Arg2, Trait2>::value))
              , Expr
            >
        {};

    } // detail

#define BOOST_PROTO_UNARY_OP_IS_POSTFIX_0
#define BOOST_PROTO_UNARY_OP_IS_POSTFIX_1 , int

#define BOOST_PROTO_DEFINE_UNARY_OPERATOR(OP, TAG, POST)                                            \
    template<typename Arg>                                                                          \
    typename detail::generate_if<                                                                   \
        typename Arg::proto_domain                                                                  \
      , proto::expr<TAG, list1<typename Arg::proto_derived_expr &> >                                \
    >::type const                                                                                   \
    operator OP(Arg &arg BOOST_PROTO_UNARY_OP_IS_POSTFIX_ ## POST)                                  \
    {                                                                                               \
        typedef proto::expr<TAG, list1<typename Arg::proto_derived_expr &> > that_type;             \
        that_type that = {arg};                                                                     \
        return typename Arg::proto_domain()(that);                                                  \
    }                                                                                               \
    template<typename Arg>                                                                          \
    typename detail::generate_if<                                                                   \
        typename Arg::proto_domain                                                                  \
      , proto::expr<TAG, list1<typename Arg::proto_derived_expr const &> >                          \
    >::type const                                                                                   \
    operator OP(Arg const &arg BOOST_PROTO_UNARY_OP_IS_POSTFIX_ ## POST)                            \
    {                                                                                               \
        typedef proto::expr<TAG, list1<typename Arg::proto_derived_expr const &> > that_type;       \
        that_type that = {arg};                                                                     \
        return typename Arg::proto_domain()(that);                                                  \
    }                                                                                               \
    /**/

#define BOOST_PROTO_DEFINE_BINARY_OPERATOR(OP, TAG)                                                 \
    template<typename Left, typename Right>                                                         \
    inline typename detail::as_expr_if<TAG, Left, Right>::type const                                \
    operator OP(Left &left, Right &right)                                                           \
    {                                                                                               \
        return detail::as_expr_if<TAG, Left, Right>::make(left, right);                             \
    }                                                                                               \
    template<typename Left, typename Right>                                                         \
    inline typename detail::as_expr_if<TAG, Left, Right const>::type const                          \
    operator OP(Left &left, Right const &right)                                                     \
    {                                                                                               \
        return detail::as_expr_if<TAG, Left, Right const>::make(left, right);                       \
    }                                                                                               \
    template<typename Left, typename Right>                                                         \
    inline typename detail::as_expr_if<TAG, Left const, Right>::type const                          \
    operator OP(Left const &left, Right &right)                                                     \
    {                                                                                               \
        return detail::as_expr_if<TAG, Left const, Right>::make(left, right);                       \
    }                                                                                               \
    template<typename Left, typename Right>                                                         \
    inline typename detail::as_expr_if<TAG, Left const, Right const>::type const                    \
    operator OP(Left const &left, Right const &right)                                               \
    {                                                                                               \
        return detail::as_expr_if<TAG, Left const, Right const>::make(left, right);                 \
    }                                                                                               \
    /**/

    BOOST_PROTO_BEGIN_ADL_NAMESPACE(exprns_)

    BOOST_PROTO_DEFINE_UNARY_OPERATOR(+, tag::unary_plus, 0)
    BOOST_PROTO_DEFINE_UNARY_OPERATOR(-, tag::negate, 0)
    BOOST_PROTO_DEFINE_UNARY_OPERATOR(*, tag::dereference, 0)
    BOOST_PROTO_DEFINE_UNARY_OPERATOR(~, tag::complement, 0)
    BOOST_PROTO_DEFINE_UNARY_OPERATOR(&, tag::address_of, 0)
    BOOST_PROTO_DEFINE_UNARY_OPERATOR(!, tag::logical_not, 0)
    BOOST_PROTO_DEFINE_UNARY_OPERATOR(++, tag::pre_inc, 0)
    BOOST_PROTO_DEFINE_UNARY_OPERATOR(--, tag::pre_dec, 0)
    BOOST_PROTO_DEFINE_UNARY_OPERATOR(++, tag::post_inc, 1)
    BOOST_PROTO_DEFINE_UNARY_OPERATOR(--, tag::post_dec, 1)

    BOOST_PROTO_DEFINE_BINARY_OPERATOR(<<, tag::shift_left)
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(>>, tag::shift_right)
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(*, tag::multiplies)
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(/, tag::divides)
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(%, tag::modulus)
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(+, tag::plus)
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(-, tag::minus)
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(<, tag::less)
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(>, tag::greater)
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(<=, tag::less_equal)
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(>=, tag::greater_equal)
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(==, tag::equal_to)
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(!=, tag::not_equal_to)
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(||, tag::logical_or)
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(&&, tag::logical_and)
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(&, tag::bitwise_and)
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(|, tag::bitwise_or)
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(^, tag::bitwise_xor)
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(BOOST_PP_COMMA(), tag::comma)
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(->*, tag::mem_ptr)

    BOOST_PROTO_DEFINE_BINARY_OPERATOR(<<=, tag::shift_left_assign)
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(>>=, tag::shift_right_assign)
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(*=, tag::multiplies_assign)
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(/=, tag::divides_assign)
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(%=, tag::modulus_assign)
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(+=, tag::plus_assign)
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(-=, tag::minus_assign)
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(&=, tag::bitwise_and_assign)
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(|=, tag::bitwise_or_assign)
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(^=, tag::bitwise_xor_assign)

    /// if_else
    ///
    BOOST_PROTO_DEFINE_FUNCTION_TEMPLATE(
        3
      , if_else
      , deduce_domain
      , (tag::if_else_)
      , BOOST_PP_SEQ_NIL
    )

    BOOST_PROTO_END_ADL_NAMESPACE(exprns_)

    BOOST_PROTO_WHEN_NOT_BUILDING_DOCS(using exprns_::if_else;)

#undef BOOST_PROTO_DEFINE_UNARY_OPERATOR
#undef BOOST_PROTO_DEFINE_BINARY_OPERATOR

#define BOOST_PROTO_DEFINE_UNARY_OPERATOR(OP, TAG, TRAIT, DOMAIN, POST)                             \
    template<typename Arg>                                                                          \
    typename boost::proto::detail::enable_unary<DOMAIN, TRAIT<Arg>, Arg                             \
        , typename boost::proto::result_of::make_expr<TAG, DOMAIN, Arg &>::type                     \
    >::type const                                                                                   \
    operator OP(Arg &arg BOOST_PROTO_UNARY_OP_IS_POSTFIX_ ## POST)                                  \
    {                                                                                               \
        return boost::proto::make_expr<TAG, DOMAIN>(boost::ref(arg));                               \
    }                                                                                               \
    template<typename Arg>                                                                          \
    typename boost::proto::detail::enable_unary<DOMAIN, TRAIT<Arg>, Arg                             \
        , typename boost::proto::result_of::make_expr<TAG, DOMAIN, Arg const &>::type               \
    >::type const                                                                                   \
    operator OP(Arg const &arg BOOST_PROTO_UNARY_OP_IS_POSTFIX_ ## POST)                            \
    {                                                                                               \
        return boost::proto::make_expr<TAG, DOMAIN>(boost::ref(arg));                               \
    }                                                                                               \
    /**/

#define BOOST_PROTO_DEFINE_BINARY_OPERATOR(OP, TAG, TRAIT, DOMAIN)                                  \
    template<typename Left, typename Right>                                                         \
    typename boost::proto::detail::enable_binary<DOMAIN, TRAIT<Left>, Left, TRAIT<Right>, Right     \
        , typename boost::proto::result_of::make_expr<TAG, DOMAIN, Left &, Right &>::type           \
    >::type const                                                                                   \
    operator OP(Left &left, Right &right)                                                           \
    {                                                                                               \
        return boost::proto::make_expr<TAG, DOMAIN>(boost::ref(left), boost::ref(right));           \
    }                                                                                               \
    template<typename Left, typename Right>                                                         \
    typename boost::proto::detail::enable_binary<DOMAIN, TRAIT<Left>, Left, TRAIT<Right>, Right     \
        , typename boost::proto::result_of::make_expr<TAG, DOMAIN, Left &, Right const &>::type     \
    >::type const                                                                                   \
    operator OP(Left &left, Right const &right)                                                     \
    {                                                                                               \
        return boost::proto::make_expr<TAG, DOMAIN>(boost::ref(left), boost::ref(right));           \
    }                                                                                               \
    template<typename Left, typename Right>                                                         \
    typename boost::proto::detail::enable_binary<DOMAIN, TRAIT<Left>, Left, TRAIT<Right>, Right     \
        , typename boost::proto::result_of::make_expr<TAG, DOMAIN, Left const &, Right &>::type     \
    >::type const                                                                                   \
    operator OP(Left const &left, Right &right)                                                     \
    {                                                                                               \
        return boost::proto::make_expr<TAG, DOMAIN>(boost::ref(left), boost::ref(right));           \
    }                                                                                               \
    template<typename Left, typename Right>                                                         \
    typename boost::proto::detail::enable_binary<DOMAIN, TRAIT<Left>, Left, TRAIT<Right>, Right     \
        , typename boost::proto::result_of::make_expr<TAG, DOMAIN, Left const &, Right const &>::type\
    >::type const                                                                                   \
    operator OP(Left const &left, Right const &right)                                               \
    {                                                                                               \
        return boost::proto::make_expr<TAG, DOMAIN>(boost::ref(left), boost::ref(right));           \
    }                                                                                               \
    /**/

#define BOOST_PROTO_DEFINE_OPERATORS(TRAIT, DOMAIN)                                                 \
    BOOST_PROTO_DEFINE_UNARY_OPERATOR(+, boost::proto::tag::unary_plus, TRAIT, DOMAIN, 0)                \
    BOOST_PROTO_DEFINE_UNARY_OPERATOR(-, boost::proto::tag::negate, TRAIT, DOMAIN, 0)               \
    BOOST_PROTO_DEFINE_UNARY_OPERATOR(*, boost::proto::tag::dereference, TRAIT, DOMAIN, 0)          \
    BOOST_PROTO_DEFINE_UNARY_OPERATOR(~, boost::proto::tag::complement, TRAIT, DOMAIN, 0)           \
    BOOST_PROTO_DEFINE_UNARY_OPERATOR(&, boost::proto::tag::address_of, TRAIT, DOMAIN, 0)           \
    BOOST_PROTO_DEFINE_UNARY_OPERATOR(!, boost::proto::tag::logical_not, TRAIT, DOMAIN, 0)          \
    BOOST_PROTO_DEFINE_UNARY_OPERATOR(++, boost::proto::tag::pre_inc, TRAIT, DOMAIN, 0)             \
    BOOST_PROTO_DEFINE_UNARY_OPERATOR(--, boost::proto::tag::pre_dec, TRAIT, DOMAIN, 0)             \
    BOOST_PROTO_DEFINE_UNARY_OPERATOR(++, boost::proto::tag::post_inc, TRAIT, DOMAIN, 1)            \
    BOOST_PROTO_DEFINE_UNARY_OPERATOR(--, boost::proto::tag::post_dec, TRAIT, DOMAIN, 1)            \
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(<<, boost::proto::tag::shift_left, TRAIT, DOMAIN)            \
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(>>, boost::proto::tag::shift_right, TRAIT, DOMAIN)           \
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(*, boost::proto::tag::multiplies, TRAIT, DOMAIN)             \
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(/, boost::proto::tag::divides, TRAIT, DOMAIN)                \
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(%, boost::proto::tag::modulus, TRAIT, DOMAIN)                \
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(+, boost::proto::tag::plus, TRAIT, DOMAIN)                   \
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(-, boost::proto::tag::minus, TRAIT, DOMAIN)                  \
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(<, boost::proto::tag::less, TRAIT, DOMAIN)                   \
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(>, boost::proto::tag::greater, TRAIT, DOMAIN)                \
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(<=, boost::proto::tag::less_equal, TRAIT, DOMAIN)            \
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(>=, boost::proto::tag::greater_equal, TRAIT, DOMAIN)         \
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(==, boost::proto::tag::equal_to, TRAIT, DOMAIN)              \
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(!=, boost::proto::tag::not_equal_to, TRAIT, DOMAIN)          \
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(||, boost::proto::tag::logical_or, TRAIT, DOMAIN)            \
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(&&, boost::proto::tag::logical_and, TRAIT, DOMAIN)           \
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(&, boost::proto::tag::bitwise_and, TRAIT, DOMAIN)            \
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(|, boost::proto::tag::bitwise_or, TRAIT, DOMAIN)             \
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(^, boost::proto::tag::bitwise_xor, TRAIT, DOMAIN)            \
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(BOOST_PP_COMMA(), boost::proto::tag::comma, TRAIT, DOMAIN)   \
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(->*, boost::proto::tag::mem_ptr, TRAIT, DOMAIN)              \
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(<<=, boost::proto::tag::shift_left_assign, TRAIT, DOMAIN)    \
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(>>=, boost::proto::tag::shift_right_assign, TRAIT, DOMAIN)   \
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(*=, boost::proto::tag::multiplies_assign, TRAIT, DOMAIN)     \
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(/=, boost::proto::tag::divides_assign, TRAIT, DOMAIN)        \
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(%=, boost::proto::tag::modulus_assign, TRAIT, DOMAIN)        \
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(+=, boost::proto::tag::plus_assign, TRAIT, DOMAIN)           \
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(-=, boost::proto::tag::minus_assign, TRAIT, DOMAIN)          \
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(&=, boost::proto::tag::bitwise_and_assign, TRAIT, DOMAIN)    \
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(|=, boost::proto::tag::bitwise_or_assign, TRAIT, DOMAIN)     \
    BOOST_PROTO_DEFINE_BINARY_OPERATOR(^=, boost::proto::tag::bitwise_xor_assign, TRAIT, DOMAIN)    \
    /**/

    template<typename T>
    struct is_extension
      : mpl::false_
    {};

    #ifndef BOOST_PROTO_BUILDING_DOCS
    namespace exops
    {
        BOOST_PROTO_DEFINE_OPERATORS(is_extension, default_domain)
        using proto::if_else;
    }
    #endif

}}

#endif
