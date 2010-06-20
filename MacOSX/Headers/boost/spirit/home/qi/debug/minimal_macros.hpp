/*=============================================================================
    Copyright (c) 2001-2008 Hartmut Kaiser

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_SPIRIT_MINIMAL_MACROS_NOV_12_2007_1047AM)
#define BOOST_SPIRIT_MINIMAL_MACROS_NOV_12_2007_1047AM

#if !defined(BOOST_SPIRIT_DEBUG_NOV_12_2007_0827AM)
#error "You must include boost/spirit/home/qi/debug.hpp, not this file"
#endif

///////////////////////////////////////////////////////////////////////////////
//  Minimum debugging tools support
#if !defined(BOOST_SPIRIT_DEBUG_OUT)
#define BOOST_SPIRIT_DEBUG_OUT std::cerr
#endif

///////////////////////////////////////////////////////////////////////////////
//  Empty implementations of the debug macros above, if no debug support is 
//  required
#if !defined(BOOST_SPIRIT_DEBUG_TRACE_NODE_NAME)
#define BOOST_SPIRIT_DEBUG_TRACE_NODE_NAME(r, n, t)
#endif

#if !defined(BOOST_SPIRIT_DEBUG_TRACE_NODE)
#define BOOST_SPIRIT_DEBUG_TRACE_NODE(r, t)
#endif

#if !defined(BOOST_SPIRIT_DEBUG_NODE)
#define BOOST_SPIRIT_DEBUG_NODE(r)
#endif

#endif


