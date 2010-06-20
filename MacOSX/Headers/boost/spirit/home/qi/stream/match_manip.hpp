//  Copyright (c) 2001-2008 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_FORMAT_MANIP_MAY_05_2007_1202PM)
#define BOOST_SPIRIT_FORMAT_MANIP_MAY_05_2007_1202PM

#include <boost/spirit/home/qi/parse.hpp>
#include <boost/spirit/home/support/unused.hpp>
#include <boost/spirit/home/qi/stream/detail/match_manip.hpp>

#include <boost/mpl/assert.hpp>
#include <boost/utility/enable_if.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace boost { namespace spirit { namespace qi
{
    ///////////////////////////////////////////////////////////////////////////
    template <typename Expr>
    inline detail::match_manip<Expr>
    match(Expr const& xpr)
    {
        typedef spirit::traits::is_component<qi::domain, Expr> is_component;

        // report invalid expression error as early as possible
        BOOST_MPL_ASSERT_MSG(is_component::value,
            xpr_is_not_convertible_to_a_parser, (Expr));

        return qi::detail::match_manip<Expr>(xpr, unused, unused);
    }

    template <typename Expr, typename Attribute>
    inline detail::match_manip<Expr, Attribute>
    match(Expr const& xpr, Attribute& p)
    {
        typedef spirit::traits::is_component<qi::domain, Expr> is_component;

        // report invalid expression error as early as possible
        BOOST_MPL_ASSERT_MSG(is_component::value,
            xpr_is_not_convertible_to_a_parser, (Expr, Attribute));

        return qi::detail::match_manip<Expr, Attribute>(xpr, p, unused);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename Expr, typename Skipper>
    inline detail::match_manip<Expr, unused_type const, Skipper>
    phrase_match(Expr const& xpr, Skipper const& s)
    {
        typedef
            spirit::traits::is_component<qi::domain, Expr>
        expr_is_component;
        typedef
            spirit::traits::is_component<qi::domain, Skipper>
        skipper_is_component;

        // report invalid expression errors as early as possible
        BOOST_MPL_ASSERT_MSG(expr_is_component::value,
            xpr_is_not_convertible_to_a_parser, (Expr, Skipper));

        BOOST_MPL_ASSERT_MSG(skipper_is_component::value,
            skipper_is_not_convertible_to_a_parser, (Expr, Skipper));

        return qi::detail::match_manip<Expr, unused_type const, Skipper>(
            xpr, unused, s);
    }

    template <typename Expr, typename Attribute, typename Skipper>
    inline detail::match_manip<Expr, Attribute, Skipper>
    phrase_match(Expr const& xpr, Attribute& p, Skipper const& s)
    {
        typedef
            spirit::traits::is_component<qi::domain, Expr>
        expr_is_component;
        typedef
            spirit::traits::is_component<qi::domain, Skipper>
        skipper_is_component;

        // report invalid expression errors as early as possible
        BOOST_MPL_ASSERT_MSG(expr_is_component::value,
            xpr_is_not_convertible_to_a_parser, (Expr, Attribute, Skipper));

        BOOST_MPL_ASSERT_MSG(skipper_is_component::value,
            skipper_is_not_convertible_to_a_parser, (Expr, Attribute, Skipper));

        return qi::detail::match_manip<Expr, Attribute, Skipper>(xpr, p, s);
    }

    ///////////////////////////////////////////////////////////////////////////
    template<typename Char, typename Traits, typename Expr>
    inline typename
        enable_if<
            spirit::traits::is_component<qi::domain, Expr>,
            std::basic_istream<Char, Traits> &
        >::type
    operator>> (std::basic_istream<Char, Traits> &is, Expr& xpr)
    {
        typedef std::istream_iterator<Char, Char, Traits> input_iterator;
        input_iterator f(is);
        input_iterator l;
        if (!qi::parse (f, l, xpr))
        {
            is.setstate(std::ios_base::failbit);
        }
        return is;
    }

}}}

#endif

