//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_LEX_CHAR_TOKEN_DEF_MAR_28_2007_0626PM)
#define BOOST_SPIRIT_LEX_CHAR_TOKEN_DEF_MAR_28_2007_0626PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/support/component.hpp>

namespace boost { namespace spirit { namespace lex
{ 
    ///////////////////////////////////////////////////////////////////////////
    //
    //  char_token_def 
    //      represents a single character token definition
    //
    ///////////////////////////////////////////////////////////////////////////
    struct char_token_def
    {
        template <typename Component, typename LexerDef, typename String>
        static void 
        collect(Component const& component, LexerDef& lexdef, 
            String const& state)
        {
            typedef typename result_of::subject<Component>::type char_type;
            
            char_type c = subject(component);
            lexdef.add_token (state.c_str(), lex::detail::escape(c), 
                static_cast<std::size_t>(c));
        }
    };
                
}}}  // namespace boost::spirit::lex

#endif 
