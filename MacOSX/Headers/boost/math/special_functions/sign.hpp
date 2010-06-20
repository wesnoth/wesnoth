//  (C) Copyright John Maddock 2006.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_MATH_TOOLS_SIGN_HPP
#define BOOST_MATH_TOOLS_SIGN_HPP

#ifdef _MSC_VER
#pragma once
#endif

#include <boost/math/tools/config.hpp>
#include <boost/math/special_functions/math_fwd.hpp>

namespace boost{ namespace math{ 

template <class T>
inline int sign BOOST_NO_MACRO_EXPAND(const T& z)
{
   return (z == 0) ? 0 : (z < 0) ? -1 : 1;
}

template <class T>
inline int signbit BOOST_NO_MACRO_EXPAND(const T& z)
{
   return (z < 0) ? 1 : 0;
}

template <class T>
inline T copysign BOOST_NO_MACRO_EXPAND(const T& x, const T& y)
{
   BOOST_MATH_STD_USING
   return fabs(x) * boost::math::sign(y);
}

} // namespace math
} // namespace boost


#endif // BOOST_MATH_TOOLS_SIGN_HPP


