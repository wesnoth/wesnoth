// tokeniser_state.hpp
// Copyright (c) 2007 Ben Hanson (http://www.benhanson.net/)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file licence_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_LEXER_RE_TOKENISER_STATE_HPP
#define BOOST_LEXER_RE_TOKENISER_STATE_HPP

#include <locale>
#include "../../size_t.hpp"

namespace boost
{
namespace lexer
{
namespace detail
{
template<typename CharT>
struct basic_re_tokeniser_state
{
    const CharT *_curr;
    const CharT * const _end;
    bool _case_sensitive;
    std::locale _locale;
    bool _dot_not_newline;
    std::size_t _index;
    long _paren_count;
    bool _in_string;
    bool _seen_BOL_assertion;
    bool _seen_EOL_assertion;

    basic_re_tokeniser_state (const CharT *regex_, const CharT * const end_,
        const bool case_sensitive_, const std::locale locale_,
        const bool dot_not_newline_) :
        _curr (regex_),
        _end (end_),
        _case_sensitive (case_sensitive_),
        _locale (locale_),
        _dot_not_newline (dot_not_newline_),
        _index (0),
        _paren_count (0),
        _in_string (false),
        _seen_BOL_assertion (false),
        _seen_EOL_assertion (false)
    {
    }

    // prevent VC++ 7.1 warning:
    const basic_re_tokeniser_state &operator = (const basic_re_tokeniser_state &rhs_)
    {
        _curr = rhs_._curr;
        _end = rhs_._end;
        _case_sensitive = rhs_._case_sensitive;
        _locale = rhs_._locale;
        _dot_not_newline = rhs_._dot_not_newline;
        _index = rhs_._index;
        _paren_count = rhs_._paren_count;
        _in_string = rhs_._in_string;
        _seen_BOL_assertion = rhs_._seen_BOL_assertion;
        _seen_EOL_assertion = rhs_._seen_EOL_assertion;
        return this;
    }

    inline bool next (CharT &ch_)
    {
        if (_curr >= _end)
        {
            ch_ = 0;
            return true;
        }
        else
        {
            ch_ = *_curr;
            increment ();
            return false;
        }
    }

    inline void increment ()
    {
        ++_curr;
        ++_index;
    }

    inline bool eos ()
    {
        return _curr >= _end;
    }
};
}
}
}

#endif
