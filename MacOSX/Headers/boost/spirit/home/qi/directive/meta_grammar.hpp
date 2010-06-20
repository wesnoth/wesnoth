/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_SPIRIT_META_GRAMMAR_FEB_05_2007_0320PM)
#define BOOST_SPIRIT_META_GRAMMAR_FEB_05_2007_0320PM

#include <boost/spirit/home/support/modifier.hpp>
#include <boost/spirit/home/support/placeholders.hpp>
#include <boost/spirit/home/support/meta_grammar.hpp>
#include <boost/spirit/home/qi/domain.hpp>
#include <boost/spirit/home/support/iso8859_1.hpp>
#include <boost/spirit/home/support/ascii.hpp>
#include <boost/spirit/home/support/standard.hpp>
#include <boost/spirit/home/support/standard_wide.hpp>
#include <boost/utility/enable_if.hpp>

namespace boost { namespace spirit { namespace qi
{
    ///////////////////////////////////////////////////////////////////////////
    // forwards
    ///////////////////////////////////////////////////////////////////////////
    struct lexeme_director;
    struct omit_director;
    struct raw_director;
    struct main_meta_grammar;

    template <typename Expr, typename Enable>
    struct is_valid_expr;

    template <typename Expr, typename Enable>
    struct expr_transform;

    ///////////////////////////////////////////////////////////////////////////
    // directive meta-grammars
    ///////////////////////////////////////////////////////////////////////////
    struct directive_meta_grammar
      : proto::or_<
            meta_grammar::deep_directive_meta_grammar<
                spirit::char_class::no_case_tag<proto::_>
              , main_meta_grammar
            >
          , meta_grammar::binary_rule_rhs<
                qi::domain, proto::tag::subscript, lexeme_director
              , proto::terminal<tag::lexeme>, main_meta_grammar
            >
          , meta_grammar::binary_rule_rhs<
                qi::domain, proto::tag::subscript, omit_director
              , proto::terminal<tag::omit>, main_meta_grammar
            >
          , meta_grammar::binary_rule_rhs<
                qi::domain, proto::tag::subscript, raw_director
              , proto::terminal<tag::raw>, main_meta_grammar
            >
        >
    {};

    ///////////////////////////////////////////////////////////////////////////
    //  These specializations non-intrusively hooks into the RD meta-grammar.
    //  (see qi/meta_grammar.hpp)
    ///////////////////////////////////////////////////////////////////////////
    template <typename Expr>
    struct is_valid_expr<Expr
      , typename enable_if<proto::matches<Expr, directive_meta_grammar> >::type>
      : mpl::true_
    {
    };

    template <typename Expr>
    struct expr_transform<Expr
      , typename enable_if<proto::matches<Expr, directive_meta_grammar> >::type>
      : mpl::identity<directive_meta_grammar>
    {
    };
}}}

#endif
