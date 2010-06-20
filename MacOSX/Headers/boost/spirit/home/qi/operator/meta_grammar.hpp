/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_SPIRIT_META_GRAMMAR_FEB_02_2007_0620PM)
#define BOOST_SPIRIT_META_GRAMMAR_FEB_02_2007_0620PM

#include <boost/spirit/home/qi/domain.hpp>
#include <boost/spirit/home/support/meta_grammar.hpp>
#include <boost/utility/enable_if.hpp>

namespace boost { namespace spirit { namespace qi
{
    ///////////////////////////////////////////////////////////////////////////
    // forwards
    ///////////////////////////////////////////////////////////////////////////
    struct sequence;
    struct expect;
    struct alternative;
    struct sequential_or;
    struct permutation;
    struct difference;
    struct list;
    struct optional;
    struct kleene;
    struct plus;
    struct and_predicate;
    struct not_predicate;
    struct main_meta_grammar;

    template <typename Expr, typename Enable>
    struct is_valid_expr;

    template <typename Expr, typename Enable>
    struct expr_transform;

    ///////////////////////////////////////////////////////////////////////////
    // operator meta-grammars
    ///////////////////////////////////////////////////////////////////////////
    struct binary_meta_grammar
      : proto::or_<
            // a >> b
            meta_grammar::binary_rule_flat<
                qi::domain, proto::tag::shift_right, sequence
              , main_meta_grammar
            >
            // a + b
          , meta_grammar::binary_rule_flat<
                qi::domain, proto::tag::plus, sequence
              , main_meta_grammar
            >
            // a > b
          , meta_grammar::binary_rule_flat<
                qi::domain, proto::tag::greater, expect
              , main_meta_grammar
            >
            // a | b
          , meta_grammar::binary_rule_flat<
                qi::domain, proto::tag::bitwise_or, alternative
              , main_meta_grammar
            >
            // a || b
          , meta_grammar::binary_rule_flat<
                qi::domain, proto::tag::logical_or, sequential_or
              , main_meta_grammar
            >
            // a ^ b
          , meta_grammar::binary_rule_flat<
                qi::domain, proto::tag::bitwise_xor, permutation
              , main_meta_grammar
            >
            // a - b
          , meta_grammar::binary_rule<
                qi::domain, proto::tag::minus, difference
              , main_meta_grammar, main_meta_grammar
            >
            // a % b
          , meta_grammar::binary_rule<
                qi::domain, proto::tag::modulus, list
              , main_meta_grammar, main_meta_grammar
            >
        >
    {
    };

    struct unary_meta_grammar
      : proto::or_<
            // -a
            meta_grammar::unary_rule<
                qi::domain, proto::tag::negate, optional
              , main_meta_grammar
            >
            // *a
          , meta_grammar::unary_rule<
                qi::domain, proto::tag::dereference, kleene
              , main_meta_grammar
            >
            // +a
          , meta_grammar::unary_rule<
                qi::domain, proto::tag::posit, plus
              , main_meta_grammar
            >
            // &a
          , meta_grammar::unary_rule<
                qi::domain, proto::tag::address_of, and_predicate
              , main_meta_grammar
            >
            // !a
          , meta_grammar::unary_rule<
                qi::domain, proto::tag::logical_not, not_predicate
              , main_meta_grammar
            >
        >
    {
    };

    struct operator_meta_grammar
      : proto::or_<
            binary_meta_grammar
          , unary_meta_grammar
        >
    {
    };

    ///////////////////////////////////////////////////////////////////////////
    //  These specializations non-intrusively hooks into the RD meta-grammar.
    //  (see qi/meta_grammar.hpp)
    ///////////////////////////////////////////////////////////////////////////
    template <typename Expr>
    struct is_valid_expr<Expr
      , typename enable_if<proto::matches<Expr, operator_meta_grammar> >::type>
      : mpl::true_
    {
    };

    template <typename Expr>
    struct expr_transform<Expr
      , typename enable_if<proto::matches<Expr, operator_meta_grammar> >::type>
      : mpl::identity<operator_meta_grammar>
    {
    };
}}}

#endif
