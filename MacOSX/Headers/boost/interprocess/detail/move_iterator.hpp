//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2006. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/interprocess for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_INTERPROCESS_MOVE_ITERATOR_HPP_INCLUDED
#define BOOST_INTERPROCESS_MOVE_ITERATOR_HPP_INCLUDED

#include <iterator>
#include <boost/interprocess/detail/move.hpp>

namespace boost{
namespace interprocess{
namespace detail{

template <class It>
class move_iterator
{
   public:
   typedef It                                                              iterator_type;
   typedef typename std::iterator_traits<iterator_type>::value_type        value_type;
   #ifndef BOOST_INTERPROCESS_RVALUE_REFERENCE
   typedef typename move_type<value_type>::type                            reference;
   #else
   typedef value_type &&                                                   reference;
   #endif
   typedef typename std::iterator_traits<iterator_type>::pointer           pointer;
   typedef typename std::iterator_traits<iterator_type>::difference_type   difference_type;
   typedef typename std::iterator_traits<iterator_type>::iterator_category iterator_category;

   move_iterator()
   {}

   explicit move_iterator(It i)
      :  m_it(i)
   {}

   template <class U>
   move_iterator(const move_iterator<U>& u)
      :  m_it(u.base())
   {}

   const iterator_type &base() const
   {  return m_it;   }

   iterator_type &base()
   {  return m_it;   }

   reference operator*() const
   #ifndef BOOST_INTERPROCESS_RVALUE_REFERENCE
   {  return detail::move_impl(*m_it);  }
   #else
   {  return *m_it;  }
   #endif

   pointer   operator->() const
   {  return m_it;   }

   move_iterator& operator++()
   {  ++m_it; return *this;   }

   move_iterator<iterator_type>  operator++(int)
   {  move_iterator<iterator_type> tmp(*this); ++(*this); return tmp;   }

   move_iterator& operator--()
   {  --m_it; return *this;   }

   move_iterator<iterator_type>  operator--(int)
   {  move_iterator<iterator_type> tmp(*this); --(*this); return tmp;   }

   move_iterator<iterator_type>  operator+ (difference_type n) const
   {  return move_iterator<iterator_type>(m_it + n);  }

   move_iterator& operator+=(difference_type n)
   {  m_it += n; return *this;   }

   move_iterator<iterator_type>  operator- (difference_type n) const
   {  return move_iterator<iterator_type>(m_it - n);  }

   move_iterator& operator-=(difference_type n)
   {  m_it -= n; return *this;   }

   reference operator[](difference_type n) const
   {  return detail::move_impl(m_it[n]);   }

   private:
   It m_it;
};

template <class It> inline
bool operator==(const move_iterator<It>& x, const move_iterator<It>& y)
{  return x.base() == y.base();  }

template <class It> inline
bool operator!=(const move_iterator<It>& x, const move_iterator<It>& y)
{  return x.base() != y.base();  }

template <class It> inline
bool operator< (const move_iterator<It>& x, const move_iterator<It>& y)
{  return x.base() < y.base();   }

template <class It> inline
bool operator<=(const move_iterator<It>& x, const move_iterator<It>& y)
{  return x.base() <= y.base();  }

template <class It> inline
bool operator> (const move_iterator<It>& x, const move_iterator<It>& y)
{  return x.base() > y.base();  }

template <class It> inline
bool operator>=(const move_iterator<It>& x, const move_iterator<It>& y)
{  return x.base() >= y.base();  }

template <class It> inline
typename move_iterator<It>::difference_type
   operator-(const move_iterator<It>& x, const move_iterator<It>& y)
{  return x.base() - y.base();   }

template <class It> inline
move_iterator<It>
   operator+(typename move_iterator<It>::difference_type n
            ,const move_iterator<It>& x)
{  return move_iterator<It>(x.base() + n);   }

template<class It>
move_iterator<It> make_move_iterator(const It &it)
{  return move_iterator<It>(it); }

}  //namespace detail{
}  //namespace interprocess{
}  //namespace boost{

#endif   //#ifndef BOOST_INTERPROCESS_MOVE_ITERATOR_HPP_INCLUDED
