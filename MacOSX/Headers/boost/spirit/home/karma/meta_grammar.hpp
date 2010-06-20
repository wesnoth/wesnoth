//  Copyright (c) 2001-2007 Joel de Guzman
//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_KARMA_META_GRAMMAR_FEB_20_2007_0939AM)
#define BOOST_SPIRIT_KARMA_META_GRAMMAR_FEB_20_2007_0939AM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/support/meta_grammar/grammar.hpp>
#include <boost/spirit/home/support/meta_grammar/basic_transforms.hpp>
#include <boost/spirit/home/karma/domain.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/placeholders.hpp>

namespace boost { namespace spirit { namespace karma
{
    // Check if Expr is a valid Karma expression
    template <typename Expr, typename Enable = void>
    struct is_valid_expr : mpl::false_ {};

    // Return a suitable transform for the given Expr
    template <typename Expr, typename Enable = void>
    struct expr_transform;

    struct main_meta_grammar
      : meta_grammar::if_transform<
            is_valid_expr<proto::_>(),
            expr_transform<proto::_> 
        >
    {
    };
    
}}}

namespace boost { namespace spirit { namespace meta_grammar
{
    ///////////////////////////////////////////////////////////////////////////
    //  The spirit karma domain meta-grammar
    ///////////////////////////////////////////////////////////////////////////
    template <>
    struct grammar<karma::domain>
    {
        typedef karma::main_meta_grammar type;
    };
    
}}}

#endif
