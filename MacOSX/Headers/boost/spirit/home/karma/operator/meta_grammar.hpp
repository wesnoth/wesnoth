//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_KARMA_META_GRAMMAR_FEB_28_2007_0346PM)
#define BOOST_SPIRIT_KARMA_META_GRAMMAR_FEB_28_2007_0346PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/karma/domain.hpp>
#include <boost/spirit/home/support/meta_grammar.hpp>
#include <boost/utility/enable_if.hpp>

namespace boost { namespace spirit { namespace karma
{
    ///////////////////////////////////////////////////////////////////////////
    // forwards
    ///////////////////////////////////////////////////////////////////////////
    struct sequence;
    struct alternative;
    struct kleene;
    struct plus;
    struct optional;
    struct list;
    
    struct main_meta_grammar;

    template <typename Expr, typename Enable>
    struct is_valid_expr;

    template <typename Expr, typename Enable>
    struct expr_transform;

    ///////////////////////////////////////////////////////////////////////////
    // operator meta-grammars
    ///////////////////////////////////////////////////////////////////////////
    struct sequence_meta_grammar
      : proto::or_<
            meta_grammar::binary_rule_flat<
                karma::domain, proto::tag::shift_left, sequence,
                main_meta_grammar
            >,
            meta_grammar::binary_rule_flat<
                karma::domain, proto::tag::plus, sequence,
                main_meta_grammar
            >
        >
    {
    };

    struct alternative_meta_grammar
      : meta_grammar::binary_rule_flat<
            karma::domain, proto::tag::bitwise_or, alternative,
            main_meta_grammar
        >
    {
    };

    struct repeat_meta_grammar
      : proto::or_<
            meta_grammar::unary_rule<
                karma::domain, proto::tag::dereference, kleene,
                main_meta_grammar
            >,
            meta_grammar::unary_rule<
                karma::domain, proto::tag::negate, optional,
                main_meta_grammar
            >,
            meta_grammar::unary_rule<
                karma::domain, proto::tag::posit, plus,
                main_meta_grammar
            >,
            meta_grammar::binary_rule<
                karma::domain, proto::tag::modulus, list,
                main_meta_grammar, main_meta_grammar
            >
        >
    {
    };

    struct operator_meta_grammar
      : proto::or_<
            sequence_meta_grammar,
            alternative_meta_grammar,
            repeat_meta_grammar
        >
    {
    };

    ///////////////////////////////////////////////////////////////////////////
    //  These specializations non-intrusively hooks into the Karma meta-grammar.
    //  (see karma/meta_grammar.hpp)
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
