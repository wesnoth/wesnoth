// mcs::units - A C++ library for zero-overhead dimensional analysis and 
// unit/quantity manipulation and conversion
//
// Copyright (C) 2003-2008 Matthias Christian Schabel
// Copyright (C) 2008 Steven Watanabe
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_UNITS_CMATH_MSVC_IMPL_BOOST_1_35_HPP 
#define BOOST_UNITS_CMATH_MSVC_IMPL_BOOST_1_35_HPP

#include <boost/config.hpp>

#if defined(BOOST_MSVC) || (defined(__COMO__) && defined(_MSC_VER))

#include <cfloat>
#include <boost/config/no_tr1/cmath.hpp>

#include <boost/static_warning.hpp>
#include <boost/mpl/bool.hpp>

namespace boost {

namespace units { 

namespace detail {

template<class Y> 
inline bool isfinite(const Y& val)
{
    return _finite(val) != 0;
}

template<class Y> 
inline bool isinf(const Y& val)
{
    return !isfinite(val) && !isnan(val);
}

template<class Y> 
inline bool isnan(const Y& val)
{
    return _isnan(val) != 0;
}

template<class Y> 
inline bool isnormal(const Y& val)
{
    int class_ = _fpclass(val);
    return class_ == _FPCLASS_NN || class_ == _FPCLASS_PN;
}

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
    return isnan(v1) || isnan(v2);
}

template<class Y>
inline Y abs(const Y& val)
{
    return ::abs(val);
}

template<class Y>
inline Y ceil(const Y& val)
{
    return ::ceil(val);
}

template<class Y>
inline Y copysign(const Y& v1,const Y& v2)
{
    return ::_copysign(v1,v2);
}

#if _MSC_VER == 1400
// unavailable on MSVC 7.1
template<>
inline long double copysign(const long double& v1,const long double& v2)
{
    return ::_copysignl(v1,v2);
}
#endif

template<class Y>
inline Y fabs(const Y& val)
{
    return ::fabs(val);
}

template<class Y>
inline Y floor(const Y& val)
{
    return ::floor(val);
}

template<class Y>
inline Y fdim(const Y& v1,const Y& v2)
{
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

template<class Y>
inline int fpclassify(const Y& val)
{
    return ::_fpclass(val);
}

template<class Y>
inline Y hypot(const Y& v1,const Y& v2)
{
    return ::_hypot(v1,v2);
}

namespace hypotf_impl {

struct convertible_from_float
{
    convertible_from_float(const float&) {}
};

typedef char no;

struct yes { no dummy[2]; };

struct hypot_result {};

hypot_result hypotf(const convertible_from_float&, const convertible_from_float&);
hypot_result _hypotf(const convertible_from_float&, const convertible_from_float&);

no has_hypot(hypot_result);

yes has_hypot(float);

template<class Float>
inline float do_hypot_(const Float& v1,const Float& v2, mpl::true_)
{
    return ::_hypotf(v1,v2);
}

template<class Float>
inline float do_hypot_(const Float& v1,const Float& v2, mpl::false_)
{
    return static_cast<float>(::_hypot(v1,v2));
}

template<class Float>
inline float do_hypot(const Float& v1,const Float& v2, mpl::true_)
{
    return ::_hypotf(v1,v2);
}

template<class Float>
inline float do_hypot(const Float& v1,const Float& v2, mpl::false_)
{
    mpl::bool_<(sizeof(hypotf_impl::has_hypot(_hypotf(v1, v2)))==sizeof(hypotf_impl::yes))> condition;
    return hypotf_impl::do_hypot_(v1,v2, condition);
}

}

template<>
inline float hypot(const float& v1,const float& v2)
{
    using namespace hypotf_impl;
    mpl::bool_<(sizeof(hypotf_impl::has_hypot(hypotf(v1, v2)))==sizeof(hypotf_impl::yes))> condition;
    return hypotf_impl::do_hypot(v1,v2,condition);
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
inline Y round(const Y& val)
{
    if(isnan(val)) return val;
    if(val == 0) return val;
    if(val > 0)
    {
        Y result1(val + .5);
        if(result1 != std::numeric_limits<Y>::infinity()) return std::floor(result1);
        Y result2(val - .5);
        Y result3(std::ceil(result2));
        if(result3 == result2)
            if(result3 == val) return val;
            else return std::ceil(val);
        else return result3;
    }
    else
    {
        Y result1(val - .5);
        if(result1 != -std::numeric_limits<Y>::infinity()) return std::ceil(result1);
        Y result2(val + .5);
        Y result3(std::floor(result2));
        if(result3 == result2)
            if(result3 == val) return val;
            else return std::floor(val);
        else return result3;
    }
}

template<class Y>
inline bool signbit(const Y& val)
{
    switch(fpclassify(val))
    {
    case _FPCLASS_SNAN:
    case _FPCLASS_QNAN:
        //whatever.

    case _FPCLASS_NINF:
    case _FPCLASS_NN:
    case _FPCLASS_ND:
    case _FPCLASS_NZ: return(true);
 
    case _FPCLASS_PZ:
    case _FPCLASS_PD:
    case _FPCLASS_PN:
    case _FPCLASS_PINF: return(false);
 
    }
    return(false);
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

#endif // BOOST_UNITS_CMATH_MSVC_IMPL_BOOST_1_35_HPP
