// consts.h
// Copyright (c) 2007 Ben Hanson (http://www.benhanson.net/)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file licence_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_LEXER_CONSTS_H
#define BOOST_LEXER_CONSTS_H

#include <boost/config.hpp>
#include <boost/integer_traits.hpp>
#include <boost/spirit/home/support/detail/lexer/size_t.hpp>

namespace boost
{
namespace lexer
{
    // 0 = end state, 1 = id, 2 = lex state, 3 = bol, 4 = eol,
    // 5 = dead_state_index
    enum {end_state_index, id_index, state_index, bol_index, eol_index,
        dead_state_index, dfa_offset};

    const std::size_t max_macro_len = 20;
    const std::size_t num_chars = 256;
    const std::size_t num_wchar_ts =
        (boost::integer_traits<wchar_t>::const_max < 0x110000) ?
        boost::integer_traits<wchar_t>::const_max : 0x110000;
    const std::size_t null_token = static_cast<std::size_t> (~0);
    const std::size_t bol_token = static_cast<std::size_t> (~1);
    const std::size_t eol_token = static_cast<std::size_t> (~2);
    const std::size_t end_state = 1;
    const std::size_t npos = static_cast<std::size_t> (~0);
}
}

#endif
