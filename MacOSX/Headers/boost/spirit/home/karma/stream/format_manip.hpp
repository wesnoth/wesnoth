//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_KARMA_FORMAT_MANIP_MAY_01_2007_1211PM)
#define BOOST_SPIRIT_KARMA_FORMAT_MANIP_MAY_01_2007_1211PM

#include <boost/spirit/home/karma/generate.hpp>
#include <boost/spirit/home/support/unused.hpp>
#include <boost/spirit/home/karma/stream/detail/format_manip.hpp>

#include <boost/mpl/assert.hpp>
#include <boost/utility/enable_if.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace boost { namespace spirit { namespace karma 
{
    ///////////////////////////////////////////////////////////////////////////
    template <typename Expr>
    inline detail::format_manip<Expr> 
    format(Expr const& xpr)
    {
        typedef spirit::traits::is_component<karma::domain, Expr> is_component;

        // report invalid expression error as early as possible
        BOOST_MPL_ASSERT_MSG(is_component::value,
            xpr_is_not_convertible_to_a_generator, (Expr));

        return karma::detail::format_manip<Expr>(xpr, unused, unused);
    }

    template <typename Expr, typename Parameter>
    inline detail::format_manip<Expr, Parameter> 
    format(Expr const& xpr, Parameter const& p)
    {
        typedef spirit::traits::is_component<karma::domain, Expr> is_component;

        // report invalid expression error as early as possible
        BOOST_MPL_ASSERT_MSG(is_component::value,
            xpr_is_not_convertible_to_a_generator, (Expr, Parameter));

        return karma::detail::format_manip<Expr, Parameter>(xpr, p, unused);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename Expr, typename Delimiter>
    inline detail::format_manip<Expr, unused_type, Delimiter> 
    format_delimited(Expr const& xpr, Delimiter const& d)
    {
        typedef
            spirit::traits::is_component<karma::domain, Expr>
        expr_is_component;
        typedef
            spirit::traits::is_component<karma::domain, Delimiter>
        delimiter_is_component;

        // report invalid expression errors as early as possible
        BOOST_MPL_ASSERT_MSG(expr_is_component::value,
            xpr_is_not_convertible_to_a_generator, (Expr, Delimiter));

        BOOST_MPL_ASSERT_MSG(delimiter_is_component::value,
            delimiter_is_not_convertible_to_a_generator, (Expr, Delimiter));

        return karma::detail::format_manip<Expr, unused_type, Delimiter>(
            xpr, unused, d);
    }

    template <typename Expr, typename Parameter, typename Delimiter>
    inline detail::format_manip<Expr, Parameter, Delimiter> 
    format_delimited(Expr const& xpr, Parameter const& p, Delimiter const& d)
    {
        typedef
            spirit::traits::is_component<karma::domain, Expr>
        expr_is_component;
        typedef
            spirit::traits::is_component<karma::domain, Delimiter>
        delimiter_is_component;

        // report invalid expression errors as early as possible
        BOOST_MPL_ASSERT_MSG(expr_is_component::value,
            xpr_is_not_convertible_to_a_generator, 
            (Expr, Parameter, Delimiter));

        BOOST_MPL_ASSERT_MSG(delimiter_is_component::value,
            delimiter_is_not_convertible_to_a_generator, 
            (Expr, Parameter, Delimiter));

        return karma::detail::format_manip<Expr, Parameter, Delimiter>(
            xpr, p, d);
    }

    ///////////////////////////////////////////////////////////////////////////
    template<typename Char, typename Traits, typename Expr> 
    inline typename 
        enable_if<
            spirit::traits::is_component<karma::domain, Expr>,
            std::basic_ostream<Char, Traits> & 
        >::type
    operator<< (std::basic_ostream<Char, Traits> &os, Expr const& xpr)
    {
        karma::detail::ostream_iterator<Char, Char, Traits> sink(os);
        if (!karma::generate (sink, xpr))
        {
            os.setstate(std::ios_base::failbit);
        }
        return os;
    }
    
}}}

#endif 

