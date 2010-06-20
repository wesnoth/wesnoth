// Boost.Units - A C++ library for zero-overhead dimensional analysis and 
// unit/quantity manipulation and conversion
//
// Copyright (C) 2003-2008 Matthias Christian Schabel
// Copyright (C) 2008 Steven Watanabe
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_UNITS_CMATH_MSVC_IMPL_HPP 
#define BOOST_UNITS_CMATH_MSVC_IMPL_HPP

#include <boost/config.hpp>

#if defined(BOOST_MSVC) || (defined(__COMO__) && defined(_MSC_VER))

#include <cfloat>
#include <boost/config/no_tr1/cmath.hpp>

#include <boost/static_warning.hpp>
#include <boost/math/special_functions/round.hpp>
#include <boost/math/special_functions/fpclassify.hpp>

namespace boost {

namespace units { 

namespace detail {

template<class Y> 
inline bool isgreater(const Y& v1,const Y& v2)
{
    if(_fpclass(v1) == _FPCLASS_SNAN || _fpclass(v2) == _FPCLASS_SNAN) return false;
    else return v1 > v2;
}

template<class Y> 
inline bool isgreaterequal(const Y& v1,const Y& v2)
{
    if(_fpclass(v1) == _FPCLASS_SNAN || _fpclass(v2) == _FPCLASS_SNAN) return false;
    else return v1 >= v2;
}

template<class Y> 
inline bool isless(const Y& v1,const Y& v2)
{
    if(::_fpclass(v1) == _FPCLASS_SNAN || ::_fpclass(v2) == _FPCLASS_SNAN) return false;
    else return v1 < v2;
}

template<class Y> 
inline bool islessequal(const Y& v1,const Y& v2)
{
    if(::_fpclass(v1) == _FPCLASS_SNAN || ::_fpclass(v2) == _FPCLASS_SNAN) return false;
    else return v1 <= v2;
}

template<class Y> 
inline bool islessgreater(const Y& v1,const Y& v2)
{
    if(::_fpclass(v1) == _FPCLASS_SNAN || ::_fpclass(v2) == _FPCLASS_SNAN) return false;
    else return v1 < v2 || v1 > v2;
}

template<class Y> 
inline bool isunordered(const Y& v1,const Y& v2)
{
    using boost::math::isnan;
    return isnan(v1) || isnan(v2);
}

template<class Y>
inline Y fdim(const Y& v1,const Y& v2)
{
    using boost::math::isnan;
    if(isnan(v1)) return v1;
    else if(isnan(v2)) return v2;
    else if(v1 > v2) return(v1 - v2);
    else return(Y(0));
}

template<class T>
struct fma_issue_warning {
    enum { value = false };
};

template<class Y>
inline Y fma(const Y& v1,const Y& v2,const Y& v3)
{
    //this implementation does *not* meet the
    //requirement of infinite intermediate precision
    BOOST_STATIC_WARNING((fma_issue_warning<Y>::value));

    return v1 * v2 + v3;
}

template<class Y>
inline Y fmax(const Y& v1,const Y& v2)
{
    return __max(v1,v2);
}

template<class Y>
inline Y fmin(const Y& v1,const Y& v2)
{
    return __min(v1,v2);
}

//template<class Y>
//inline long long llrint(const Y& val)
//{
//    return static_cast<long long>(rint(val));
//}
//
//template<class Y>
//inline long long llround(const Y& val)
//{
//    return static_cast<long long>(round(val));
//}

template<class Y>
inline Y nearbyint(const Y& val)
{
    //this is not really correct.
    //the result should be according to the
    //current rounding mode.
    using boost::math::round;
    return round(val);
}

template<class Y>
inline Y nextafter(const Y& v1,const Y& v2)
{
    return ::_nextafter(v1,v2);
}

template<class Y>
inline Y nexttoward(const Y& v1,const Y& v2)
{
    //the only diference between nextafter and
    //nexttoward is the types of the operands
    return ::_nextafter(v1,v2);
}

template<class Y>
inline Y rint(const Y& val)
{
    //I don't feel like trying to figure out
    //how to raise a floating pointer exception
    return nearbyint(val);
}

template<class Y>
inline Y trunc(const Y& val)
{
    if(val > 0) return std::floor(val);
    else if(val < 0) return std::ceil(val);
    else return val;
}

} // namespace detail

} // namespace units

} // namespace boost

#endif // __MSVC__

#endif // BOOST_UNITS_CMATH_MSVC_IMPL_HPP
