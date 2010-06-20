//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_LEX_LEXER_FWD_MAR_22_2007_1137PM)
#define BOOST_SPIRIT_LEX_LEXER_FWD_MAR_22_2007_1137PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/support/unused.hpp>

namespace boost { namespace spirit { namespace lex
{
    ///////////////////////////////////////////////////////////////////////////
    //  This component represents a token definition
    ///////////////////////////////////////////////////////////////////////////
    template<typename Attribute = unused_type, typename Char = char, 
        typename Idtype = std::size_t>
    class token_def;

    ///////////////////////////////////////////////////////////////////////////
    //  token_set
    ///////////////////////////////////////////////////////////////////////////
    template <typename TokenSet>
    class token_set;
    
    ///////////////////////////////////////////////////////////////////////////
    //  This represents a lexer definition (helper for token and token set 
    //  definitions)
    ///////////////////////////////////////////////////////////////////////////
    template <typename Lexer>
    class lexer_def;
    
    ///////////////////////////////////////////////////////////////////////////
    //  This represents a lexer object
    ///////////////////////////////////////////////////////////////////////////
    template <typename Definition>
    class lexer;
    
}}}

#endif
