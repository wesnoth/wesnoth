//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2005-2008.
// (C) Copyright Gennaro Prota 2003 - 2004.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/interprocess for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_INTERPROCESS_DETAIL_UTILITIES_HPP
#define BOOST_INTERPROCESS_DETAIL_UTILITIES_HPP

#if (defined _MSC_VER) && (_MSC_VER >= 1200)
#  pragma once
#endif

#include <boost/interprocess/detail/config_begin.hpp>
#include <boost/interprocess/detail/workaround.hpp>

#include <boost/interprocess/interprocess_fwd.hpp>
#include <boost/interprocess/detail/move.hpp>
#include <boost/type_traits/has_trivial_destructor.hpp>
#include <boost/interprocess/detail/min_max.hpp>
#include <boost/interprocess/detail/type_traits.hpp>
#include <boost/interprocess/detail/iterators.hpp>
#include <boost/interprocess/detail/version_type.hpp>
#ifndef BOOST_INTERPROCESS_PERFECT_FORWARDING
#include <boost/interprocess/detail/preprocessor.hpp>
#endif
#include <utility>
#include <algorithm>

namespace boost {
namespace interprocess { 
namespace detail {

template<class SmartPtr>
struct smart_ptr_type
{
   typedef typename SmartPtr::value_type value_type;
   typedef value_type *pointer;
   static pointer get (const SmartPtr &smartptr)
   {  return smartptr.get();}
};

template<class T>
struct smart_ptr_type<T*>
{
   typedef T value_type;
   typedef value_type *pointer;
   static pointer get (pointer ptr)
   {  return ptr;}
};

//!Overload for smart pointers to avoid ADL problems with get_pointer
template<class Ptr>
inline typename smart_ptr_type<Ptr>::pointer
get_pointer(const Ptr &ptr)
{  return smart_ptr_type<Ptr>::get(ptr);   }

//!To avoid ADL problems with swap
template <class T>
inline void do_swap(T& x, T& y)
{
   using std::swap;
   swap(x, y);
}

//!A deleter for scoped_ptr that deallocates the memory
//!allocated for an object using a STL allocator.
template <class Allocator>
struct scoped_ptr_dealloc_functor
{
   typedef typename Allocator::pointer pointer;
   typedef detail::integral_constant<unsigned,
      boost::interprocess::detail::
         version<Allocator>::value>                   alloc_version;
   typedef detail::integral_constant<unsigned, 1>     allocator_v1;
   typedef detail::integral_constant<unsigned, 2>     allocator_v2;

   private:
   void priv_deallocate(const typename Allocator::pointer &p, allocator_v1)
   {  m_alloc.deallocate(p, 1); }

   void priv_deallocate(const typename Allocator::pointer &p, allocator_v2)
   {  m_alloc.deallocate_one(p); }

   public:
   Allocator& m_alloc;

   scoped_ptr_dealloc_functor(Allocator& a)
      : m_alloc(a) {}

   void operator()(pointer ptr)
   {  if (ptr) priv_deallocate(ptr, alloc_version());  }
};

//!A deleter for scoped_ptr that deallocates the memory
//!allocated for an object using a STL allocator.
template <class Allocator>
struct scoped_deallocator
{
   typedef typename Allocator::pointer pointer;
   typedef detail::integral_constant<unsigned,
      boost::interprocess::detail::
         version<Allocator>::value>                   alloc_version;
   typedef detail::integral_constant<unsigned, 1>     allocator_v1;
   typedef detail::integral_constant<unsigned, 2>     allocator_v2;

   private:
   void priv_deallocate(allocator_v1)
   {  m_alloc.deallocate(m_ptr, 1); }

   void priv_deallocate(allocator_v2)
   {  m_alloc.deallocate_one(m_ptr); }

   scoped_deallocator(const scoped_deallocator &);
   scoped_deallocator& operator=(const scoped_deallocator &);

   public:
   pointer     m_ptr;
   Allocator&  m_alloc;

   scoped_deallocator(pointer p, Allocator& a)
      : m_ptr(p), m_alloc(a) {}

   ~scoped_deallocator()
   {  if (m_ptr)priv_deallocate(alloc_version());  }

