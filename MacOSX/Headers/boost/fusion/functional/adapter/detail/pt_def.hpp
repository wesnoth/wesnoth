/*=============================================================================
    Copyright (c) 2006-2007 Tobias Schwinger
  
    Use modification and distribution are subject to the Boost Software 
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt).
==============================================================================*/

// No include guard - this file is included multiple times intentionally.

#if BOOST_PP_SLOT_1() & 0x001
#   define PT0 T0 &
#else
#   define PT0 T0 const &
#endif
#if BOOST_PP_SLOT_1() & 0x002
#   define PT1 T1 &
#else
#   define PT1 T1 const &
#endif
#if BOOST_PP_SLOT_1() & 0x004
#   define PT2 T2 &
#else
#   define PT2 T2 const &
#endif
#if BOOST_PP_SLOT_1() & 0x008
#   define PT3 T3 &
#else
#   define PT3 T3 const &
#endif
#if BOOST_PP_SLOT_1() & 0x010
#   define PT4 T4 &
#else
#   define PT4 T4 const &
#endif
#if BOOST_PP_SLOT_1() & 0x020
#   define PT5 T5 &
#else
#   define PT5 T5 const &
#endif
#if BOOST_PP_SLOT_1() & 0x040
#   define PT6 T6 &
#else
#   define PT6 T6 const &
#endif
#if BOOST_PP_SLOT_1() & 0x080
#   define PT7 T7 &
#else
#   define PT7 T7 const &
#endif
#if BOOST_PP_SLOT_1() & 0x100
#   define PT8 T8 &
#else
#   define PT8 T8 const &
#endif
#if BOOST_PP_SLOT_1() & 0x200
#   define PT9 T9 &
#else
#   define PT9 T9 const &
#endif
#if BOOST_PP_SLOT_1() & 0x400
#   define PT10 T10 &
#else
#   define PT10 T10 const &
#endif
#if BOOST_PP_SLOT_1() & 0x800
#   define PT11 T11 &
#else
#   define PT11 T11 const &
#endif

