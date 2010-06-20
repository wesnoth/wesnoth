//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2005-2008.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/interprocess for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_INTERPROCESS_DETAIL_ALGORITHMS_HPP
#define BOOST_INTERPROCESS_DETAIL_ALGORITHMS_HPP

#if (defined _MSC_VER) && (_MSC_VER >= 1200)
#  pragma once
#endif

#include <boost/interprocess/detail/config_begin.hpp>
#include <boost/interprocess/detail/workaround.hpp>
#include <boost/interprocess/detail/iterators.hpp>
#include <boost/interprocess/detail/type_traits.hpp>
#include <boost/type_traits/has_trivial_copy.hpp>
#include <boost/type_traits/has_trivial_assign.hpp>
#include <boost/detail/no_exceptions_support.hpp>
#include <boost/get_pointer.hpp>
#include <cstring>

namespace boost {
namespace interprocess { 

#if !defined(BOOST_INTERPROCESS_RVALUE_REFERENCE) || !defined(BOOST_INTERPROCESS_RVALUE_PAIR)

template<class T>
struct has_own_construct_from_it
{
   static const bool value = false;
};

namespace detail  {

template<class T, class InpIt>
inline void construct_in_place_impl(T* dest, const InpIt &source, detail::true_)
{
   T::construct(dest, *source);
}

template<class T, class InpIt>
inline void construct_in_place_impl(T* dest, const InpIt &source, detail::false_)
{
   new((void*)dest)T(*source);
}

}  //namespace detail   {

template<class T, class InpIt>
inline void construct_in_place(T* dest, InpIt source)
{
   typedef detail::bool_<has_own_construct_from_it<T>::value> boolean_t;
   detail::construct_in_place_impl(dest, source, boolean_t());
}

#else
template<class T, class InpIt>
inline void construct_in_place(T* dest, InpIt source)
{     new((void*)dest)T(*source);   }
#endif

template<class T, class U, class D>
inline void construct_in_place(T *dest, default_construct_iterator<U, D>)
{
   new((void*)dest)T();
}

template<class InIt, class OutIt>
struct optimize_assign
{
   static const bool value = false;
};

template<class T>
struct optimize_assign<const T*, T*>
{
   static const bool value = boost::has_trivial_assign<T>::value;
};

template<class T>
struct optimize_assign<T*, T*>
   :  public optimize_assign<const T*, T*>
{};

template<class InIt, class OutIt>
struct optimize_copy
{
   static const bool value = false;
};

template<class T>
struct optimize_copy<const T*, T*>
{
   static const bool value = boost::has_trivial_copy<T>::value;
};

template<class T>
struct optimize_copy<T*, T*>
   :  public optimize_copy<const T*, T*>
{};

template<class InIt, class OutIt> inline
OutIt copy_n_dispatch(InIt first, typename std::iterator_traits<InIt>::difference_type length, OutIt dest, detail::bool_<false>)
{
   for (; length--; ++dest, ++first)
      *dest = *first;
   return dest;
}

template<class T> inline
T *copy_n_dispatch(const T *first, typename std::iterator_traits<const T*>::difference_type length, T *dest, detail::bool_<true>)
{
   std::size_t size = length*sizeof(T);
   return (static_cast<T*>(std::memmove(dest, first, size))) + size;
}

template<class InIt, class OutIt> inline
OutIt copy_n(InIt first, typename std::iterator_traits<InIt>::difference_type length, OutIt dest)
{
   const bool do_optimized_assign = optimize_assign<InIt, OutIt>::value;
   return copy_n_dispatch(first, length, dest, detail::bool_<do_optimized_assign>());
}

template<class InIt, class FwdIt> inline
FwdIt uninitialized_copy_n_dispatch
   (InIt first, 
    typename std::iterator_traits<InIt>::difference_type count,
    FwdIt dest, detail::bool_<false>)
{
   typedef typename std::iterator_traits<FwdIt>::value_type value_type;
   //Save initial destination position
   FwdIt dest_init = dest;
   typename std::iterator_traits<InIt>::difference_type new_count = count+1;

   BOOST_TRY{
      //Try to build objects
      for (; --new_count; ++dest, ++first){
         construct_in_place(detail::get_pointer(&*dest), first);
      }
   }
   BOOST_CATCH(...){
      //Call destructors
      new_count = count - new_count;
      for (; new_count--; ++dest_init){
         detail::get_pointer(&*dest_init)->~value_type();
      }
      BOOST_RETHROW
   }
   BOOST_CATCH_END
   return dest;
}
template<class T> inline
T *uninitialized_copy_n_dispatch(const T *first, typename std::iterator_traits<const T*>::difference_type length, T *dest, detail::bool_<true>)
{
   std::size_t size = length*sizeof(T);
   return (static_cast<T*>(std::memmove(dest, first, size))) + size;
}

template<class InIt, class FwdIt> inline
FwdIt uninitialized_copy_n
   (InIt first, 
    typename std::iterator_traits<InIt>::difference_type count,
    FwdIt dest)
{
   const bool do_optimized_copy = optimize_copy<InIt, FwdIt>::value;
   return uninitialized_copy_n_dispatch(first, count, dest, detail::bool_<do_optimized_copy>());
}

// uninitialized_copy_copy
// Copies [first1, last1) into [result, result + (last1 - first1)), and
// copies [first2, last2) into
// [result + (last1 - first1), result + (last1 - first1) + (last2 - first2)).
template <class InpIt1, class InpIt2, class FwdIt>
FwdIt uninitialized_copy_copy
   (InpIt1 first1, InpIt1 last1, InpIt2 first2, InpIt2 last2, FwdIt result)
{
   typedef typename std::iterator_traits<FwdIt>::value_type value_type;
   FwdIt mid = std::uninitialized_copy(first1, last1, result);
   BOOST_TRY {
      return std::uninitialized_copy(first2, last2, mid);
   }
   BOOST_CATCH(...){
      for(;result != mid; ++result){
         detail::get_pointer(&*result)->~value_type();
      }
      BOOST_RETHROW
   }
   BOOST_CATCH_END
}

}  //namespace interprocess { 
}  //namespace boost {

#include <boost/interprocess/detail/config_end.hpp>

#endif   //#ifndef BOOST_INTERPROCESS_DETAIL_ALGORITHMS_HPP