   #ifdef BOOST_INTERPROCESS_RVALUE_REFERENCE
   scoped_deallocator(scoped_deallocator &&o)
      :  m_ptr(o.m_ptr), m_alloc(o.m_alloc)
   {
   #else
   scoped_deallocator(moved_object<scoped_deallocator> mo)
      :  m_ptr(mo.get().m_ptr), m_alloc(mo.get().m_alloc)
   {
      scoped_deallocator &o = mo.get();
   #endif
      o.release();
   }

   pointer get() const
   {  return m_ptr;  }

   void release()
   {  m_ptr = 0; }
};

}  //namespace detail {

template <class Allocator>
struct is_movable<boost::interprocess::detail::scoped_deallocator<Allocator> >
{
   static const bool value = true;
};

namespace detail {

//!A deleter for scoped_ptr that deallocates the memory
//!allocated for an array of objects using a STL allocator.
template <class Allocator>
struct scoped_array_deallocator
{
   typedef typename Allocator::pointer    pointer;
   typedef typename Allocator::size_type  size_type;

   scoped_array_deallocator(pointer p, Allocator& a, size_type length)
      : m_ptr(p), m_alloc(a), m_length(length) {}

   ~scoped_array_deallocator()
   {  if (m_ptr) m_alloc.deallocate(m_ptr, m_length);  }

   void release()
   {  m_ptr = 0; }

   private:
   pointer     m_ptr;
   Allocator&  m_alloc;
   size_type   m_length;
};

template <class Allocator>
struct null_scoped_array_deallocator
{
   typedef typename Allocator::pointer    pointer;
   typedef typename Allocator::size_type  size_type;

   null_scoped_array_deallocator(pointer, Allocator&, size_type)
   {}

   void release()
   {}
};

//!A deleter for scoped_ptr that destroys
//!an object using a STL allocator.
template <class Allocator>
struct scoped_destructor_n
{
   typedef typename Allocator::pointer    pointer;
   typedef typename Allocator::value_type value_type;
   typedef typename Allocator::size_type  size_type;

   pointer     m_p;
   size_type   m_n;

   scoped_destructor_n(pointer p, size_type n)
      : m_p(p), m_n(n)
   {}

   void release()
   {  m_p = 0; }

   void increment_size(size_type inc)
   {  m_n += inc;   }
   
   ~scoped_destructor_n()
   {
      if(!m_p) return;
      value_type *raw_ptr = detail::get_pointer(m_p);
      for(std::size_t i = 0; i < m_n; ++i, ++raw_ptr)
         raw_ptr->~value_type();
   }
};

//!A deleter for scoped_ptr that destroys
//!an object using a STL allocator.
template <class Allocator>
struct null_scoped_destructor_n
{
   typedef typename Allocator::pointer pointer;
   typedef typename Allocator::size_type size_type;

   null_scoped_destructor_n(pointer, size_type)
   {}

   void increment_size(size_type)
   {}

   void release()
   {}
};

template <class A>
class allocator_destroyer
{
   typedef typename A::value_type value_type;
   typedef detail::integral_constant<unsigned,
      boost::interprocess::detail::
         version<A>::value>                           alloc_version;
   typedef detail::integral_constant<unsigned, 1>     allocator_v1;
   typedef detail::integral_constant<unsigned, 2>     allocator_v2;

   private:
   A & a_;

   private:
   void priv_deallocate(const typename A::pointer &p, allocator_v1)
   {  a_.deallocate(p, 1); }

   void priv_deallocate(const typename A::pointer &p, allocator_v2)
   {  a_.deallocate_one(p); }

   public:
   allocator_destroyer(A &a)
      :  a_(a)
   {}

   void operator()(const typename A::pointer &p)
   {  
      detail::get_pointer(p)->~value_type();
      priv_deallocate(p, alloc_version());
   }
};

template <class A>
class allocator_destroyer_and_chain_builder
{
   typedef typename A::value_type value_type;
   typedef typename A::multiallocation_iterator multiallocation_iterator;
   typedef typename A::multiallocation_chain    multiallocation_chain;

   A & a_;
   multiallocation_chain &c_;

   public:
   allocator_destroyer_and_chain_builder(A &a, multiallocation_chain &c)
      :  a_(a), c_(c)
   {}

