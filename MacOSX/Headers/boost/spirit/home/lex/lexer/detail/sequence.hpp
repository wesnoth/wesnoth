//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_LEX_SEQUENCE_FEB_28_2007_0249PM)
#define BOOST_SPIRIT_LEX_SEQUENCE_FEB_28_2007_0249PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/support/unused.hpp>

namespace boost { namespace spirit { namespace lex { namespace detail
{
    template <typename LexerDef, typename String>
    struct sequence_collect
    {
        sequence_collect(LexerDef& def_, String const& state_)
          : def(def_), state(state_)
        {
        }
        
        template <typename Component>
        bool operator()(Component const& component)
        {
            Component::director::collect(component, def, state);
            return false;   // execute for all sequence elements
        }

        LexerDef& def;
        String const& state;
    };
    
}}}}  // namespace boost::spirit::lex::detail

#endif
