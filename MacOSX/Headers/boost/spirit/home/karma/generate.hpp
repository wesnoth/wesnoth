//  Copyright (c) 2001-2008 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_KARMA_GENERATE_FEB_20_2007_0959AM)
#define BOOST_SPIRIT_KARMA_GENERATE_FEB_20_2007_0959AM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/karma/meta_grammar.hpp>
#include <boost/spirit/home/karma/delimit.hpp>
#include <boost/spirit/home/karma/detail/output_iterator.hpp>
#include <boost/spirit/home/support/unused.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/mpl/bool.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace boost { namespace spirit { namespace karma
{
    ///////////////////////////////////////////////////////////////////////////
    template <typename OutputIterator, typename Expr>
    inline bool
    generate(OutputIterator target_sink, Expr const& xpr)
    {
        typedef spirit::traits::is_component<karma::domain, Expr> is_component;

        // report invalid expression error as early as possible
        BOOST_MPL_ASSERT_MSG(is_component::value,
            xpr_is_not_convertible_to_a_generator, 
            (OutputIterator, Expr));

        // wrap user supplied iterator into our own output iterator
        detail::output_iterator<OutputIterator> sink(target_sink);
        
        typedef
            typename result_of::as_component<karma::domain, Expr>::type
        component;
        typedef typename component::director director;

        component c = spirit::as_component(karma::domain(), xpr);
        return director::generate(c, sink, unused, unused, unused);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename OutputIterator, typename Expr>
    inline bool
    generate(detail::output_iterator<OutputIterator>& sink, Expr const& xpr)
    {
        typedef spirit::traits::is_component<karma::domain, Expr> is_component;

        // report invalid expression error as early as possible
        BOOST_MPL_ASSERT_MSG(is_component::value,
            xpr_is_not_convertible_to_a_generator, 
            (OutputIterator, Expr));

        typedef
            typename result_of::as_component<karma::domain, Expr>::type
        component;
        typedef typename component::director director;

        component c = spirit::as_component(karma::domain(), xpr);
        return director::generate(c, sink, unused, unused, unused);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename OutputIterator, typename Expr, typename Parameter>
    inline bool
    generate(OutputIterator target_sink, Expr const& xpr, Parameter const& param)
    {
        typedef spirit::traits::is_component<karma::domain, Expr> is_component;

        // report invalid expression error as early as possible
        BOOST_MPL_ASSERT_MSG(is_component::value,
            xpr_is_not_convertible_to_a_generator, 
            (OutputIterator, Expr, Parameter));

        // wrap user supplied iterator into our own output iterator
        detail::output_iterator<OutputIterator> sink(target_sink);
        
        typedef
            typename result_of::as_component<karma::domain, Expr>::type
        component;
        typedef typename component::director director;

        component c = spirit::as_component(karma::domain(), xpr);
        return director::generate(c, sink, unused, unused, param);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename OutputIterator, typename Expr, typename Parameter>
    inline bool
    generate(detail::output_iterator<OutputIterator>& sink, Expr const& xpr, 
        Parameter const& param)
    {
        typedef spirit::traits::is_component<karma::domain, Expr> is_component;

        // report invalid expression error as early as possible
        BOOST_MPL_ASSERT_MSG(is_component::value,
            xpr_is_not_convertible_to_a_generator, 
            (OutputIterator, Expr, Parameter));

        typedef
            typename result_of::as_component<karma::domain, Expr>::type
        component;
        typedef typename component::director director;

        component c = spirit::as_component(karma::domain(), xpr);
        return director::generate(c, sink, unused, unused, param);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename OutputIterator, typename Expr, typename Delimiter>
    inline bool
    generate_delimited(OutputIterator target_sink, Expr const& xpr,
        Delimiter const& delimiter)
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
            (OutputIterator, Expr, Delimiter));

        BOOST_MPL_ASSERT_MSG(delimiter_is_component::value,
            delimiter_is_not_convertible_to_a_generator, 
            (OutputIterator, Expr, Delimiter));
        
        // wrap user supplied iterator into our own output iterator
        detail::output_iterator<OutputIterator> sink(target_sink);
        
        typedef
            typename result_of::as_component<karma::domain, Expr>::type
        component;
        typedef typename component::director director;
        typedef
            typename result_of::as_component<karma::domain, Delimiter>::type
        delim_component;

        component c = spirit::as_component(karma::domain(), xpr);
        delim_component d = spirit::as_component(karma::domain(), delimiter);
        return director::generate(c, sink, unused, d, unused);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename OutputIterator, typename Expr, typename Delimiter>
    inline bool
    generate_delimited(detail::output_iterator<OutputIterator>& sink, 
        Expr const& xpr, Delimiter const& delimiter)
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
            (OutputIterator, Expr, Delimiter));

        BOOST_MPL_ASSERT_MSG(delimiter_is_component::value,
            delimiter_is_not_convertible_to_a_generator, 
            (OutputIterator, Expr, Delimiter));
        
        typedef
            typename result_of::as_component<karma::domain, Expr>::type
        component;
        typedef typename component::director director;
        typedef
            typename result_of::as_component<karma::domain, Delimiter>::type
        delim_component;

        component c = spirit::as_component(karma::domain(), xpr);
        delim_component d = spirit::as_component(karma::domain(), delimiter);
        return director::generate(c, sink, unused, d, unused);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename OutputIterator, typename Expr, typename Parameter,
        typename Delimiter>
    inline bool
    generate_delimited(OutputIterator target_sink, Expr const& xpr,
        Parameter const& param, Delimiter const& delimiter)
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
            (OutputIterator, Expr, Parameter, Delimiter));

        BOOST_MPL_ASSERT_MSG(delimiter_is_component::value,
            delimiter_is_not_convertible_to_a_generator, 
            (OutputIterator, Expr, Parameter, Delimiter));
        
        // wrap user supplied iterator into our own output iterator
        detail::output_iterator<OutputIterator> sink(target_sink);
        
        typedef
            typename result_of::as_component<karma::domain, Expr>::type
        component;
        typedef typename component::director director;
        typedef
            typename result_of::as_component<karma::domain, Delimiter>::type
        delim_component;

        component c = spirit::as_component(karma::domain(), xpr);
        delim_component d = spirit::as_component(karma::domain(), delimiter);
        return director::generate(c, sink, unused, d, param);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename OutputIterator, typename Expr, typename Parameter,
        typename Delimiter>
    inline bool
    generate_delimited(detail::output_iterator<OutputIterator>& sink, 
        Expr const& xpr, Parameter const& param, Delimiter const& delimiter)
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
            (OutputIterator, Expr, Parameter, Delimiter));

        BOOST_MPL_ASSERT_MSG(delimiter_is_component::value,
            delimiter_is_not_convertible_to_a_generator, 
            (OutputIterator, Expr, Parameter, Delimiter));
        
        typedef
            typename result_of::as_component<karma::domain, Expr>::type
        component;
        typedef typename component::director director;
        typedef
            typename result_of::as_component<karma::domain, Delimiter>::type
        delim_component;

        component c = spirit::as_component(karma::domain(), xpr);
        delim_component d = spirit::as_component(karma::domain(), delimiter);
        return director::generate(c, sink, unused, d, param);
    }

}}}

#endif