   void operator()(const typename A::pointer &p)
   {  
      value_type *vp = detail::get_pointer(p);
      vp->~value_type();
      c_.push_back(vp);
   }
};

template <class A>
class allocator_multialloc_chain_node_deallocator
{
   typedef typename A::value_type value_type;
   typedef typename A::multiallocation_iterator multiallocation_iterator;
   typedef typename A::multiallocation_chain    multiallocation_chain;
   typedef allocator_destroyer_and_chain_builder<A> chain_builder;

   A & a_;
   multiallocation_chain c_;

   public:
   allocator_multialloc_chain_node_deallocator(A &a)
      :  a_(a), c_()
   {}

   chain_builder get_chain_builder()
   {  return chain_builder(a_, c_);  }

   ~allocator_multialloc_chain_node_deallocator()
   {
      multiallocation_iterator it(c_.get_it());
      if(it != multiallocation_iterator())
         a_.deallocate_individual(it);
   }
};

template <class A>
class allocator_multialloc_chain_array_deallocator
{
   typedef typename A::value_type value_type;
   typedef typename A::multiallocation_iterator multiallocation_iterator;
   typedef typename A::multiallocation_chain    multiallocation_chain;
   typedef allocator_destroyer_and_chain_builder<A> chain_builder;

   A & a_;
   multiallocation_chain c_;

   public:
   allocator_multialloc_chain_array_deallocator(A &a)
      :  a_(a), c_()
   {}

   chain_builder get_chain_builder()
   {  return chain_builder(a_, c_);  }

   ~allocator_multialloc_chain_array_deallocator()
   {
      multiallocation_iterator it(c_.get_it());
      if(it != multiallocation_iterator())
         a_.deallocate_many(it);
   }
};

//!A class used for exception-safe multi-allocation + construction.
template <class Allocator>
struct multiallocation_destroy_dealloc
{
   typedef typename Allocator::multiallocation_iterator multiallocation_iterator;
   typedef typename Allocator::value_type value_type;

   multiallocation_iterator m_itbeg;
   Allocator&  m_alloc;

   multiallocation_destroy_dealloc(multiallocation_iterator itbeg, Allocator& a)
      : m_itbeg(itbeg), m_alloc(a) {}

   ~multiallocation_destroy_dealloc()
   {
      multiallocation_iterator endit;
      while(m_itbeg != endit){
         detail::get_pointer(&*m_itbeg)->~value_type();
         m_alloc.deallocate(&*m_itbeg, 1);
         ++m_itbeg;
      }
   }

   void next()
   {  ++m_itbeg; }

   void release()
   {  m_itbeg = multiallocation_iterator(); }
};

//Rounds "orig_size" by excess to round_to bytes
inline std::size_t get_rounded_size(std::size_t orig_size, std::size_t round_to)
{
   return ((orig_size-1)/round_to+1)*round_to;
}

//Truncates "orig_size" to a multiple of "multiple" bytes.
inline std::size_t get_truncated_size(std::size_t orig_size, std::size_t multiple)
{
   return orig_size/multiple*multiple;
}

//Rounds "orig_size" by excess to round_to bytes. round_to must be power of two
inline std::size_t get_rounded_size_po2(std::size_t orig_size, std::size_t round_to)
{
   return ((orig_size-1)&(~(round_to-1))) + round_to;
}

//Truncates "orig_size" to a multiple of "multiple" bytes. multiple must be power of two
inline std::size_t get_truncated_size_po2(std::size_t orig_size, std::size_t multiple)
{
   return (orig_size & (~(multiple-1)));
}

template <std::size_t OrigSize, std::size_t RoundTo>
struct ct_rounded_size
{
   enum { value = ((OrigSize-1)/RoundTo+1)*RoundTo };
};

template <std::size_t Value1, std::size_t Value2>
struct ct_min
{
   enum { value = (Value1 < Value2)? Value1 : Value2 };
};

template <std::size_t Value1, std::size_t Value2>
struct ct_max
{
   enum { value = (Value1 > Value2)? Value1 : Value2 };
};

// Gennaro Prota wrote this. Thanks!
template <int p, int n = 4>
struct ct_max_pow2_less
{
   enum { c = 2*n < p };

