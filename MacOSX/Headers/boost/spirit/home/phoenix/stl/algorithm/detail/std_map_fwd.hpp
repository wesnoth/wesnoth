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
// std_map_fwd.hpp
//
/////////////////////////////////////////////////////////////////////////////

#if defined(_MSC_VER) && _MSC_VER >= 1000
# pragma once
#endif

#ifndef STD_MAP_FWD_EN_16_12_2004
#define STD_MAP_FWD_EN_16_12_2004

namespace std
{
    template<
        class Kty
      , class Ty
      , class Pr
      , class Alloc
    >
    class map;

    template<
        class Kty
      , class Ty
      , class Pr
      , class Alloc
    >
    class multimap;
}

#endif
