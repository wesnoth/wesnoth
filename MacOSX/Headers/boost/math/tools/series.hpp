//  (C) Copyright John Maddock 2005-2006.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_MATH_TOOLS_SERIES_INCLUDED
#define BOOST_MATH_TOOLS_SERIES_INCLUDED

#ifdef _MSC_VER
#pragma once
#endif

#include <boost/config/no_tr1/cmath.hpp>
#include <boost/cstdint.hpp>
#include <boost/math/tools/config.hpp>

namespace boost{ namespace math{ namespace tools{

//
// Simple series summation come first:
//
template <class Functor>
typename Functor::result_type sum_series(Functor& func, int bits)
{
   BOOST_MATH_STD_USING

   typedef typename Functor::result_type result_type;

   result_type factor = pow(result_type(2), bits);
   result_type result = func();
   result_type next_term;
   do{
      next_term = func();
      result += next_term;
   }
   while(fabs(result) < fabs(factor * next_term));
   return result;
}

template <class Functor>
typename Functor::result_type sum_series(Functor& func, int bits, boost::uintmax_t& max_terms)
{
   BOOST_MATH_STD_USING

   typedef typename Functor::result_type result_type;

   boost::uintmax_t counter = max_terms;

   result_type factor = ldexp(result_type(1), bits);
   result_type result = func();
   result_type next_term;
   do{
      next_term = func();
      result += next_term;
   }
   while((fabs(result) < fabs(factor * next_term)) && --counter);

   // set max_terms to the actual number of terms of the series evaluated:
   max_terms = max_terms - counter;

   return result;
}

template <class Functor, class U>
typename Functor::result_type sum_series(Functor& func, int bits, U init_value)
{
   BOOST_MATH_STD_USING

   typedef typename Functor::result_type result_type;

   result_type factor = ldexp(result_type(1), bits);
   result_type result = static_cast<result_type>(init_value);
   result_type next_term;
   do{
      next_term = func();
      result += next_term;
   }
   while(fabs(result) < fabs(factor * next_term));

   return result;
}

template <class Functor, class U>
typename Functor::result_type sum_series(Functor& func, int bits, boost::uintmax_t& max_terms, U init_value)
{
   BOOST_MATH_STD_USING

   typedef typename Functor::result_type result_type;

   boost::uintmax_t counter = max_terms;

   result_type factor = ldexp(result_type(1), bits);
   result_type result = init_value;
   result_type next_term;
   do{
      next_term = func();
      result += next_term;
   }
   while((fabs(result) < fabs(factor * next_term)) && --counter);

   // set max_terms to the actual number of terms of the series evaluated:
   max_terms = max_terms - counter;

   return result;
}

//
// Algorithm kahan_sum_series invokes Functor func until the N'th
// term is too small to have any effect on the total, the terms
// are added using the Kahan summation method.
//
// CAUTION: Optimizing compilers combined with extended-precision
// machine registers conspire to render this algorithm partly broken:
// double rounding of intermediate terms (first to a long double machine
// register, and then to a double result) cause the rounding error computed
// by the algorithm to be off by up to 1ulp.  However this occurs rarely, and
// in any case the result is still much better than a naive summation.
//
template <class Functor>
typename Functor::result_type kahan_sum_series(Functor& func, int bits)
{
   BOOST_MATH_STD_USING

   typedef typename Functor::result_type result_type;

   result_type factor = pow(result_type(2), bits);
   result_type result = func();
   result_type next_term, y, t;
   result_type carry = 0;
   do{
      next_term = func();
      y = next_term - carry;
      t = result + y;
      carry = t - result;
      carry -= y;
      result = t;
   }
   while(fabs(result) < fabs(factor * next_term));
   return result;
}

template <class Functor>
typename Functor::result_type kahan_sum_series(Functor& func, int bits, boost::uintmax_t& max_terms)
{
   BOOST_MATH_STD_USING

   typedef typename Functor::result_type result_type;

   boost::uintmax_t counter = max_terms;

   result_type factor = ldexp(result_type(1), bits);
   result_type result = func();
   result_type next_term, y, t;
   result_type carry = 0;
   do{
      next_term = func();
      y = next_term - carry;
      t = result + y;
      carry = t - result;
      carry -= y;
      result = t;
   }
   while((fabs(result) < fabs(factor * next_term)) && --counter);

   // set max_terms to the actual number of terms of the series evaluated:
   max_terms = max_terms - counter;

   return result;
}

} // namespace tools
} // namespace math
} // namespace boost

#endif // BOOST_MATH_TOOLS_SERIES_INCLUDED

