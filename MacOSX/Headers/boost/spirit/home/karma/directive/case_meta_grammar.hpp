//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_KARMA_CASE_META_GRAMMAR_FEB_21_2007_0826PM)
#define BOOST_SPIRIT_KARMA_CASE_META_GRAMMAR_FEB_21_2007_0826PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/support/modifier.hpp>
#include <boost/spirit/home/support/meta_grammar.hpp>
#include <boost/spirit/home/karma/domain.hpp>
#include <boost/spirit/home/support/iso8859_1.hpp>
#include <boost/spirit/home/support/ascii.hpp>
#include <boost/spirit/home/support/standard.hpp>
#include <boost/spirit/home/support/standard_wide.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/or.hpp>

namespace boost { namespace spirit { namespace karma
{
    ///////////////////////////////////////////////////////////////////////////
    // forwards
    ///////////////////////////////////////////////////////////////////////////
    struct main_meta_grammar;
    
    ///////////////////////////////////////////////////////////////////////////
    template <typename Expr, typename Enable>
    struct is_valid_expr;

    template <typename Expr, typename Enable>
    struct expr_transform;

    ///////////////////////////////////////////////////////////////////////////
    //  lower and upper directive meta-grammars
    ///////////////////////////////////////////////////////////////////////////
    struct lower_case_directive_meta_grammar 
      : meta_grammar::deep_directive_meta_grammar<
            spirit::char_class::lower_case_tag<proto::_>,
            main_meta_grammar
        >
    {};

    struct upper_case_directive_meta_grammar  
      : meta_grammar::deep_directive_meta_grammar<
            spirit::char_class::upper_case_tag<proto::_>,
            main_meta_grammar
        >
    {};

    // main directive_meta_grammar
    struct directive_meta_grammar
      : proto::or_<
            lower_case_directive_meta_grammar,
            upper_case_directive_meta_grammar
        >
    {};

    ///////////////////////////////////////////////////////////////////////////
    //  These specializations non-intrusively hooks into the Karma meta-grammar.
    //  (see karma/meta_grammar.hpp)
    ///////////////////////////////////////////////////////////////////////////  
    template <typename Expr>
    struct is_valid_expr<Expr,
        typename enable_if<proto::matches<Expr, directive_meta_grammar> >::type>
      : mpl::true_
    {
    };

    template <typename Expr>
    struct expr_transform<Expr,
        typename enable_if<proto::matches<Expr, directive_meta_grammar> >::type>
      : mpl::identity<directive_meta_grammar>
    {
    };
    
}}}

namespace boost { namespace spirit 
{
    ///////////////////////////////////////////////////////////////////////////
    //  The following specializations for the add_modifier template are added
    //  to allow for special handling of the lower[] and upper[] directives
    //  which are mutually exclusive. Since the first of these directives 
    //  added to the modifier corresponds to the outermost one in the generator
    //  expression, we just ignore the request to add a tag if one of the two
    //  has been already added.
    template <typename Modifier, typename Tag>
    struct add_modifier<Modifier, spirit::char_class::lower_case_tag<Tag> >
    {
        // add the new tag to the modifier (if it isn't already)
        typedef spirit::char_class::upper_case_tag<Tag> reciprocal_tag;
        typedef spirit::char_class::lower_case_tag<Tag> tag;

        typedef typename
            mpl::if_<
                mpl::or_<
                    is_member_of_modifier<Modifier, reciprocal_tag>,
                    is_member_of_modifier<Modifier, tag>
                >,
                Modifier,
                modifier<Modifier, tag>
            >::type 
        type;
    };
    
    ///////////////////////////////////////////////////////////////////////////
    template <typename Modifier, typename Tag>
    struct add_modifier<Modifier, spirit::char_class::upper_case_tag<Tag> >
    {
        // add the new tag to the modifier (if it isn't already)
        typedef spirit::char_class::lower_case_tag<Tag> reciprocal_tag;
        typedef spirit::char_class::upper_case_tag<Tag> tag;
        
        typedef typename
            mpl::if_<
                mpl::or_<
                    is_member_of_modifier<Modifier, reciprocal_tag>,
                    is_member_of_modifier<Modifier, tag>
                >,
                Modifier,
                modifier<Modifier, tag>
            >::type 
        type;
    };
    
}}

#endif
