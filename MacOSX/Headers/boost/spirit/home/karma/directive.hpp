//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_KARMA_DIRECTIVE_FEB_21_2007_0833PM)
#define BOOST_SPIRIT_KARMA_DIRECTIVE_FEB_21_2007_0833PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//  directives related to alignment 
//  left_align[...], right_align[...], center[...]
///////////////////////////////////////////////////////////////////////////////
#include <boost/spirit/home/karma/directive/left_alignment.hpp>
#include <boost/spirit/home/karma/directive/right_alignment.hpp>
#include <boost/spirit/home/karma/directive/center_alignment.hpp>
#include <boost/spirit/home/karma/directive/alignment_meta_grammar.hpp>

///////////////////////////////////////////////////////////////////////////////
//  directives related to character case
//  lower[...] and upper[...]
///////////////////////////////////////////////////////////////////////////////
#include <boost/spirit/home/karma/directive/case_meta_grammar.hpp>

///////////////////////////////////////////////////////////////////////////////
//  directives related to delimiting generators 
//  delimit[...] and verbatim[...]
///////////////////////////////////////////////////////////////////////////////
#include <boost/spirit/home/karma/directive/verbatim.hpp>
#include <boost/spirit/home/karma/directive/delimit.hpp>
#include <boost/spirit/home/karma/directive/delimiter_meta_grammar.hpp>

#endif
