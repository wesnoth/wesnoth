//  Copyright (c) 2006-7 John Maddock
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_MATH_TOOLS_CONFIG_HPP
#define BOOST_MATH_TOOLS_CONFIG_HPP

#ifdef _MSC_VER
#pragma once
#endif

#include <boost/cstdint.hpp> // for boost::uintmax_t
#include <boost/config.hpp>
#include <boost/detail/workaround.hpp>
#include <algorithm>  // for min and max
#include <boost/config/no_tr1/cmath.hpp>
#include <climits>
#if (defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__))
#  include <math.h>
#endif

#include <boost/math/tools/user.hpp>
#include <boost/math/special_functions/detail/round_fwd.hpp>

#if defined(__CYGWIN__) || defined(__FreeBSD__) || defined(__NetBSD__) \
   || defined(__hppa) || defined(__NO_LONG_DOUBLE_MATH)
#  define BOOST_MATH_NO_LONG_DOUBLE_MATH_FUNCTIONS
#endif
#if BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x582))
//
// Borland post 5.8.2 uses Dinkumware's std C lib which
// doesn't have true long double precision.  Earlier
// versions are problematic too:
//
#  define BOOST_MATH_NO_REAL_CONCEPT_TESTS
#  define BOOST_MATH_NO_LONG_DOUBLE_MATH_FUNCTIONS
#  define BOOST_MATH_CONTROL_FP _control87(MCW_EM,MCW_EM)
#  include <float.h>
#endif
#if (defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__)) && ((LDBL_MANT_DIG == 106) || (__LDBL_MANT_DIG__ == 106))
//
// Darwin's rather strange "double double" is rather hard to
// support, it should be possible given enough effort though...
//
#  define BOOST_MATH_NO_LONG_DOUBLE_MATH_FUNCTIONS
#endif
#if defined(unix) && defined(__INTEL_COMPILER) && (__INTEL_COMPILER <= 1000)
//
// Intel compiler prior to version 10 has sporadic problems
// calling the long double overloads of the std lib math functions:
// calling ::powl is OK, but std::pow(long double, long double) 
// may segfault depending upon the value of the arguments passed 
// and the specific Linux distribution.
//
// We'll be conservative and disable long double support for this compiler.
//
// Comment out this #define and try building the tests to determine whether
// your Intel compiler version has this issue or not.
//
#  define BOOST_MATH_NO_LONG_DOUBLE_MATH_FUNCTIONS
#endif

#if defined(BOOST_MSVC) && !defined(_WIN32_WCE)
   // Better safe than sorry, our tests don't support hardware exceptions:
#  define BOOST_MATH_CONTROL_FP _control87(MCW_EM,MCW_EM)
#endif

#ifdef __IBMCPP__
#  define BOOST_MATH_NO_DEDUCED_FUNCTION_POINTERS
#endif

#if defined(BOOST_NO_EXPLICIT_FUNCTION_TEMPLATE_ARGUMENTS) || BOOST_WORKAROUND(__SUNPRO_CC, <= 0x590)

#  include "boost/type.hpp"
#  include "boost/non_type.hpp"

#  define BOOST_MATH_EXPLICIT_TEMPLATE_TYPE(t)         boost::type<t>* = 0
#  define BOOST_MATH_EXPLICIT_TEMPLATE_TYPE_SPEC(t)    boost::type<t>*
#  define BOOST_MATH_EXPLICIT_TEMPLATE_NON_TYPE(t, v)  boost::non_type<t, v>* = 0
#  define BOOST_MATH_EXPLICIT_TEMPLATE_NON_TYPE_SPEC(t, v)  boost::non_type<t, v>*

#  define BOOST_MATH_APPEND_EXPLICIT_TEMPLATE_TYPE(t)         \
             , BOOST_MATH_EXPLICIT_TEMPLATE_TYPE(t)
#  define BOOST_MATH_APPEND_EXPLICIT_TEMPLATE_TYPE_SPEC(t)    \
             , BOOST_MATH_EXPLICIT_TEMPLATE_TYPE_SPEC(t)
#  define BOOST_MATH_APPEND_EXPLICIT_TEMPLATE_NON_TYPE(t, v)  \
             , BOOST_MATH_EXPLICIT_TEMPLATE_NON_TYPE(t, v)
#  define BOOST_MATH_APPEND_EXPLICIT_TEMPLATE_NON_TYPE_SPEC(t, v)  \
             , BOOST_MATH_EXPLICIT_TEMPLATE_NON_TYPE_SPEC(t, v)

#else

// no workaround needed: expand to nothing

#  define BOOST_MATH_EXPLICIT_TEMPLATE_TYPE(t)
#  define BOOST_MATH_EXPLICIT_TEMPLATE_TYPE_SPEC(t)
#  define BOOST_MATH_EXPLICIT_TEMPLATE_NON_TYPE(t, v)
#  define BOOST_MATH_EXPLICIT_TEMPLATE_NON_TYPE_SPEC(t, v)

#  define BOOST_MATH_APPEND_EXPLICIT_TEMPLATE_TYPE(t)
#  define BOOST_MATH_APPEND_EXPLICIT_TEMPLATE_TYPE_SPEC(t)
#  define BOOST_MATH_APPEND_EXPLICIT_TEMPLATE_NON_TYPE(t, v)
#  define BOOST_MATH_APPEND_EXPLICIT_TEMPLATE_NON_TYPE_SPEC(t, v)


