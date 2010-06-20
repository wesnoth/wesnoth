//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_KARMA_ALIGNMENT_META_GRAMMAR_FEB_21_2007_0826PM)
#define BOOST_SPIRIT_KARMA_ALIGNMENT_META_GRAMMAR_FEB_21_2007_0826PM

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
    
    struct simple_left_aligment;
    struct simple_right_aligment;
    struct simple_center_aligment;
    
    struct width_left_aligment;
    struct width_right_aligment;
    struct width_center_aligment;
    
    struct padding_left_aligment;
    struct padding_right_aligment;
    struct padding_center_aligment;
    
    struct full_left_aligment;
    struct full_right_aligment;
    struct full_center_aligment;
    
    ///////////////////////////////////////////////////////////////////////////
    template <typename Expr, typename Enable>
    struct is_valid_expr;

    template <typename Expr, typename Enable>
    struct expr_transform;

    ///////////////////////////////////////////////////////////////////////////
    //  left, right and center directive meta-grammars
    ///////////////////////////////////////////////////////////////////////////
    struct simple_align_directive_meta_grammar 
      : proto::or_<
            meta_grammar::binary_rule<
                karma::domain, proto::tag::subscript, simple_left_aligment,
                proto::terminal<tag::left_align>, main_meta_grammar
            >,
            meta_grammar::binary_rule<
                karma::domain, proto::tag::subscript, simple_right_aligment,
                proto::terminal<tag::right_align>, main_meta_grammar
            >,
            meta_grammar::binary_rule<
                karma::domain, proto::tag::subscript, simple_center_aligment,
                proto::terminal<tag::center>, main_meta_grammar
            >
        >
    {};
    
    ///////////////////////////////////////////////////////////////////////////
    //  matches alignment directives defining the width only: 
    //  left_align(width)[...], right_align(width)[...], center(width)[...]
    ///////////////////////////////////////////////////////////////////////////
    struct width_align_directive_meta_grammar 
      : proto::or_<
            meta_grammar::subscript_function1_rule<
                karma::domain, tag::left_align, width_left_aligment,
                proto::terminal<int>, main_meta_grammar
            >,
            meta_grammar::subscript_function1_rule<
                karma::domain, tag::right_align, width_right_aligment,
                proto::terminal<int>, main_meta_grammar
            >,
            meta_grammar::subscript_function1_rule<
                karma::domain, tag::center, width_center_aligment,
                proto::terminal<int>, main_meta_grammar
            >
        >
    {};
    
    ///////////////////////////////////////////////////////////////////////////
    //  matches alignment directives defining the padding generator only: 
    //  left_align(padding)[...], right_align(padding)[...], center(padding)[...]
    ///////////////////////////////////////////////////////////////////////////
    struct padding_align_directive_meta_grammar 
      : proto::or_<
            meta_grammar::subscript_function1_rule<
                karma::domain, tag::left_align, padding_left_aligment,
                main_meta_grammar, main_meta_grammar
            >,
            meta_grammar::subscript_function1_rule<
                karma::domain, tag::right_align, padding_right_aligment,
                main_meta_grammar, main_meta_grammar
            >,
            meta_grammar::subscript_function1_rule<
                karma::domain, tag::center, padding_center_aligment,
                main_meta_grammar, main_meta_grammar
            >
        >
    {};
    
    ///////////////////////////////////////////////////////////////////////////
    //  matches full alignment directives: left_align(width, padding)[...],
    //  right_align(width, padding)[...], center(width, padding)[...]
    ///////////////////////////////////////////////////////////////////////////
    struct full_align_directive_meta_grammar 
      : proto::or_<
            meta_grammar::subscript_function2_rule<
                karma::domain, tag::left_align, full_left_aligment,
                proto::terminal<int>, main_meta_grammar, main_meta_grammar
            >,
            meta_grammar::subscript_function2_rule<
                karma::domain, tag::right_align, full_right_aligment,
                proto::terminal<int>, main_meta_grammar, main_meta_grammar
            >,
            meta_grammar::subscript_function2_rule<
                karma::domain, tag::center, full_center_aligment,
                proto::terminal<int>, main_meta_grammar, main_meta_grammar
            >
        >
    {};
    
    // main alignment_directive_meta_grammar
    struct alignment_directive_meta_grammar
      : proto::or_<
            simple_align_directive_meta_grammar,
            width_align_directive_meta_grammar,
            padding_align_directive_meta_grammar,
            full_align_directive_meta_grammar
        >
    {};

    ///////////////////////////////////////////////////////////////////////////
    //  These specializations non-intrusively hooks into the Karma meta-grammar.
    //  (see karma/meta_grammar.hpp)
    ///////////////////////////////////////////////////////////////////////////  
    template <typename Expr>
    struct is_valid_expr<
            Expr,
            typename enable_if<
                proto::matches<Expr, alignment_directive_meta_grammar> 
            >::type
        >
      : mpl::true_
    {
    };

    template <typename Expr>
    struct expr_transform<
            Expr,
            typename enable_if<
                    proto::matches<Expr, alignment_directive_meta_grammar> 
            >::type
        >
      : mpl::identity<alignment_directive_meta_grammar>
    {
    };
    
}}}

#endif
