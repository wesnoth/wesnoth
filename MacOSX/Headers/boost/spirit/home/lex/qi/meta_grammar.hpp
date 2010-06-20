//  Copyright (c) 2001-2007 Joel de Guzman
//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_LEX_QI_META_GRAMMAR_NOV_18_2007_1144AM)
#define BOOST_SPIRIT_LEX_QI_META_GRAMMAR_NOV_18_2007_1144AM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/qi/domain.hpp>
#include <boost/spirit/home/support/placeholders.hpp>
#include <boost/spirit/home/support/meta_grammar.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/mpl/bool.hpp>

namespace boost { namespace spirit { namespace qi
{
    ///////////////////////////////////////////////////////////////////////////
    // forward declarations
    ///////////////////////////////////////////////////////////////////////////
    struct main_meta_grammar;
    struct lexer_meta_grammar;

    struct state_switcher;
    struct state_switcher_context;

    struct plain_token;

    template <typename Expr, typename Enable>
    struct is_valid_expr;

    template <typename Expr, typename Enable>
    struct expr_transform;

    ///////////////////////////////////////////////////////////////////////////
    //  main lexer_meta_grammar in the qi namespace
    ///////////////////////////////////////////////////////////////////////////
    struct lexer_meta_grammar
      : proto::or_<
            // lexer, lexer_def, token_def
            meta_grammar::terminal_rule<
                qi::domain, 
                lex::terminal_holder<proto::_, proto::_>, 
                lex::terminal_director
            >,
            // set_state("..."), set_state(str)
            meta_grammar::terminal_rule<
                qi::domain, tag::set_state_tag<proto::_>, state_switcher
            >,
            // in_state("...")[], in_state(str)[]
            meta_grammar::subscript_rule<
                qi::domain, in_state_tag<proto::_>, state_switcher_context,
                main_meta_grammar
            >,
            // token(id)
            meta_grammar::function1_rule<
                qi::domain, tag::token, plain_token, 
                proto::terminal<proto::convertible_to<int> >
            >
        >
    {
    };
    
    ///////////////////////////////////////////////////////////////////////////
    //  These specializations non-intrusively hook into the Qi meta-grammar.
    //  (see qi/meta_grammar.hpp)
    ///////////////////////////////////////////////////////////////////////////
    template <typename Expr>
    struct is_valid_expr<
            Expr,
            typename enable_if<
                proto::matches<Expr, lexer_meta_grammar> 
            >::type
        >
      : mpl::true_
    {
    };

    template <typename Expr>
    struct expr_transform<
            Expr,
            typename enable_if<
                proto::matches<Expr, lexer_meta_grammar> 
            >::type
        >
      : mpl::identity<lexer_meta_grammar>
    {
    };

}}}

#endif
