
// Copyright 2008 Daniel James.
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_UNORDERED_DETAIL_CONFIG_HEADER)
#define BOOST_UNORDERED_DETAIL_CONFIG_HEADER

#include <boost/config.hpp>

#if defined(BOOST_NO_SFINAE)
#  define BOOST_UNORDERED_NO_HAS_MOVE_ASSIGN
#elif defined(__GNUC__) && \
    (__GNUC__ < 3 || __GNUC__ == 3 && __GNUC_MINOR__ <= 3)
#  define BOOST_UNORDERED_NO_HAS_MOVE_ASSIGN
#elif BOOST_WORKAROUND(BOOST_INTEL, < 900) || \
    BOOST_WORKAROUND(__EDG_VERSION__, < 304) || \
    BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x0593))
#  define BOOST_UNORDERED_NO_HAS_MOVE_ASSIGN
#endif

#endif
