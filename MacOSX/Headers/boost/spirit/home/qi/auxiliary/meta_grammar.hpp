/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman
    Copyright (c) 2001-2008 Hartmut Kaiser

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_SPIRIT_META_GRAMMAR_MARCH_23_2007_0537PM)
#define BOOST_SPIRIT_META_GRAMMAR_MARCH_23_2007_0537PM

#include <boost/spirit/home/qi/domain.hpp>
#include <boost/spirit/home/support/placeholders.hpp>
#include <boost/spirit/home/support/meta_grammar.hpp>
#include <boost/utility/enable_if.hpp>

namespace boost { namespace spirit
{
    template <typename T, typename Functor>
    struct functor_holder;
}}

namespace boost { namespace spirit { namespace qi
{
    ///////////////////////////////////////////////////////////////////////////
    // forwards
    ///////////////////////////////////////////////////////////////////////////
    struct main_meta_grammar;

    struct none;
    struct eps_parser;
    struct semantic_predicate;
    struct lazy_parser;
    struct functor_director;
    struct confix_director;

    struct eol_director;
    struct eoi_director;

    template <typename Positive>
    struct negated_end_director;

    ///////////////////////////////////////////////////////////////////////////
    template <typename Expr, typename Enable>
    struct is_valid_expr;

    template <typename Expr, typename Enable>
    struct expr_transform;

    ///////////////////////////////////////////////////////////////////////////
    // auxiliary parsers meta-grammar
    ///////////////////////////////////////////////////////////////////////////

    // none, eps and eps(f)
    struct auxiliary_meta_grammar1
      : proto::or_<
        // none
            meta_grammar::empty_terminal_rule<
                qi::domain, tag::none, none>
        // eps
          , meta_grammar::empty_terminal_rule<
                qi::domain, tag::eps, eps_parser>
        // eps()
          , meta_grammar::function1_rule<
                qi::domain, tag::eps, semantic_predicate>
        // lazy()
          , meta_grammar::function1_rule<
                qi::domain, tag::lazy, lazy_parser>
        // functor parser
          , meta_grammar::terminal_rule<
                qi::domain
              , functor_holder<proto::_, proto::_>
              , functor_director
            >
        // confix(..., ...)[...]
          , meta_grammar::subscript_rule<
                qi::domain, tag::confix_tag<proto::_, proto::_>, 
                confix_director, main_meta_grammar
            >
        >
    {
    };

    // eol, eoi
    struct auxiliary_end_meta_grammar
      : proto::or_<
            meta_grammar::terminal_rule<qi::domain, tag::eol, eol_director>
          , meta_grammar::terminal_rule<qi::domain, tag::eoi, eoi_director>
        >
    {
    };

    struct negated_auxiliary_end_meta_grammar
      : proto::or_<
            auxiliary_end_meta_grammar
          , meta_grammar::compose_single<
                proto::unary_expr<
                    proto::tag::complement
                  , negated_auxiliary_end_meta_grammar
                >
              , qi::domain
              , negated_end_director<mpl::_>
            >
        >
    {
    };

    struct auxiliary_meta_grammar
      : proto::or_<
            auxiliary_meta_grammar1
          , negated_auxiliary_end_meta_grammar
        >
    {
    };

    ///////////////////////////////////////////////////////////////////////////
    //  These specializations non-intrusively hooks into the RD meta-grammar.
    //  (see qi/meta_grammar.hpp)
    ///////////////////////////////////////////////////////////////////////////
    template <typename Expr>
    struct is_valid_expr<Expr
      , typename enable_if<proto::matches<Expr, auxiliary_meta_grammar> >::type>
      : mpl::true_
    {
    };

    template <typename Expr>
    struct expr_transform<Expr
      , typename enable_if<proto::matches<Expr, auxiliary_meta_grammar> >::type>
      : mpl::identity<auxiliary_meta_grammar>
    {
    };
}}}

#endif
