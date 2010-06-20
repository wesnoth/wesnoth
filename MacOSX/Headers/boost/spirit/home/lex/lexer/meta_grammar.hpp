//  Copyright (c) 2001-2007 Joel de Guzman
//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_LEX_LEXER_META_GRAMMAR_MAR_22_2007_0548PM)
#define BOOST_SPIRIT_LEX_LEXER_META_GRAMMAR_MAR_22_2007_0548PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/lex/domain.hpp>
#include <boost/spirit/home/lex/lexer/lexer_fwd.hpp>
#include <boost/spirit/home/lex/lexer/terminal_holder.hpp>
#include <boost/spirit/home/support/placeholders.hpp>
#include <boost/spirit/home/support/meta_grammar.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/mpl/bool.hpp>

namespace boost { namespace spirit { namespace lex
{
    ///////////////////////////////////////////////////////////////////////////
    // forward declarations
    ///////////////////////////////////////////////////////////////////////////
    struct terminal_director;
    struct sequence;
    struct action;
    struct string_token_def;
    struct char_token_def;
    
    struct lexer_meta_grammar;

    template <typename Expr, typename Enable>
    struct is_valid_expr;

    template <typename Expr, typename Enable>
    struct expr_transform;

    ///////////////////////////////////////////////////////////////////////////
    //  main lexer_meta_grammar in the lex namespace
    ///////////////////////////////////////////////////////////////////////////

    struct token_def_meta_grammar
      : proto::or_<
            // token_def<>
            meta_grammar::terminal_rule<
                lex::domain, 
                terminal_holder<proto::_, lex::token_def<proto::_, proto::_, proto::_> >, 
                terminal_director
            >,
            // token_set
            meta_grammar::terminal_rule<
                lex::domain, 
                terminal_holder<proto::_, lex::token_set<proto::_> >, 
                terminal_director
            >
        >
    {
    };
    
    // 'x', L'x', "x", L"x", std::string, std::wstring
    struct literal_token_def_meta_grammar
      : proto::or_<
            meta_grammar::terminal_rule<
                lex::domain, char, char_token_def
            >,
            meta_grammar::terminal_rule<
                lex::domain, wchar_t, char_token_def
            >,
            meta_grammar::terminal_rule<
                lex::domain, char const*, string_token_def 
            >,
            meta_grammar::terminal_rule<
                lex::domain, wchar_t const*, string_token_def 
            >,
            meta_grammar::terminal_rule<
                lex::domain,
                std::basic_string<char, proto::_, proto::_>,
                string_token_def 
            >,
            meta_grammar::terminal_rule<
                lex::domain,
                std::basic_string<wchar_t, proto::_, proto::_>,
                string_token_def 
            >
        >
    {
    };
    
    struct action_lexer_meta_grammar
      : proto::or_<
            // semantic actions for tokens
            meta_grammar::binary_rule<
                lex::domain, proto::tag::subscript, action,
                token_def_meta_grammar, proto::when<proto::_, proto::_arg>
            >,
            meta_grammar::binary_rule<
                lex::domain, proto::tag::subscript, action,
                action_lexer_meta_grammar, proto::when<proto::_, proto::_arg>
            >
        >
    {
    };
    
    struct lexer_meta_grammar
      : proto::or_<
            // token_def<>, ' ', L' ', "...", L"...", std::string, std::wstring
            token_def_meta_grammar,
            literal_token_def_meta_grammar,
            // token_def[...]
            action_lexer_meta_grammar,
            // sequence delimited by '|'
            meta_grammar::binary_rule_flat<
                lex::domain, proto::tag::bitwise_or, sequence,
                lexer_meta_grammar
            >
        >
    {
    };
    
    ///////////////////////////////////////////////////////////////////////////
    //  These specializations non-intrusively hook into the Lex meta-grammar.
    //  (see lex/meta_grammar.hpp)
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