   static const std::size_t value =
         c ? (ct_max_pow2_less< c*p, 2*c*n>::value) : n;
};

template <>
struct ct_max_pow2_less<0, 0>
{
   static const std::size_t value = 0;
};

//!Obtains a generic pointer of the same type that
//!can point to other pointed type: Ptr<?> -> Ptr<NewValueType>
template<class T, class U>
struct pointer_to_other;

template<class T, class U, 
         template<class> class Sp>
struct pointer_to_other< Sp<T>, U >
{
   typedef Sp<U> type;
};

template<class T, class T2, class U, 
         template<class, class> class Sp>
struct pointer_to_other< Sp<T, T2>, U >
{
   typedef Sp<U, T2> type;
};

template<class T, class T2, class T3, class U, 
         template<class, class, class> class Sp>
struct pointer_to_other< Sp<T, T2, T3>, U >
{
   typedef Sp<U, T2, T3> type;
};

template<class T, class U>
struct pointer_to_other< T*, U >
{
   typedef U* type;
};

}  //namespace detail {

//!Trait class to detect if an index is a node
//!index. This allows more efficient operations
//!when deallocating named objects.
template <class Index>
struct is_node_index
{
   enum {   value = false };
};


//!Trait class to detect if an index is an intrusive
//!index. This will embed the derivation hook in each
//!allocation header, to provide memory for the intrusive
//!container.
template <class Index>
struct is_intrusive_index
{
   enum {   value = false };
};

template <class SizeType>
SizeType
   get_next_capacity(const SizeType max_size
                    ,const SizeType capacity
                    ,const SizeType n)
{
//   if (n > max_size - capacity)
//      throw std::length_error("get_next_capacity");

   const SizeType m3 = max_size/3;

   if (capacity < m3)
      return capacity + max_value(3*(capacity+1)/5, n);

   if (capacity < m3*2)
      return capacity + max_value((capacity+1)/2, n);

   return max_size;
}

namespace detail {

template <class T1, class T2>
struct pair
{
   typedef T1 first_type;
   typedef T2 second_type;

   T1 first;
   T2 second;

   //std::pair compatibility
   template <class D, class S>
   pair(const std::pair<D, S>& p)
      : first(p.first), second(p.second)
   {}

   //To resolve ambiguity with the variadic constructor of 1 argument
   //and the previous constructor
   pair(std::pair<T1, T2>& x)
      : first(x.first), second(x.second)
   {}

   #ifndef BOOST_INTERPROCESS_RVALUE_REFERENCE
   template <class D, class S>
   pair(const detail::moved_object<std::pair<D, S> >& p)
      : first(detail::move_impl(p.get().first)), second(detail::move_impl(p.get().second))
   {}
   #else
   template <class D, class S>
   pair(std::pair<D, S> && p)
      : first(detail::move_impl(p.first)), second(detail::move_impl(p.second))
   {}
   #endif

   pair()
      : first(), second()
   {}

   pair(const pair<T1, T2>& x)
      : first(x.first), second(x.second)
   {}

   //To resolve ambiguity with the variadic constructor of 1 argument
   //and the copy constructor
   pair(pair<T1, T2>& x)
      : first(x.first), second(x.second)
   {}

   #ifndef BOOST_INTERPROCESS_RVALUE_REFERENCE
   pair(const detail::moved_object<pair<T1, T2> >& p)
      : first(detail::move_impl(p.get().first)), second(detail::move_impl(p.get().second))
   {}
   #else
   pair(pair<T1, T2> && p)
      : first(detail::move_impl(p.first)), second(detail::move_impl(p.second))
   {}
   #endif

   #ifndef BOOST_INTERPROCESS_RVALUE_REFERENCE
   template <class D, class S>
   pair(const detail::moved_object<pair<D, S> >& p)
      : first(detail::move_impl(p.get().first)), second(detail::move_impl(p.get().second))
   {}
   #else
   template <class D, class S>
   pair(pair<D, S> && p)
      : first(detail::move_impl(p.first)), second(detail::move_impl(p.second))
   {}
   #endif

   #ifdef BOOST_INTERPROCESS_PERFECT_FORWARDING

   template<class U, class ...Args>
   pair(U &&u, Args &&... args)
      : first(detail::forward_impl<U>(u))
      , second(detail::forward_impl<Args>(args)...)
   {}

   #else

