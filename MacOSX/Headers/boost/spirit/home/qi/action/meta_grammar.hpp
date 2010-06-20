/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying 
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_SPIRIT_META_GRAMMAR_FEB_07_2007_1100AM)
#define BOOST_SPIRIT_META_GRAMMAR_FEB_07_2007_1100AM

#include <boost/spirit/home/qi/domain.hpp>
#include <boost/spirit/home/support/meta_grammar.hpp>
#include <boost/utility/enable_if.hpp>

namespace boost { namespace spirit { namespace qi
{
    ///////////////////////////////////////////////////////////////////////////
    // forwards
    /////////////////////////////////////////////////////////////////////////// 
    struct action;
    struct main_meta_grammar;

    template <typename Expr, typename Enable>
    struct is_valid_expr;

    template <typename Expr, typename Enable>
    struct expr_transform;

    ///////////////////////////////////////////////////////////////////////////
    // action meta-grammar
    ///////////////////////////////////////////////////////////////////////////
    struct action_meta_grammar : 
        meta_grammar::binary_rule<
            qi::domain, proto::tag::subscript, action
          , main_meta_grammar, proto::when<proto::_, proto::_arg>
        >
    {
    };

    ///////////////////////////////////////////////////////////////////////////
    //  These specializations non-intrusively hooks into the RD meta-grammar.
    //  (see qi/meta_grammar.hpp)
    ///////////////////////////////////////////////////////////////////////////
    template <typename Expr>
    struct is_valid_expr<Expr
      , typename enable_if<proto::matches<Expr, action_meta_grammar> >::type>
      : mpl::true_
    {
    };

    template <typename Expr>
    struct expr_transform<Expr
      , typename enable_if<proto::matches<Expr, action_meta_grammar> >::type>
      : mpl::identity<action_meta_grammar>
    {
    };
}}}

#endif
