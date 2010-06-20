/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying 
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#ifndef PHOENIX_CORE_LIMITS_HPP
#define PHOENIX_CORE_LIMITS_HPP

#if !defined(PHOENIX_LIMIT)
# define PHOENIX_LIMIT 10
#endif

#if !defined(PHOENIX_ACTOR_LIMIT)
# define PHOENIX_ACTOR_LIMIT PHOENIX_LIMIT
#elif (PHOENIX_ACTOR_LIMIT > PHOENIX_LIMIT)
# error "PHOENIX_ACTOR_LIMIT is set too high"
#endif

#if !defined(FUSION_MAX_TUPLE_SIZE)
# define FUSION_MAX_TUPLE_SIZE PHOENIX_LIMIT
#elif (FUSION_MAX_TUPLE_SIZE < PHOENIX_LIMIT)
# error "FUSION_MAX_TUPLE_SIZE is set too low"
#endif

// this include will bring in mpl::vectorN and 
// fusion::tupleN where N is PHOENIX_ACTOR_LIMIT
#include <boost/fusion/include/vector.hpp>

// for some reason, this must be included now to make
// detail/type_deduction.hpp compile. $$$ TODO: Investigate further $$$
#include <boost/mpl/vector/vector20.hpp>

#endif
