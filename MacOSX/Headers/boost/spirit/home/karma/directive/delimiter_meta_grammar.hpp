//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_KARMA_DELIMITER_META_GRAMMAR_FEB_21_2007_0826PM)
#define BOOST_SPIRIT_KARMA_DELIMITER_META_GRAMMAR_FEB_21_2007_0826PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/support/modifier.hpp>
#include <boost/spirit/home/support/meta_grammar.hpp>
#include <boost/spirit/home/karma/domain.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/or.hpp>

namespace boost { namespace spirit { namespace karma
{
    ///////////////////////////////////////////////////////////////////////////
    // forwards
    ///////////////////////////////////////////////////////////////////////////
    struct main_meta_grammar;
    
    struct delimit_;
    struct delimit_space;
    struct verbatim;
    
    ///////////////////////////////////////////////////////////////////////////
    template <typename Expr, typename Enable>
    struct is_valid_expr;

    template <typename Expr, typename Enable>
    struct expr_transform;

    ///////////////////////////////////////////////////////////////////////////
    //  delimit and verbatim directive meta-grammars
    //  delimit[...], delimit(delimiter)[...] and verbatim[...]
    ///////////////////////////////////////////////////////////////////////////
    struct delimiter_directive_meta_grammar
      : proto::or_<
            meta_grammar::binary_rule<
                karma::domain, proto::tag::subscript, verbatim,
                proto::terminal<tag::verbatim>, main_meta_grammar
            >,
            meta_grammar::binary_rule<
                karma::domain, proto::tag::subscript, delimit_space,
                proto::terminal<tag::delimit>, main_meta_grammar
            >,
            meta_grammar::subscript_function1_rule<
                karma::domain, tag::delimit, delimit_,
                main_meta_grammar, main_meta_grammar
            >
        >
    {};

    ///////////////////////////////////////////////////////////////////////////
    //  These specializations non-intrusively hook into the Karma meta-grammar.
    //  (see karma/meta_grammar.hpp)
    ///////////////////////////////////////////////////////////////////////////  
    template <typename Expr>
    struct is_valid_expr<
            Expr,
            typename enable_if<
                proto::matches<Expr, delimiter_directive_meta_grammar> 
            >::type
        >
      : mpl::true_
    {
    };

    template <typename Expr>
    struct expr_transform<
            Expr,
            typename enable_if<
                proto::matches<Expr, delimiter_directive_meta_grammar> 
            >::type
        >
      : mpl::identity<delimiter_directive_meta_grammar>
    {
    };
    
}}}

#endif
