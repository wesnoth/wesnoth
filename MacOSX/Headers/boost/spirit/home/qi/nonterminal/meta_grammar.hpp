/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying 
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_SPIRIT_META_GRAMMAR_FEB_12_2007_0534PM)
#define BOOST_SPIRIT_META_GRAMMAR_FEB_12_2007_0534PM

#include <boost/spirit/home/support/nonterminal/nonterminal.hpp>
#include <boost/spirit/home/qi/domain.hpp>
#include <boost/spirit/home/qi/nonterminal/rule.hpp>
#include <boost/spirit/home/qi/nonterminal/grammar.hpp>
#include <boost/spirit/home/support/meta_grammar.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/identity.hpp>

namespace boost { namespace spirit { namespace qi
{
    ///////////////////////////////////////////////////////////////////////////
    // forwards
    ///////////////////////////////////////////////////////////////////////////
    template <typename Expr, typename Enable>
    struct is_valid_expr;

    template <typename Expr, typename Enable>
    struct expr_transform;

    ///////////////////////////////////////////////////////////////////////////
    // nonterminal meta-grammar
    ///////////////////////////////////////////////////////////////////////////
    struct nonterminal_meta_grammar
      : meta_grammar::terminal_rule<
            qi::domain
          , nonterminal_holder<proto::_, proto::_>
          , nonterminal_director
        >
    {
    };

    ///////////////////////////////////////////////////////////////////////////
    //  These specializations non-intrusively hooks into the RD meta-grammar.
    //  (see qi/meta_grammar.hpp)
    ///////////////////////////////////////////////////////////////////////////
    template <typename Expr>
    struct is_valid_expr<Expr
      , typename enable_if<proto::matches<Expr, nonterminal_meta_grammar> >::type>
      : mpl::true_
    {
    };

    template <typename Expr>
    struct expr_transform<Expr
      , typename enable_if<proto::matches<Expr, nonterminal_meta_grammar> >::type>
      : mpl::identity<nonterminal_meta_grammar>
    {
    };
}}}

#endif