   template<class U>
   pair(BOOST_INTERPROCESS_PARAM(U, u))
      : first(detail::forward_impl<U>(u))
   {}                                                                     

   #define BOOST_PP_LOCAL_MACRO(n)                                                              \
   template<class U, BOOST_PP_ENUM_PARAMS(n, class P)>                                          \
   pair(BOOST_INTERPROCESS_PARAM(U, u)                                                          \
       ,BOOST_PP_ENUM(n, BOOST_INTERPROCESS_PP_PARAM_LIST, _))                                  \
      : first(detail::forward_impl<U>(u))                                                       \
      , second(BOOST_PP_ENUM(n, BOOST_INTERPROCESS_PP_PARAM_FORWARD, _))                        \
   {}                                                                                           \
   //!
   #define BOOST_PP_LOCAL_LIMITS (1, BOOST_INTERPROCESS_MAX_CONSTRUCTOR_PARAMETERS)
   #include BOOST_PP_LOCAL_ITERATE()
   #endif

   #ifndef BOOST_INTERPROCESS_RVALUE_REFERENCE
   pair& operator=(const detail::moved_object<pair<T1, T2> > &p)
   {
      first  = detail::move_impl(p.get().first);
      second = detail::move_impl(p.get().second);
      return *this;
   }
   #else
   pair& operator=(pair<T1, T2> &&p)
   {
      first  = detail::move_impl(p.first);
      second = detail::move_impl(p.second);
      return *this;
   }
   #endif

   #ifndef BOOST_INTERPROCESS_RVALUE_REFERENCE
   pair& operator=(const detail::moved_object<std::pair<T1, T2> > &p)
   {
      first  = detail::move_impl(p.get().first);
      second = detail::move_impl(p.get().second);
      return *this;
   }
   #else
   pair& operator=(std::pair<T1, T2> &&p)
   {
      first  = detail::move_impl(p.first);
      second = detail::move_impl(p.second);
      return *this;
   }
   #endif

   #ifndef BOOST_INTERPROCESS_RVALUE_REFERENCE
   template <class D, class S>
   pair& operator=(const detail::moved_object<std::pair<D, S> > &p)
   {
      first  = detail::move_impl(p.get().first);
      second = detail::move_impl(p.get().second);
      return *this;
   }
   #else
   template <class D, class S>
   pair& operator=(std::pair<D, S> &&p)
   {
      first  = detail::move_impl(p.first);
      second = detail::move_impl(p.second);
      return *this;
   }
   #endif

   #ifndef BOOST_INTERPROCESS_RVALUE_REFERENCE
   void swap(const detail::moved_object<pair> &p)
   {  std::swap(*this, p.get()); }

   void swap(pair& p)
   {  std::swap(*this, p); }

   #else
   void swap(pair &&p)
   {  std::swap(*this, p); }
   #endif
};

template <class T1, class T2>
inline bool operator==(const pair<T1,T2>& x, const pair<T1,T2>& y)
{  return static_cast<bool>(x.first == y.first && x.second == y.second);  }

template <class T1, class T2>
inline bool operator< (const pair<T1,T2>& x, const pair<T1,T2>& y)
{  return static_cast<bool>(x.first < y.first ||
                         (!(y.first < x.first) && x.second < y.second)); }

template <class T1, class T2>
inline bool operator!=(const pair<T1,T2>& x, const pair<T1,T2>& y)
{  return static_cast<bool>(!(x == y));  }

template <class T1, class T2>
inline bool operator> (const pair<T1,T2>& x, const pair<T1,T2>& y)
{  return y < x;  }

template <class T1, class T2>
inline bool operator>=(const pair<T1,T2>& x, const pair<T1,T2>& y)
{  return static_cast<bool>(!(x < y)); }

template <class T1, class T2>
inline bool operator<=(const pair<T1,T2>& x, const pair<T1,T2>& y)
{  return static_cast<bool>(!(y < x)); }

template <class T1, class T2>
inline pair<T1, T2> make_pair(T1 x, T2 y)
{  return pair<T1, T2>(x, y); }

#ifndef BOOST_INTERPROCESS_RVALUE_REFERENCE
template <class T1, class T2>
inline void swap(const detail::moved_object<pair<T1, T2> > &x, pair<T1, T2>& y)
{
   swap(x.get().first, y.first);
   swap(x.get().second, y.second);
}

template <class T1, class T2>
inline void swap(pair<T1, T2>& x, const detail::moved_object<pair<T1, T2> > &y)
{
   swap(x.first, y.get().first);
   swap(x.second, y.get().second);
}

template <class T1, class T2>
inline void swap(pair<T1, T2>& x, pair<T1, T2>& y)
{
   swap(x.first, y.first);
   swap(x.second, y.second);
}

#else
template <class T1, class T2>
inline void swap(pair<T1, T2>&&x, pair<T1, T2>&&y)
{
   swap(x.first, y.first);
   swap(x.second, y.second);
}
#endif

template<class T>
struct cast_functor
{
   typedef typename detail::add_reference<T>::type result_type;
   result_type operator()(char &ptr) const
   {  return *static_cast<T*>(static_cast<void*>(&ptr));  }
};

template<class MultiallocChain, class T>
class multiallocation_chain_adaptor
{
   private:
   MultiallocChain   chain_;

