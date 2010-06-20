//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_LEX_LEXER_ACTIONS_FEB_13_2008_1232PM)
#define BOOST_SPIRIT_LEX_LEXER_ACTIONS_FEB_13_2008_1232PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <iosfwd>

///////////////////////////////////////////////////////////////////////////////
namespace boost { namespace spirit { namespace lex 
{ 
    ///////////////////////////////////////////////////////////////////////////
    template <typename Char, typename Traits>
    struct echo_input_functor
    {
        echo_input_functor (std::basic_ostream<Char, Traits>& os_)
          : os(os_)
        {
        }
        
        template <typename Range, typename LexerContext>
        void operator()(Range const& r, std::size_t, bool&, LexerContext&) const
        {
            os << r;
        }
        
        std::basic_ostream<Char, Traits>& os;
    };
    
    ///////////////////////////////////////////////////////////////////////////
    template <typename Char, typename Traits>
    inline echo_input_functor<Char, Traits> 
    echo_input(std::basic_ostream<Char, Traits>& os)
    {
        return echo_input_functor<Char, Traits>(os);
    }
    
///////////////////////////////////////////////////////////////////////////////
}}}

#endif
