//  Copyright (c) 2001-2008 Hartmut Kaiser
//  Copyright (c) 2001-2007 Joel de Guzman
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_KARMA_META_GRAMMAR_MAR_05_2007_0436PM)
#define BOOST_SPIRIT_KARMA_META_GRAMMAR_MAR_05_2007_0436PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/support/nonterminal/nonterminal.hpp>
#include <boost/spirit/home/karma/nonterminal/nonterminal.hpp>
#include <boost/spirit/home/karma/domain.hpp>
#include <boost/spirit/home/support/meta_grammar.hpp>
#include <boost/utility/enable_if.hpp>

namespace boost { namespace spirit { namespace karma
{
    ///////////////////////////////////////////////////////////////////////////
    // forwards
    ///////////////////////////////////////////////////////////////////////////
    struct nonterminal_director;
    
    template <typename Expr, typename Enable>
    struct is_valid_expr;

    template <typename Expr, typename Enable>
    struct expr_transform;

    ///////////////////////////////////////////////////////////////////////////
    // nonterminal meta-grammar
    ///////////////////////////////////////////////////////////////////////////
    struct nonterminal_meta_grammar
      : meta_grammar::terminal_rule<
            karma::domain,
            nonterminal_holder<proto::_, proto::_>,
            nonterminal_director
        >
    {
    };

    ///////////////////////////////////////////////////////////////////////////
    //  These specializations non-intrusively hook into the Karma meta-grammar.
    //  (see karma/meta_grammar.hpp)
    ///////////////////////////////////////////////////////////////////////////
    template <typename Expr>
    struct is_valid_expr<
            Expr,
            typename enable_if<
                proto::matches<Expr, nonterminal_meta_grammar> 
            >::type
        >
      : mpl::true_
    {
    };

    template <typename Expr>
    struct expr_transform<
            Expr,
            typename enable_if<
                proto::matches<Expr, nonterminal_meta_grammar> 
            >::type
        >
      : mpl::identity<nonterminal_meta_grammar>
    {
    };
}}}

#endif
