/*=============================================================================
    Copyright (c) 2001-2008 Hartmut Kaiser

    Distributed under the Boost Software License, Version 1.0. (See accompanying 
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(BOOST_SPIRIT_STANDARD_WIDE_NOV_10_2006_0913AM)
#define BOOST_SPIRIT_STANDARD_WIDE_NOV_10_2006_0913AM

#include <cwctype>

namespace boost { namespace spirit { namespace char_class
{
    ///////////////////////////////////////////////////////////////////////////
    //  Test characters for specified conditions (using std wchar_t functions)
    ///////////////////////////////////////////////////////////////////////////
    struct standard_wide
    {
        typedef wchar_t char_type;

        template <typename Char>
        static typename std::char_traits<Char>::int_type
        to_int_type(Char ch)
        {
            return std::char_traits<Char>::to_int_type(ch);
        }
    
        template <typename Char>
        static Char
        to_char_type(typename std::char_traits<Char>::int_type ch)
        {
            return std::char_traits<Char>::to_char_type(ch);
        }
        
        static bool 
        isalnum(wchar_t ch)
        { 
            using namespace std;
            return iswalnum(to_int_type(ch)) ? true : false; 
        }
    
        static bool 
        isalpha(wchar_t ch)
        { 
            using namespace std;
            return iswalpha(to_int_type(ch)) ? true : false; 
        }
    
        static bool 
        iscntrl(wchar_t ch)
        { 
            using namespace std;
            return iswcntrl(to_int_type(ch)) ? true : false; 
        }
    
        static bool 
        isdigit(wchar_t ch)
        { 
            using namespace std;
            return iswdigit(to_int_type(ch)) ? true : false; 
        }
    
        static bool 
        isgraph(wchar_t ch)
        { 
            using namespace std;
            return iswgraph(to_int_type(ch)) ? true : false; 
        }
    
        static bool 
        islower(wchar_t ch)
        { 
            using namespace std;
            return iswlower(to_int_type(ch)) ? true : false; 
        }
    
        static bool 
        isprint(wchar_t ch)
        { 
            using namespace std;
            return iswprint(to_int_type(ch)) ? true : false; 
        }
    
        static bool 
        ispunct(wchar_t ch)
        { 
            using namespace std;
            return iswpunct(to_int_type(ch)) ? true : false; 
        }
    
        static bool 
        isspace(wchar_t ch)
        { 
            using namespace std;
            return iswspace(to_int_type(ch)) ? true : false; 
        }
    
        static bool 
        isupper(wchar_t ch)
        { 
            using namespace std;
            return iswupper(to_int_type(ch)) ? true : false; 
        }
    
        static bool 
        isxdigit(wchar_t ch)
        { 
            using namespace std;
            return iswxdigit(to_int_type(ch)) ? true : false; 
        }
    
        static bool 
        isblank BOOST_PREVENT_MACRO_SUBSTITUTION (wchar_t ch)
        { 
            return (ch == L' ' || ch == L'\t'); 
        } 
    
        static wchar_t 
        tolower(wchar_t ch)
        { 
            using namespace std;
            return isupper(ch) ?
                to_char_type<wchar_t>(towlower(to_int_type(ch))) : ch; 
        }
    
        static wchar_t 
        toupper(wchar_t ch)
        { 
            using namespace std;
            return islower(ch) ?
                to_char_type<wchar_t>(towupper(to_int_type(ch))) : ch; 
        }
    };
}}}

#endif