   multiallocation_chain_adaptor
      (const multiallocation_chain_adaptor &);
   multiallocation_chain_adaptor &operator=
      (const multiallocation_chain_adaptor &);

   public:
   typedef transform_iterator
      < typename MultiallocChain::
         multiallocation_iterator
      , detail::cast_functor <T> >        multiallocation_iterator;

   multiallocation_chain_adaptor()
      : chain_()
   {}

   void push_back(T *mem)
   {  chain_.push_back(mem);  }

   void push_front(T *mem)
   {  chain_.push_front(mem);  }

   void swap(multiallocation_chain_adaptor &other_chain)
   {  chain_.swap(other_chain.chain_); }

   void splice_back(multiallocation_chain_adaptor &other_chain)
   {  chain_.splice_back(other_chain.chain_);   }

   T *pop_front()
   {  return static_cast<T*>(chain_.pop_front());   }

   bool empty() const
   {  return chain_.empty(); }

   multiallocation_iterator get_it() const
   {  return multiallocation_iterator(chain_.get_it()); }

   std::size_t size() const
   {  return chain_.size(); }
};

template<class T>
struct value_init
{
   value_init()
      : m_t()
   {}

   T m_t;
};

}  //namespace detail {

//!The pair is movable if any of its members is movable
template <class T1, class T2>
struct is_movable<boost::interprocess::detail::pair<T1, T2> >
{
   enum {  value = is_movable<T1>::value || is_movable<T2>::value };
};

//!The pair is movable if any of its members is movable
template <class T1, class T2>
struct is_movable<std::pair<T1, T2> >
{
   enum {  value = is_movable<T1>::value || is_movable<T2>::value };
};

///has_trivial_destructor_after_move<> == true_type
///specialization for optimizations
template <class T>
struct has_trivial_destructor_after_move
   : public boost::has_trivial_destructor<T>
{};

template <typename T> T*
addressof(T& v)
{
  return reinterpret_cast<T*>(
       &const_cast<char&>(reinterpret_cast<const volatile char &>(v)));
}

//Anti-exception node eraser
template<class Cont>
class value_eraser
{
   public:
   value_eraser(Cont & cont, typename Cont::iterator it) 
      : m_cont(cont), m_index_it(it), m_erase(true){}
   ~value_eraser()  
   {  if(m_erase) m_cont.erase(m_index_it);  }

   void release() {  m_erase = false;  }

   private:
   Cont                   &m_cont;
   typename Cont::iterator m_index_it;
   bool                    m_erase;
};

template <class T>
struct sizeof_value
{
   static const std::size_t value = sizeof(T);
};

template <>
struct sizeof_value<void>
{
   static const std::size_t value = sizeof(void*);
};

template <>
struct sizeof_value<const void>
{
   static const std::size_t value = sizeof(void*);
};

template <>
struct sizeof_value<volatile void>
{
   static const std::size_t value = sizeof(void*);
};

template <>
struct sizeof_value<const volatile void>
{
   static const std::size_t value = sizeof(void*);
};

}  //namespace interprocess { 
}  //namespace boost {

#include <boost/interprocess/detail/config_end.hpp>

#endif   //#ifndef BOOST_INTERPROCESS_DETAIL_UTILITIES_HPP

