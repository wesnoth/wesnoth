//  Copyright (c) 2001-2008 Hartmut Kaiser
//  Copyright (c) 2001-2007 Joel de Guzman
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_LEX_TERMINAL_HOLDER_MAR_22_2007_0217PM)
#define BOOST_SPIRIT_LEX_TERMINAL_HOLDER_MAR_22_2007_0217PM

#include <boost/xpressive/proto/proto.hpp>

namespace boost { namespace spirit { namespace lex
{
    template <typename T, typename Terminal>
    struct terminal_holder
    {
        typedef Terminal terminal_type;
        T held;
    };

    template <typename T, typename Terminal>
    struct make_terminal_holder
      : proto::terminal<terminal_holder<T, Terminal> >
    {
    };

}}}

#endif
