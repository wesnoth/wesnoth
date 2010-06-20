//  Copyright (c) 2001-2007 Joel de Guzman
//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_LEX_META_GRAMMAR_MAR_13_2007_0243PM)
#define BOOST_SPIRIT_LEX_META_GRAMMAR_MAR_13_2007_0243PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/support/meta_grammar/grammar.hpp>
#include <boost/spirit/home/support/meta_grammar/basic_transforms.hpp>
#include <boost/spirit/home/lex/domain.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/placeholders.hpp>

namespace boost { namespace spirit { namespace lex
{
    // Check if Expr is a valid lexer expression
    template <typename Expr, typename Enable = void>
    struct is_valid_expr : mpl::false_ {};

    // Return a suitable transform for the given Expr
    template <typename Expr, typename Enable = void>
    struct expr_transform;

    struct main_meta_grammar
      : meta_grammar::if_transform<
            is_valid_expr<proto::_>(),
            expr_transform<proto::_> 
        >
    {
    };
    
}}}

namespace boost { namespace spirit { namespace meta_grammar
{
    ///////////////////////////////////////////////////////////////////////////
    //  The spirit lexer domain meta-grammar
    ///////////////////////////////////////////////////////////////////////////
    template <>
    struct grammar<lex::domain>
    {
        typedef lex::main_meta_grammar type;
    };
    
}}}

#endif
