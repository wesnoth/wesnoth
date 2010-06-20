//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_WHAT_MAY_04_2007_0116PM)
#define BOOST_SPIRIT_WHAT_MAY_04_2007_0116PM

#include <boost/spirit/home/karma/meta_grammar.hpp>
#include <boost/mpl/assert.hpp>
#include <string>

namespace boost { namespace spirit { namespace karma
{
    template <typename Expr>
    inline std::string what(Expr const& xpr)
    {
        typedef spirit::traits::is_component<karma::domain, Expr> is_component;

        // report invalid expression error as early as possible
        BOOST_MPL_ASSERT_MSG(
            is_component::value,
            xpr_is_not_convertible_to_a_generator, ());

        typedef typename 
            spirit::result_of::as_component<karma::domain, Expr>::type 
        component;
        typedef typename component::director director;

        component c = spirit::as_component(karma::domain(), xpr);
        return director::what(c, unused);
    }
}}}

#endif

