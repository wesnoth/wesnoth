// Boost.Units - A C++ library for zero-overhead dimensional analysis and 
// unit/quantity manipulation and conversion
//
// Copyright (C) 2003-2008 Matthias Christian Schabel
// Copyright (C) 2008 Steven Watanabe
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_UNITS_CMATH_GNU_IMPL_HPP 
#define BOOST_UNITS_CMATH_GNU_IMPL_HPP

#if (__GNUC__ && __cplusplus && __GNUC__ >= 3)

#include <boost/config/no_tr1/cmath.hpp>

namespace boost {

namespace units { 

namespace detail {

template<class Y> 
inline bool isgreater(const Y& v1,const Y& v2)
{
    return __builtin_isgreater(v1,v2);
}

template<class Y> 
inline bool isgreaterequal(const Y& v1,const Y& v2)
{
    return __builtin_isgreaterequal(v1,v2);
}

template<class Y> 
inline bool isless(const Y& v1,const Y& v2)
{
    return __builtin_isless(v1,v2);
}

template<class Y> 
inline bool islessequal(const Y& v1,const Y& v2)
{
    return __builtin_islessequal(v1,v2);
}

template<class Y> 
inline bool islessgreater(const Y& v1,const Y& v2)
{
    return __builtin_islessgreater(v1,v2);
}

template<class Y> 
inline bool isunordered(const Y& v1,const Y& v2)
{
    return __builtin_isunordered(v1,v2);
}

template<class Y>
inline Y fdim(const Y& v1,const Y& v2)
{
    return __builtin_fdim(v1,v2);
}

template<class Y>
inline Y fma(const Y& v1,const Y& v2,const Y& v3)
{
    return __builtin_fma(v1,v2,v3);
}

template<class Y>
inline Y fmax(const Y& v1,const Y& v2)
{
    return __builtin_fmax(v1,v2);
}

template<class Y>
inline Y fmin(const Y& v1,const Y& v2)
{
    return __builtin_fmin(v1,v2);
}

//template<class Y>
//inline long long llrint(const Y& val)
//{
//    return __builtin_llrint(val);
//}
//
//template<class Y>
//inline long long llround(const Y& val)
//{
//    return __builtin_llround(val);
//}

template<class Y>
inline Y nearbyint(const Y& val)
{
    return __builtin_nearbyint(val);
}

template<class Y>
inline Y nextafter(const Y& v1,const Y& v2)
{
    return __builtin_nextafter(v1,v2);
}

template<class Y>
inline Y nexttoward(const Y& v1,const Y& v2)
{
    return __builtin_nexttoward(v1,v2);
}

template<class Y>
inline Y rint(const Y& val)
{
    return __builtin_rint(val);
}

template<class Y>
inline bool signbit(const Y& val)
{
    return __builtin_signbit(val);
}

template<class Y>
inline Y trunc(const Y& val)
{
    return __builtin_trunc(val);
}

} // namespace detail

} // namespace units

} // namespace boost

#endif // __GNUC__

#endif // BOOST_UNITS_CMATH_GNU_IMPL_HPP
