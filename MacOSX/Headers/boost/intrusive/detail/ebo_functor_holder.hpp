/////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Joaquin M Lopez Munoz  2006-2007
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/intrusive for documentation.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_INTRUSIVE_DETAIL_EBO_HOLDER_HPP
#define BOOST_INTRUSIVE_DETAIL_EBO_HOLDER_HPP

#include <boost/intrusive/detail/mpl.hpp>

namespace boost {
namespace intrusive {
namespace detail {

template<typename T, bool IsEmpty = true>
class ebo_functor_holder_impl
{
   public:
   ebo_functor_holder_impl()
   {}
   ebo_functor_holder_impl(const T& t)
      :  t(t)
   {}
   template<class Arg1, class Arg2>
   ebo_functor_holder_impl(const Arg1& arg1, const Arg2& arg2)
      :  t(arg1, arg2)
   {}

   T&       get(){return t;}
   const T& get()const{return t;}

   private:
   T t;
};

template<typename T>
class ebo_functor_holder_impl<T, false>
   :  public T
{
   public:
   ebo_functor_holder_impl()
   {}
   ebo_functor_holder_impl(const T& t)
      :  T(t)
   {}
   template<class Arg1, class Arg2>
   ebo_functor_holder_impl(const Arg1& arg1, const Arg2& arg2)
      :  T(arg1, arg2)
   {}

   T&       get(){return *this;}
   const T& get()const{return *this;}
};

template<typename T>
class ebo_functor_holder
   :  public ebo_functor_holder_impl<T, is_unary_or_binary_function<T>::value>
{
   private:
   typedef ebo_functor_holder_impl<T, is_unary_or_binary_function<T>::value> super;

   public:
   ebo_functor_holder(){}
   ebo_functor_holder(const T& t)
      :  super(t)
   {}

   template<class Arg1, class Arg2>
   ebo_functor_holder(const Arg1& arg1, const Arg2& arg2)
      :  super(arg1, arg2)
   {}

   ebo_functor_holder& operator=(const ebo_functor_holder& x)
   {
      this->get()=x.get();
      return *this;
   }
};


}  //namespace detail {
}  //namespace intrusive {
}  //namespace boost {

#endif   //#ifndef BOOST_INTRUSIVE_DETAIL_EBO_HOLDER_HPP
