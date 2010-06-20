/*=============================================================================
    Copyright (c) 2001-2008 Hartmut Kaiser
    Copyright (c) 2001-2007 Joel de Guzman
    Copyright (c) 2003 Gustavo Guerra

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_SPIRIT_PRINT_NODE_INFO_NOV_12_2007_1045AM)
#define BOOST_SPIRIT_PRINT_NODE_INFO_NOV_12_2007_1045AM

#include <cctype>     // iscntrl
#include <iostream>
#include <iomanip>

#include <boost/type_traits/is_convertible.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/and.hpp>

namespace boost { namespace spirit { namespace qi { namespace debug 
{

namespace detail
{
    struct token_printer_aux_for_chars
    {
        template<typename Char>
        static void print(std::ostream& o, Char c)
        {
            using namespace std;    // allow for ADL to find the proper iscntrl
            
            if (c == static_cast<Char>('\a'))
                o << "\\a";

            else if (c == static_cast<Char>('\b'))
                o << "\\b";

            else if (c == static_cast<Char>('\f'))
                o << "\\f";

            else if (c == static_cast<Char>('\n'))
                o << "\\n";

            else if (c == static_cast<Char>('\r'))
                o << "\\r";

            else if (c == static_cast<Char>('\t'))
                o << "\\t";

            else if (c == static_cast<Char>('\v'))
                o << "\\v";

            else if (iscntrl(c))
                o << "\\" << std::oct << static_cast<int>(c);

            else
                o << static_cast<char>(c);
        }
    };

    // for token types where the comparison with char constants wouldn't work
    struct token_printer_aux_for_other_types
    {
        template<typename Char>
        static void print(std::ostream& o, Char c)
        {
            o << c;
        }
    };

    template <typename Char>
    struct token_printer_aux
      : mpl::if_<
            mpl::and_<
                is_convertible<Char, char>,
                is_convertible<char, Char> 
            >,
            token_printer_aux_for_chars,
            token_printer_aux_for_other_types
        >::type
    {};

    template<typename Char>
    inline void token_printer(std::ostream& o, Char c)
    {
    // allow to customize the token printer routine
#if !defined(BOOST_SPIRIT_DEBUG_TOKEN_PRINTER)
        token_printer_aux<Char>::print(o, c);
#else
        BOOST_SPIRIT_DEBUG_TOKEN_PRINTER(o, c);
#endif
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename Iterator>
    inline void
    print_node_info(bool hit, int level, bool close, std::string const& name,
        Iterator /*first*/, Iterator const& /*last*/)
    {
        if (!name.empty())
        {
            for (int i = 0; i < level; ++i)
                BOOST_SPIRIT_DEBUG_OUT << "  ";

            if (close) {
                if (hit)
                    BOOST_SPIRIT_DEBUG_OUT << "/";
                else
                    BOOST_SPIRIT_DEBUG_OUT << "#";
            }
            
//             BOOST_SPIRIT_DEBUG_OUT << name << ":\t\"";
//             for (int j = 0; j < BOOST_SPIRIT_DEBUG_PRINT_SOME; ++j)
//             {
//                 if (first == last)
//                     break;
// 
//                 token_printer(BOOST_SPIRIT_DEBUG_OUT, *first);
//                 ++first;
//             }
//             BOOST_SPIRIT_DEBUG_OUT << "\"\n";
            BOOST_SPIRIT_DEBUG_OUT << name << "\n";
        }
    }

}
}}}}

#endif
