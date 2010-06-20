// Copyright 2005 Daniel Wallin.
// Copyright 2005 Joel de Guzman.
//
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// Modeled after range_ex, Copyright 2004 Eric Niebler
///////////////////////////////////////////////////////////////////////////////
//
// is_std_hash_map.hpp
//
/////////////////////////////////////////////////////////////////////////////

#if defined(_MSC_VER) && _MSC_VER >= 1000
# pragma once
#endif

#ifndef IS_STD_HASH_MAP_EN_16_12_2004
#define IS_STD_HASH_MAP_EN_16_12_2004

#include <boost/mpl/bool.hpp>
#include "./std_hash_map_fwd.hpp"

namespace boost
{
    template<class T>
    struct is_std_hash_map
        : boost::mpl::false_
    {};

    template<
        class Kty
      , class Tr
      , class Alloc
    >
    struct is_std_hash_map< ::stdext::hash_map<Kty,Tr,Alloc> >
        : boost::mpl::true_
    {};

    template<class T>
    struct is_std_hash_multimap
        : boost::mpl::false_
    {};

    template<
        class Kty
      , class Tr
      , class Alloc
    >
    struct is_std_hash_multimap< ::stdext::hash_multimap<Kty,Tr,Alloc> >
        : boost::mpl::true_
    {};
}

#endif
