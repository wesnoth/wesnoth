//  (C) Copyright John Maddock 2005.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

/*
 * The gcc include path logic is derived from STLport:
 *
 * Copyright (c) 1994
 * Hewlett-Packard Company
 *
 * Copyright (c) 1996-1999
 * Silicon Graphics Computer Systems, Inc.
 *
 * Copyright (c) 1997
 * Moscow Center for SPARC Technology
 *
 * Copyright (c) 1999-2003
 * Boris Fomitchev
 *
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use or copy this software for any purpose is hereby granted 
 * without fee, provided the above notices are retained on all copies.
 * Permission to modify the code and to distribute modified code is granted,
 * provided the above notices are retained, and a notice that the code was
 * modified is included with the above copyright notice.
 *
 */

#ifndef BOOST_TR1_DETAIL_CONFIG_ALL_HPP_INCLUDED
#  define BOOST_TR1_DETAIL_CONFIG_ALL_HPP_INCLUDED

//
// IMPORTANT: we must figure out the basics, such as how to
// forward to the real std lib headers *without* including
// boost/config.hpp or any of the std lib headers.  A classic 
// chicken and the egg problem....
//
// Including <cstddef> at least lets us detect STLport:
//
#include <cstddef>

#  if (defined(__SGI_STL_PORT) || defined(_STLPORT_VERSION)) && !defined(__BORLANDC__)
#     ifdef __SUNPRO_CC
         // can't use <../stlport/name> since some compilers put stlport in a different directory:
#        define BOOST_TR1_STD_HEADER(name) <../stlport4/name>
#     elif defined(__PGI)
#        define BOOST_TR1_STD_HEADER(name) <../CC/name>
#     else
#        define BOOST_TR1_STD_HEADER(name) <../stlport/name>
#     endif

#  elif defined(__HP_aCC)
      // HP aCC include path:
#     define BOOST_TR1_STD_HEADER(name) <../include_std/name>

#  elif defined(__DECCXX)
#     define BOOST_TR1_STD_HEADER(name) <../cxx/name>

#  elif defined(__BORLANDC__) && __BORLANDC__ >= 0x570
#     define BOOST_TR1_STD_HEADER(name) <../include/dinkumware/name>

#  elif defined(__GNUC__) && __GNUC__ >= 3
#    if defined(BOOST_TR1_GCC_INCLUDE_PATH)
#      define BOOST_TR1_STD_HEADER(name) <../BOOST_TR1_GCC_INCLUDE_PATH/name>
#      ifndef BOOST_TR1_DISABLE_INCLUDE_NEXT
#        define BOOST_TR1_DISABLE_INCLUDE_NEXT
#      endif
#    elif ( (__GNUC__ == 3 ) && ((__GNUC_MINOR__ == 0) || ((__GNUC_MINOR__ < 3) && defined(__APPLE_CC__))))
#      define BOOST_TR1_STD_HEADER(name) <../g++-v3/name>
#    else
#      if ( ((__GNUC__ == 4 ) || (__GNUC_MINOR__ >= 3)) && defined(__APPLE_CC__))
#        define BOOST_TR1_STD_HEADER(name) <../c++/name>
         /*
          *  Before version 3.4.0 the 0 patch level was not part of the include path:
          */
#      elif defined (__GNUC_PATCHLEVEL__) && ((__GNUC_PATCHLEVEL__ > 0) || \
                                              (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || \
                                              (__GNUC__ > 3))
#        define BOOST_TR1_STD_HEADER(name) <../__GNUC__.__GNUC_MINOR__.__GNUC_PATCHLEVEL__/name>
#      else
#        define BOOST_TR1_STD_HEADER(name) <../__GNUC__.__GNUC_MINOR__/name>
#      endif
#    endif

#  else
#     define BOOST_TR1_STD_HEADER(name) <../include/name>
#  endif

#if defined(__GNUC__) && !defined(BOOST_HAS_INCLUDE_NEXT)
#  define BOOST_HAS_INCLUDE_NEXT
#endif
#ifdef __GXX_EXPERIMENTAL_CXX0X__
#  define BOOST_HAS_CPP_0X
#endif

//
// We may be in the middle of parsing boost/config.hpp
// when this header is included, so don't rely on config
// stuff in the rest of this header...
//
// Find our actual std lib:
//
#if defined(BOOST_HAS_INCLUDE_NEXT) && !defined(BOOST_TR1_DISABLE_INCLUDE_NEXT)
//
// We don't take this branch if BOOST_TR1_DISABLE_INCLUDE_NEXT
// is defined as we may be installed in 
// /usr/include, in which case #include_next won't work as our
// include path will occur AFTER the regular std lib one :-(
//
#  ifndef BOOST_TR1_NO_RECURSION
#     define BOOST_TR1_NO_RECURSION
#     define BOOST_TR1_NO_CONFIG_ALL_RECURSION
#  endif
#  include_next <utility>
#  if (__GNUC__ < 3)
#     include_next <algorithm>
#     include_next <iterator>
#  endif
#  ifdef BOOST_TR1_NO_CONFIG_ALL_RECURSION
#     undef BOOST_TR1_NO_CONFIG_ALL_RECURSION
#     undef BOOST_TR1_NO_RECURSION
#  endif
#else
#  include BOOST_TR1_STD_HEADER(utility)
#endif

#include <boost/tr1/detail/config.hpp>

#endif