#endif // defined BOOST_NO_EXPLICIT_FUNCTION_TEMPLATE_ARGUMENTS

#if BOOST_WORKAROUND(__SUNPRO_CC, <= 0x590) || defined(__hppa)
// Sun's compiler emits a hard error if a constant underflows,
// as does aCC on PA-RISC:
#  define BOOST_MATH_SMALL_CONSTANT(x) 0
#else
#  define BOOST_MATH_SMALL_CONSTANT(x) x
#endif


#if BOOST_WORKAROUND(BOOST_MSVC, < 1400)
//
// Define if constants too large for a float cause "bad"
// values to be stored in the data, rather than infinity
// or a suitably large value.
//
#  define BOOST_MATH_BUGGY_LARGE_FLOAT_CONSTANTS
#endif
//
// Tune performance options for specific compilers:
//
#ifdef BOOST_MSVC
#  define BOOST_MATH_POLY_METHOD 3
#elif defined(BOOST_INTEL)
#  define BOOST_MATH_POLY_METHOD 2
#  define BOOST_MATH_RATIONAL_METHOD 2
#elif defined(__GNUC__)
#  define BOOST_MATH_POLY_METHOD 3
#  define BOOST_MATH_RATIONAL_METHOD 3
#  define BOOST_MATH_INT_TABLE_TYPE(RT, IT) RT
#endif

//
// The maximum order of polynomial that will be evaluated 
// via an unrolled specialisation:
//
#ifndef BOOST_MATH_MAX_POLY_ORDER
#  define BOOST_MATH_MAX_POLY_ORDER 17
#endif 
//
// Set the method used to evaluate polynomials and rationals:
//
#ifndef BOOST_MATH_POLY_METHOD
#  define BOOST_MATH_POLY_METHOD 1
#endif 
#ifndef BOOST_MATH_RATIONAL_METHOD
#  define BOOST_MATH_RATIONAL_METHOD 0
#endif 
//
// decide whether to store constants as integers or reals:
//
#ifndef BOOST_MATH_INT_TABLE_TYPE
#  define BOOST_MATH_INT_TABLE_TYPE(RT, IT) IT
#endif
//
// Helper macro for controlling the FP behaviour:
//
#ifndef BOOST_MATH_CONTROL_FP
#  define BOOST_MATH_CONTROL_FP
#endif
//
// Helper macro for using statements:
//
#define BOOST_MATH_STD_USING \
   using std::abs;\
   using std::acos;\
   using std::cos;\
   using std::fmod;\
   using std::modf;\
   using std::tan;\
   using std::asin;\
   using std::cosh;\
   using std::frexp;\
   using std::pow;\
   using std::tanh;\
   using std::atan;\
   using std::exp;\
   using std::ldexp;\
   using std::sin;\
   using std::atan2;\
   using std::fabs;\
   using std::log;\
   using std::sinh;\
   using std::ceil;\
   using std::floor;\
   using std::log10;\
   using std::sqrt;\
   using boost::math::round;\
   using boost::math::iround;\
   using boost::math::lround;\
   using boost::math::trunc;\
   using boost::math::itrunc;\
   using boost::math::ltrunc;\
   using boost::math::modf;


namespace boost{ namespace math{
namespace tools
{

template <class T>
inline T max BOOST_PREVENT_MACRO_SUBSTITUTION(T a, T b, T c)
{
   return (std::max)((std::max)(a, b), c);
}

template <class T>
inline T max BOOST_PREVENT_MACRO_SUBSTITUTION(T a, T b, T c, T d)
{
   return (std::max)((std::max)(a, b), (std::max)(c, d));
}
} // namespace tools
}} // namespace boost namespace math

#ifdef __linux__

   #include <fenv.h>

   namespace boost{ namespace math{
   namespace detail
   {
   struct fpu_guard
   {
      fpu_guard()
      {
         fegetexceptflag(&m_flags, FE_ALL_EXCEPT);
         feclearexcept(FE_ALL_EXCEPT);
      }
      ~fpu_guard()
      {
         fesetexceptflag(&m_flags, FE_ALL_EXCEPT);
      }
   private:
      fexcept_t m_flags;
   };

   } // namespace detail
   }} // namespaces

   #define BOOST_FPU_EXCEPTION_GUARD boost::math::detail::fpu_guard local_guard_object;
#else // All other platforms.
  #define BOOST_FPU_EXCEPTION_GUARD
#endif

#ifdef BOOST_MATH_INSTRUMENT
#define BOOST_MATH_INSTRUMENT_CODE(x) \
   std::cout << std::setprecision(35) << __FILE__ << ":" << __LINE__ << " " << x << std::endl;
#define BOOST_MATH_INSTRUMENT_VARIABLE(name) BOOST_MATH_INSTRUMENT_CODE(BOOST_STRINGIZE(name) << " = " << name)
#else
#define BOOST_MATH_INSTRUMENT_CODE(x)
#define BOOST_MATH_INSTRUMENT_VARIABLE(name)
#endif

#endif // BOOST_MATH_TOOLS_CONFIG_HPP





