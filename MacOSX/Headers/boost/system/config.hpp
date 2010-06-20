//  boost/system/config.hpp  -------------------------------------------------//

//  Copyright Beman Dawes 2003, 2006

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org/libs/system for documentation.

#ifndef BOOST_SYSTEM_CONFIG_HPP                  
#define BOOST_SYSTEM_CONFIG_HPP

#include <boost/config.hpp>

//  BOOST_POSIX_API or BOOST_WINDOWS_API specify which API to use.
//  If not specified, a sensible default will be applied.

# if defined( BOOST_WINDOWS_API ) && defined( BOOST_POSIX_API )
#   error both BOOST_WINDOWS_API and BOOST_POSIX_API are defined
# elif !defined( BOOST_WINDOWS_API ) && !defined( BOOST_POSIX_API )
#   if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__)
#     define BOOST_WINDOWS_API
#   else
#     define BOOST_POSIX_API 
#   endif
# endif

//  enable dynamic linking on Windows  ---------------------------------------//

//#  if (defined(BOOST_ALL_DYN_LINK) || defined(BOOST_SYSTEM_DYN_LINK)) && defined(__BORLANDC__) && defined(__WIN32__)
//#    error Dynamic linking Boost.System does not work for Borland; use static linking instead
//#  endif

#ifdef BOOST_HAS_DECLSPEC // defined in config system
// we need to import/export our code only if the user has specifically
// asked for it by defining either BOOST_ALL_DYN_LINK if they want all boost
// libraries to be dynamically linked, or BOOST_SYSTEM_DYN_LINK
// if they want just this one to be dynamically liked:
#if defined(BOOST_ALL_DYN_LINK) || defined(BOOST_SYSTEM_DYN_LINK)
// export if this is our own source, otherwise import:
#ifdef BOOST_SYSTEM_SOURCE
# define BOOST_SYSTEM_DECL __declspec(dllexport)
#else
# define BOOST_SYSTEM_DECL __declspec(dllimport)
#endif  // BOOST_SYSTEM_SOURCE
#endif  // DYN_LINK
#endif  // BOOST_HAS_DECLSPEC
//
// if BOOST_SYSTEM_DECL isn't defined yet define it now:
#ifndef BOOST_SYSTEM_DECL
#define BOOST_SYSTEM_DECL
#endif

//  enable automatic library variant selection  ------------------------------// 

#if !defined(BOOST_SYSTEM_SOURCE) && !defined(BOOST_ALL_NO_LIB) && !defined(BOOST_SYSTEM_NO_LIB)
//
// Set the name of our library, this will get undef'ed by auto_link.hpp
// once it's done with it:
//
#define BOOST_LIB_NAME boost_system
//
// If we're importing code from a dll, then tell auto_link.hpp about it:
//
#if defined(BOOST_ALL_DYN_LINK) || defined(BOOST_SYSTEM_DYN_LINK)
#  define BOOST_DYN_LINK
#endif
//
// And include the header that does the work:
//
#include <boost/config/auto_link.hpp>
#endif  // auto-linking disabled

#endif // BOOST_SYSTEM_CONFIG_HPP

