/*=============================================================================
    Copyright (c) 2006-2007 Tobias Schwinger
  
    Use modification and distribution are subject to the Boost Software 
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt).
==============================================================================*/

#if !defined(BOOST_PP_IS_ITERATING)
#   error "This file has to be included by a preprocessor loop construct!"
#elif BOOST_PP_ITERATION_DEPTH() == 1

#   if !defined(BOOST_FUSION_FUNCTIONAL_ADAPTER_DETAIL_POW2_EXPLODE_HPP_INCLUDED)
#       include <boost/preprocessor/config/limits.hpp>
#       include <boost/preprocessor/slot/slot.hpp>
#       include <boost/preprocessor/arithmetic/dec.hpp>
#       define BOOST_FUSION_FUNCTIONAL_ADAPTER_DETAIL_POW2_EXPLODE_HPP_INCLUDED
#   endif

#   define  BOOST_PP_VALUE 0
#   include BOOST_PP_ASSIGN_SLOT(1)

#   define  BOOST_PP_FILENAME_2 \
        <boost/fusion/functional/adapter/detail/pow2_explode.hpp>
#   define  BOOST_PP_VALUE (1 << N) >> 4 
#   if BOOST_PP_VALUE > BOOST_PP_LIMIT_ITERATION
#       error "Preprocessor limit exceeded."
#   endif

#   include BOOST_PP_ASSIGN_SLOT(2)
#   define  BOOST_PP_ITERATION_LIMITS (0,BOOST_PP_DEC(BOOST_PP_SLOT_2()))
#   include BOOST_PP_ITERATE()

#elif BOOST_PP_ITERATION_DEPTH() == 2

#   if BOOST_PP_SLOT_1() < 1 << N
#   include BOOST_PP_INDIRECT_SELF
#   define  BOOST_PP_VALUE BOOST_PP_SLOT_1() + 1
#   include BOOST_PP_ASSIGN_SLOT(1)
#   if BOOST_PP_SLOT_1() < 1 << N
#   include BOOST_PP_INDIRECT_SELF
#   define  BOOST_PP_VALUE BOOST_PP_SLOT_1() + 1
#   include BOOST_PP_ASSIGN_SLOT(1)
#   if BOOST_PP_SLOT_1() < 1 << N
#   include BOOST_PP_INDIRECT_SELF
#   define  BOOST_PP_VALUE BOOST_PP_SLOT_1() + 1
#   include BOOST_PP_ASSIGN_SLOT(1)
#   if BOOST_PP_SLOT_1() < 1 << N
#   include BOOST_PP_INDIRECT_SELF
#   define  BOOST_PP_VALUE BOOST_PP_SLOT_1() + 1
#   include BOOST_PP_ASSIGN_SLOT(1)
#   if BOOST_PP_SLOT_1() < 1 << N
#   include BOOST_PP_INDIRECT_SELF
#   define  BOOST_PP_VALUE BOOST_PP_SLOT_1() + 1
#   include BOOST_PP_ASSIGN_SLOT(1)
#   if BOOST_PP_SLOT_1() < 1 << N
#   include BOOST_PP_INDIRECT_SELF
#   define  BOOST_PP_VALUE BOOST_PP_SLOT_1() + 1
#   include BOOST_PP_ASSIGN_SLOT(1)
#   if BOOST_PP_SLOT_1() < 1 << N
#   include BOOST_PP_INDIRECT_SELF
#   define  BOOST_PP_VALUE BOOST_PP_SLOT_1() + 1
#   include BOOST_PP_ASSIGN_SLOT(1)
#   if BOOST_PP_SLOT_1() < 1 << N
#   include BOOST_PP_INDIRECT_SELF
#   define  BOOST_PP_VALUE BOOST_PP_SLOT_1() + 1
#   include BOOST_PP_ASSIGN_SLOT(1)
#   if BOOST_PP_SLOT_1() < 1 << N
#   include BOOST_PP_INDIRECT_SELF
#   define  BOOST_PP_VALUE BOOST_PP_SLOT_1() + 1
#   include BOOST_PP_ASSIGN_SLOT(1)
#   if BOOST_PP_SLOT_1() < 1 << N
#   include BOOST_PP_INDIRECT_SELF
#   define  BOOST_PP_VALUE BOOST_PP_SLOT_1() + 1
#   include BOOST_PP_ASSIGN_SLOT(1)
#   if BOOST_PP_SLOT_1() < 1 << N
#   include BOOST_PP_INDIRECT_SELF
#   define  BOOST_PP_VALUE BOOST_PP_SLOT_1() + 1
#   include BOOST_PP_ASSIGN_SLOT(1)
#   if BOOST_PP_SLOT_1() < 1 << N
#   include BOOST_PP_INDIRECT_SELF
#   define  BOOST_PP_VALUE BOOST_PP_SLOT_1() + 1
#   include BOOST_PP_ASSIGN_SLOT(1)
#   if BOOST_PP_SLOT_1() < 1 << N
#   include BOOST_PP_INDIRECT_SELF
#   define  BOOST_PP_VALUE BOOST_PP_SLOT_1() + 1
#   include BOOST_PP_ASSIGN_SLOT(1)
#   if BOOST_PP_SLOT_1() < 1 << N
#   include BOOST_PP_INDIRECT_SELF
#   define  BOOST_PP_VALUE BOOST_PP_SLOT_1() + 1
#   include BOOST_PP_ASSIGN_SLOT(1)
#   if BOOST_PP_SLOT_1() < 1 << N
#   include BOOST_PP_INDIRECT_SELF
#   define  BOOST_PP_VALUE BOOST_PP_SLOT_1() + 1
#   include BOOST_PP_ASSIGN_SLOT(1)
#   if BOOST_PP_SLOT_1() < 1 << N
#   include BOOST_PP_INDIRECT_SELF
#   define  BOOST_PP_VALUE BOOST_PP_SLOT_1() + 1
#   include BOOST_PP_ASSIGN_SLOT(1)
#   endif
#   endif
#   endif
#   endif
#   endif
#   endif
#   endif
#   endif
#   endif
#   endif
#   endif
#   endif
#   endif
#   endif
#   endif
#   endif

#endif

