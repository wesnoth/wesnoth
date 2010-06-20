/*=============================================================================
    Copyright (c) 2001-2008 Hartmut Kaiser

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_SPIRIT_SIMPLE_DEBUG_MACROS_NOV_12_2007_0828AM)
#define BOOST_SPIRIT_SIMPLE_DEBUG_MACROS_NOV_12_2007_0828AM

#if !defined(BOOST_SPIRIT_DEBUG_NOV_12_2007_0827AM)
#error "You must include boost/spirit/home/qi/debug.hpp, not this file"
#endif

///////////////////////////////////////////////////////////////////////////////
//  Make sure all required debug helper macros are defined properly
#if !defined(BOOST_SPIRIT_DEBUG_TRACE_NODE_NAME)
#define BOOST_SPIRIT_DEBUG_TRACE_NODE_NAME(r, n, t)                           \
        r.name(n);                                                            \
        boost::spirit::qi::debug::enable_simple_debug_support(r, t)           \
    /**/
#endif

#if !defined(BOOST_SPIRIT_DEBUG_TRACE_NODE)
#define BOOST_SPIRIT_DEBUG_TRACE_NODE(r, t)                                   \
        if (r.name().empty()) r.name(#r);                                     \
        boost::spirit::qi::debug::enable_simple_debug_support(r, t)           \
    /**/
#endif

#if !defined(BOOST_SPIRIT_DEBUG_NODE)
#define BOOST_SPIRIT_DEBUG_NODE(r)                                            \
        BOOST_SPIRIT_DEBUG_TRACE_NODE(r, true)                                \
    /**/
#endif

//  number of input tokens to print while debugging
#if !defined(BOOST_SPIRIT_DEBUG_PRINT_SOME)
#define BOOST_SPIRIT_DEBUG_PRINT_SOME 20
#endif

//  The stream to use for debug output
#if !defined(BOOST_SPIRIT_DEBUG_OUT)
#define BOOST_SPIRIT_DEBUG_OUT std::cerr
#endif

#endif
