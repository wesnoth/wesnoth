//  (C) Copyright Noel Belcourt 2007.
//  Use, modification and distribution are subject to the 
//  Boost Software License, Version 1.0. (See accompanying file 
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for most recent version.

//  PGI C++ compiler setup:

#define BOOST_COMPILER_VERSION __PGIC__##__PGIC_MINOR__
#define BOOST_COMPILER "PGI compiler version " BOOST_STRINGIZE(_COMPILER_VERSION)

//
// Threading support:
// Turn this on unconditionally here, it will get turned off again later
// if no threading API is detected.
//

#if (__PGIC__ >= 7)

#define BOOST_FUNCTION_SCOPE_USING_DECLARATION_BREAKS_ADL 
#define BOOST_NO_TWO_PHASE_NAME_LOOKUP
#define BOOST_NO_SWPRINTF

#else

#  error "Pgi compiler not configured - please reconfigure"

#endif
//
// version check:
// probably nothing to do here?

