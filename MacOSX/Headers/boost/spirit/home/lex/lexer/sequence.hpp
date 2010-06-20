//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(SPIRIT_LEX_SEQUENCE_MAR_28_2007_0610PM)
#define SPIRIT_LEX_SEQUENCE_MAR_28_2007_0610PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/lex/lexer/detail/sequence.hpp>
#include <boost/fusion/include/any.hpp>

namespace boost { namespace spirit { namespace lex
{
    struct sequence
    {
        template <typename Component, typename LexerDef, typename String>
        static void 
        collect(Component const& component, LexerDef& lexdef, 
            String const& state)
        {
            detail::sequence_collect<LexerDef, String> f (lexdef, state);
            fusion::any(component.elements, f);
        }
    };
    
}}} // namespace boost::spirit::lex

#endif
