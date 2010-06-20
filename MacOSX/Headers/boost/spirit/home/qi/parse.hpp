/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(BOOST_SPIRIT_PARSE_APR_16_2006_0442PM)
#define BOOST_SPIRIT_PARSE_APR_16_2006_0442PM

#include <boost/spirit/home/qi/meta_grammar.hpp>
#include <boost/spirit/home/qi/skip.hpp>
#include <boost/spirit/home/support/unused.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/mpl/bool.hpp>

namespace boost { namespace spirit { namespace qi
{
    template <typename Iterator, typename Expr>
    inline bool
    parse(
        Iterator& first
      , Iterator last
      , Expr const& xpr)
    {
        typedef spirit::traits::is_component<qi::domain, Expr> is_component;

        // report invalid expression error as early as possible
        BOOST_MPL_ASSERT_MSG(
            is_component::value,
            xpr_is_not_convertible_to_a_parser, (Iterator, Expr));

        typedef typename result_of::as_component<qi::domain, Expr>::type component;
        typedef typename component::director director;
        component c = spirit::as_component(qi::domain(), xpr);
        return director::parse(c, first, last, unused, unused, unused);
    }

    template <typename Iterator, typename Expr, typename Attr>
    inline bool
    parse(
        Iterator& first
      , Iterator last
      , Expr const& xpr
      , Attr& attr)
    {
        typedef spirit::traits::is_component<qi::domain, Expr> is_component;

        // report invalid expression error as early as possible
        BOOST_MPL_ASSERT_MSG(
            is_component::value,
            xpr_is_not_convertible_to_a_parser, (Iterator, Expr, Attr));

        typedef typename result_of::as_component<qi::domain, Expr>::type component;
        typedef typename component::director director;
        component c = spirit::as_component(qi::domain(), xpr);
        return director::parse(c, first, last, unused, unused, attr);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename Iterator, typename Expr, typename Skipper>
    inline bool
    phrase_parse(
        Iterator& first
      , Iterator last
      , Expr const& xpr
      , Skipper const& skipper_)
    {
        typedef spirit::traits::is_component<qi::domain, Expr> expr_is_component;
        typedef spirit::traits::is_component<qi::domain, Skipper> skipper_is_component;

        // report invalid expressions error as early as possible
        BOOST_MPL_ASSERT_MSG(
            expr_is_component::value,
            xpr_is_not_convertible_to_a_parser, (Iterator, Expr, Skipper));

        BOOST_MPL_ASSERT_MSG(
            skipper_is_component::value,
            skipper_is_not_convertible_to_a_parser, (Iterator, Expr, Skipper));

        typedef typename result_of::as_component<qi::domain, Expr>::type component;
        typedef typename component::director director;
        component c = spirit::as_component(qi::domain(), xpr);

        typename result_of::as_component<qi::domain, Skipper>::type 
            skipper = spirit::as_component(qi::domain(), skipper_);

        if (!director::parse(c, first, last, unused, skipper, unused))
            return false;

        // do a final post-skip
        skip(first, last, skipper);
        return true;
    }

    template <typename Iterator, typename Expr, typename Attr, typename Skipper>
    inline bool
    phrase_parse(
        Iterator& first
      , Iterator last
      , Expr const& xpr
      , Attr& attr
      , Skipper const& skipper_)
    {
        typedef spirit::traits::is_component<qi::domain, Expr> expr_is_component;
        typedef spirit::traits::is_component<qi::domain, Skipper> skipper_is_component;

        // report invalid expressions error as early as possible
        BOOST_MPL_ASSERT_MSG(
            expr_is_component::value,
            xpr_is_not_convertible_to_a_parser, 
            (Iterator, Expr, Attr, Skipper));

        BOOST_MPL_ASSERT_MSG(
            skipper_is_component::value,
            skipper_is_not_convertible_to_a_parser, 
            (Iterator, Expr, Attr, Skipper));

        typedef typename result_of::as_component<qi::domain, Expr>::type component;
        typedef typename component::director director;
        component c = spirit::as_component(qi::domain(), xpr);

        typename result_of::as_component<qi::domain, Skipper>::type 
            skipper = spirit::as_component(qi::domain(), skipper_);

        if (!director::parse(c, first, last, unused, skipper, attr))
            return false;

        // do a final post-skip
        skip(first, last, skipper);
        return true;
    }
    
}}}

#endif

