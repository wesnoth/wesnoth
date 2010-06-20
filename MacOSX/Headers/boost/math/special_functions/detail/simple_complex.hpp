//  Copyright (c) 2007 John Maddock
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_MATH_SF_DETAIL_SIMPLE_COMPLEX_HPP
#define BOOST_MATH_SF_DETAIL_SIMPLE_COMPLEX_HPP

#ifdef _MSC_VER
#pragma once
#endif

namespace boost{ namespace math{ namespace detail{ namespace sc{

template <class T>
class simple_complex
{
public:
   simple_complex() : r(0), i(0) {}
   simple_complex(T a) : r(a) {}
   template <class U>
   simple_complex(U a) : r(a) {}
   simple_complex(T a, T b) : r(a), i(b) {}

   simple_complex& operator += (const simple_complex& o)
   {
      r += o.r;
      i += o.i;
      return *this;
   }
   simple_complex& operator -= (const simple_complex& o)
   {
      r -= o.r;
      i -= o.i;
      return *this;
   }
   simple_complex& operator *= (const simple_complex& o)
   { 
      T lr = r * o.r - i * o.i;
      T li = i * o.r + r * o.i;
      r = lr;
      i = li;
      return *this;
   }
   simple_complex& operator /= (const simple_complex& o)
   { 
      BOOST_MATH_STD_USING
      T lr;
      T li;
      if(fabs(o.r) > fabs(o.i))
      {
         T rat = o.i / o.r;
         lr = r + i * rat;
         li = i - r * rat;
         rat = o.r + o.i * rat;
         lr /= rat;
         li /= rat;
      }
      else
      {
         T rat = o.r / o.i;
         lr = i + r * rat;
         li = i * rat - r;
         rat = o.r * rat + o.i;
         lr /= rat;
         li /= rat;
      }
      r = lr;
      i = li;
      return *this;
   }
   bool operator == (const simple_complex& o)
   {
      return (r == o.r) && (i == o.i);
   }
   bool operator != (const simple_complex& o)
   {
      return !((r == o.r) && (i == o.i));
   }
   bool operator == (const T& o)
   {
      return (r == o) && (i == 0);
   }
   simple_complex& operator += (const T& o)
   {
      r += o;
      return *this;
   }
   simple_complex& operator -= (const T& o)
   {
      r -= o;
      return *this;
   }
   simple_complex& operator *= (const T& o)
   { 
      r *= o;
      i *= o;
      return *this;
   }
   simple_complex& operator /= (const T& o)
   { 
      r /= o;
      i /= o;
      return *this;
   }
   T real()const
   {
      return r;
   }
   T imag()const
   {
      return i;
   }
private:
   T r, i;
};

template <class T>
inline simple_complex<T> operator+(const simple_complex<T>& a, const simple_complex<T>& b)
{
   simple_complex<T> result(a);
   result += b;
   return result;
}

template <class T>
inline simple_complex<T> operator-(const simple_complex<T>& a, const simple_complex<T>& b)
{
   simple_complex<T> result(a);
   result -= b;
   return result;
}

template <class T>
inline simple_complex<T> operator*(const simple_complex<T>& a, const simple_complex<T>& b)
{
   simple_complex<T> result(a);
   result *= b;
   return result;
}

template <class T>
inline simple_complex<T> operator/(const simple_complex<T>& a, const simple_complex<T>& b)
{
   simple_complex<T> result(a);
   result /= b;
   return result;
}

template <class T>
inline T real(const simple_complex<T>& c)
{
   return c.real();
}

template <class T>
inline T imag(const simple_complex<T>& c)
{
   return c.imag();
}

template <class T>
inline T abs(const simple_complex<T>& c)
{
   return hypot(c.real(), c.imag());
}

}}}} // namespace

#endif


