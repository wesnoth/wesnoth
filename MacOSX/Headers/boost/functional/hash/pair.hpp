
// Copyright 2005-2008 Daniel James.
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  Based on Peter Dimov's proposal
//  http://www.open-std.org/JTC1/SC22/WG21/docs/papers/2005/n1756.pdf
//  issue 6.18. 

#if !defined(BOOST_FUNCTIONAL_HASH_PAIR_HPP)
#define BOOST_FUNCTIONAL_HASH_PAIR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#if defined(__EDG__)
#elif defined(_MSC_VER) || defined(__BORLANDC__) || defined(__DMC__)
#pragma message("Warning: boost/functional/hash/pair.hpp is deprecated, use boost/functional/hash.hpp instead.")
#elif defined(__GNUC__) || defined(__HP_aCC) || \
    defined(__SUNPRO_CC) || defined(__IBMCPP__)
#warning "boost/functional/hash/pair.hpp is deprecated, use boost/functional/hash.hpp instead."
#endif

#include <boost/functional/hash.hpp>

#endif
