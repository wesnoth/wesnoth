//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2005-2008. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/interprocess for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_INTERPROCESS_FLAT_MAP_HPP
#define BOOST_INTERPROCESS_FLAT_MAP_HPP

#if (defined _MSC_VER) && (_MSC_VER >= 1200)
#  pragma once
#endif

#include <boost/interprocess/detail/config_begin.hpp>
#include <boost/interprocess/detail/workaround.hpp>

#include <boost/interprocess/interprocess_fwd.hpp>
#include <utility>
#include <functional>
#include <memory>
#include <boost/interprocess/containers/detail/flat_tree.hpp>
#include <boost/interprocess/detail/utilities.hpp>
#include <boost/type_traits/has_trivial_destructor.hpp>
#include <boost/interprocess/detail/mpl.hpp>
#include <boost/interprocess/detail/move.hpp>

namespace boost { namespace interprocess {

/// @cond
// Forward declarations of operators == and <, needed for friend declarations.
template <class Key, class T, class Pred, class Alloc>
class flat_map;

template <class Key, class T, class Pred, class Alloc>
inline bool operator==(const flat_map<Key,T,Pred,Alloc>& x, 
                       const flat_map<Key,T,Pred,Alloc>& y);

template <class Key, class T, class Pred, class Alloc>
inline bool operator<(const flat_map<Key,T,Pred,Alloc>& x, 
                      const flat_map<Key,T,Pred,Alloc>& y);
/// @endcond

//! A flat_map is a kind of associative container that supports unique keys (contains at
//! most one of each key value) and provides for fast retrieval of values of another 
//! type T based on the keys. The flat_map class supports random-access iterators.
//! 
//! A flat_map satisfies all of the requirements of a container and of a reversible 
//! container and of an associative container. A flat_map also provides 
//! most operations described for unique keys. For a 
//! flat_map<Key,T> the key_type is Key and the value_type is std::pair<Key,T>
//! (unlike std::map<Key, T> which value_type is std::pair<<b>const</b> Key, T>).
//!
//! Pred is the ordering function for Keys (e.g. <i>std::less<Key></i>).
//!
//! Alloc is the allocator to allocate the value_types
//! (e.g. <i>boost::interprocess:allocator< std::pair<Key, T></i>).
//! 
//! flat_map is similar to std::map but it's implemented like an ordered vector.
//! This means that inserting a new element into a flat_map invalidates
//! previous iterators and references
//!
//! Erasing an element of a flat_map invalidates iterators and references 
//! pointing to elements that come after (their keys are bigger) the erased element.
template <class Key, class T, class Pred, class Alloc>
class flat_map 
{
   /// @cond
   private:
   //This is the tree that we should store if pair was movable
   typedef detail::flat_tree<Key, 
                           std::pair<Key, T>, 
                           detail::select1st< std::pair<Key, T> >, 
                           Pred, 
                           Alloc> tree_t;

   //#ifndef BOOST_INTERPROCESS_RVALUE_REFERENCE
   //This is the real tree stored here. It's based on a movable pair
   typedef detail::flat_tree<Key, 
                           detail::pair<Key, T>, 
                           detail::select1st< detail::pair<Key, T> >, 
                           Pred, 
                           typename Alloc::template
                              rebind<detail::pair<Key, T> >::other> impl_tree_t;
/*
   #else
   typedef tree_t    impl_tree_t;
   #endif   
*/
   impl_tree_t m_flat_tree;  // flat tree representing flat_map

   typedef typename impl_tree_t::value_type             impl_value_type;
   typedef typename impl_tree_t::pointer                impl_pointer;
   typedef typename impl_tree_t::const_pointer          impl_const_pointer;
   typedef typename impl_tree_t::reference              impl_reference;
   typedef typename impl_tree_t::const_reference        impl_const_reference;
   typedef typename impl_tree_t::value_compare          impl_value_compare;
   typedef typename impl_tree_t::iterator               impl_iterator;
   typedef typename impl_tree_t::const_iterator         impl_const_iterator;
   typedef typename impl_tree_t::reverse_iterator       impl_reverse_iterator;
   typedef typename impl_tree_t::const_reverse_iterator impl_const_reverse_iterator;
   typedef typename impl_tree_t::allocator_type         impl_allocator_type;
   #ifndef BOOST_INTERPROCESS_RVALUE_REFERENCE
   typedef detail::moved_object<impl_value_type>        impl_moved_value_type;
   #endif

   //#ifndef BOOST_INTERPROCESS_RVALUE_REFERENCE
   template<class D, class S>
   static D &force(const S &s)
   {  return *const_cast<D*>(reinterpret_cast<const D*>(&s)); }

   template<class D, class S>
   static D force_copy(S s)
   {
      value_type *vp = reinterpret_cast<value_type *>(&*s);
      return D(vp);
   }

   /// @endcond

   public:
   // typedefs:
   typedef typename tree_t::key_type               key_type;
   typedef typename tree_t::value_type             value_type;
   typedef typename tree_t::pointer                pointer;
   typedef typename tree_t::const_pointer          const_pointer;
   typedef typename tree_t::reference              reference;
   typedef typename tree_t::const_reference        const_reference;
   typedef typename tree_t::value_compare          value_compare;
   typedef T                                       mapped_type;
   typedef typename tree_t::key_compare            key_compare;
   typedef typename tree_t::iterator               iterator;
   typedef typename tree_t::const_iterator         const_iterator;
   typedef typename tree_t::reverse_iterator       reverse_iterator;
   typedef typename tree_t::const_reverse_iterator const_reverse_iterator;
   typedef typename tree_t::size_type              size_type;
   typedef typename tree_t::difference_type        difference_type;
   typedef typename tree_t::allocator_type         allocator_type;
   typedef typename tree_t::stored_allocator_type  stored_allocator_type;

   //! <b>Effects</b>: Constructs an empty flat_map using the specified
   //! comparison object and allocator.
   //! 
   //! <b>Complexity</b>: Constant.
   explicit flat_map(const Pred& comp = Pred(), const allocator_type& a = allocator_type()) 
      : m_flat_tree(comp, force<impl_allocator_type>(a)) {}

   //! <b>Effects</b>: Constructs an empty flat_map using the specified comparison object and 
   //! allocator, and inserts elements from the range [first ,last ).
   //! 
   //! <b>Complexity</b>: Linear in N if the range [first ,last ) is already sorted using 
   //! comp and otherwise N logN, where N is last - first.
   template <class InputIterator>
   flat_map(InputIterator first, InputIterator last, const Pred& comp = Pred(),
         const allocator_type& a = allocator_type())
      : m_flat_tree(comp, force<impl_allocator_type>(a)) 
      { m_flat_tree.insert_unique(first, last); }

   //! <b>Effects</b>: Copy constructs a flat_map.
   //! 
   //! <b>Complexity</b>: Linear in x.size().
   flat_map(const flat_map<Key,T,Pred,Alloc>& x) 
      : m_flat_tree(x.m_flat_tree) {}

   //! <b>Effects</b>: Move constructs a flat_map.
   //!   Constructs *this using x's resources.
   //! 
   //! <b>Complexity</b>: Construct.
   //! 
   //! <b>Postcondition</b>: x is emptied.
   #ifndef BOOST_INTERPROCESS_RVALUE_REFERENCE
   flat_map(const detail::moved_object<flat_map<Key,T,Pred,Alloc> >& x) 
      : m_flat_tree(detail::move_impl(x.get().m_flat_tree)) {}

   #else
   flat_map(flat_map<Key,T,Pred,Alloc> && x) 
      : m_flat_tree(detail::move_impl(x.m_flat_tree)) {}
   #endif

   //! <b>Effects</b>: Makes *this a copy of x.
   //! 
   //! <b>Complexity</b>: Linear in x.size().
   flat_map<Key,T,Pred,Alloc>& operator=(const flat_map<Key, T, Pred, Alloc>& x)
      {  m_flat_tree = x.m_flat_tree;   return *this;  }

   //! <b>Effects</b>: Move constructs a flat_map.
   //!   Constructs *this using x's resources.
   //! 
   //! <b>Complexity</b>: Construct.
   //! 
   //! <b>Postcondition</b>: x is emptied.
   #ifndef BOOST_INTERPROCESS_RVALUE_REFERENCE
   flat_map<Key,T,Pred,Alloc>& operator=(const detail::moved_object<flat_map<Key, T, Pred, Alloc> >& mx)
      {  m_flat_tree = detail::move_impl(mx.get().m_flat_tree);   return *this;  }
   #else
   flat_map<Key,T,Pred,Alloc>& operator=(flat_map<Key, T, Pred, Alloc> && mx)
      {  m_flat_tree = detail::move_impl(mx.m_flat_tree);   return *this;  }
   #endif

   //! <b>Effects</b>: Returns the comparison object out
   //!   of which a was constructed.
   //! 
   //! <b>Complexity</b>: Constant.
   key_compare key_comp() const 
      { return force<key_compare>(m_flat_tree.key_comp()); }

   //! <b>Effects</b>: Returns an object of value_compare constructed out
   //!   of the comparison object.
   //! 
   //! <b>Complexity</b>: Constant.
   value_compare value_comp() const 
      { return value_compare(force<key_compare>(m_flat_tree.key_comp())); }

   //! <b>Effects</b>: Returns a copy of the Allocator that
   //!   was passed to the object's constructor.
   //! 
   //! <b>Complexity</b>: Constant.
   allocator_type get_allocator() const 
      { return force<allocator_type>(m_flat_tree.get_allocator()); }

   const stored_allocator_type &get_stored_allocator() const 
      { return force<stored_allocator_type>(m_flat_tree.get_stored_allocator()); }

   stored_allocator_type &get_stored_allocator()
      { return force<stored_allocator_type>(m_flat_tree.get_stored_allocator()); }

   //! <b>Effects</b>: Returns an iterator to the first element contained in the container.
   //! 
   //! <b>Throws</b>: Nothing.
   //! 
   //! <b>Complexity</b>: Constant.
   iterator begin() 
      { return force_copy<iterator>(m_flat_tree.begin()); }

   //! <b>Effects</b>: Returns a const_iterator to the first element contained in the container.
   //! 
   //! <b>Throws</b>: Nothing.
   //! 
   //! <b>Complexity</b>: Constant.
   const_iterator begin() const 
      { return force<const_iterator>(m_flat_tree.begin()); }

   //! <b>Effects</b>: Returns a const_iterator to the first element contained in the container.
   //! 
   //! <b>Throws</b>: Nothing.
   //! 
   //! <b>Complexity</b>: Constant.
   const_iterator cbegin() const 
      { return force<const_iterator>(m_flat_tree.cbegin()); }

   //! <b>Effects</b>: Returns an iterator to the end of the container.
   //! 
   //! <b>Throws</b>: Nothing.
   //! 
   //! <b>Complexity</b>: Constant.
   iterator end() 
      { return force_copy<iterator>(m_flat_tree.end()); }

   //! <b>Effects</b>: Returns a const_iterator to the end of the container.
   //! 
   //! <b>Throws</b>: Nothing.
   //! 
   //! <b>Complexity</b>: Constant.
   const_iterator end() const 
      { return force<const_iterator>(m_flat_tree.end()); }

   //! <b>Effects</b>: Returns a const_iterator to the end of the container.
   //! 
   //! <b>Throws</b>: Nothing.
   //! 
   //! <b>Complexity</b>: Constant.
   const_iterator cend() const 
      { return force<const_iterator>(m_flat_tree.cend()); }

   //! <b>Effects</b>: Returns a reverse_iterator pointing to the beginning 
   //! of the reversed container. 
   //! 
   //! <b>Throws</b>: Nothing.
   //! 
   //! <b>Complexity</b>: Constant.
   reverse_iterator rbegin() 
      { return force<reverse_iterator>(m_flat_tree.rbegin()); }

   //! <b>Effects</b>: Returns a const_reverse_iterator pointing to the beginning 
   //! of the reversed container. 
   //! 
   //! <b>Throws</b>: Nothing.
   //! 
   //! <b>Complexity</b>: Constant.
   const_reverse_iterator rbegin() const 
      { return force<const_reverse_iterator>(m_flat_tree.rbegin()); }

   //! <b>Effects</b>: Returns a const_reverse_iterator pointing to the beginning 
   //! of the reversed container. 
   //! 
   //! <b>Throws</b>: Nothing.
   //! 
   //! <b>Complexity</b>: Constant.
   const_reverse_iterator crbegin() const 
      { return force<const_reverse_iterator>(m_flat_tree.crbegin()); }

   //! <b>Effects</b>: Returns a reverse_iterator pointing to the end
   //! of the reversed container. 
   //! 
   //! <b>Throws</b>: Nothing.
   //! 
   //! <b>Complexity</b>: Constant.
   reverse_iterator rend() 
      { return force<reverse_iterator>(m_flat_tree.rend()); }

   //! <b>Effects</b>: Returns a const_reverse_iterator pointing to the end
   //! of the reversed container. 
   //! 
   //! <b>Throws</b>: Nothing.
   //! 
   //! <b>Complexity</b>: Constant.
   const_reverse_iterator rend() const 
      { return force<const_reverse_iterator>(m_flat_tree.rend()); }

   //! <b>Effects</b>: Returns a const_reverse_iterator pointing to the end
   //! of the reversed container. 
   //! 
   //! <b>Throws</b>: Nothing.
   //! 
   //! <b>Complexity</b>: Constant.
   const_reverse_iterator crend() const 
      { return force<const_reverse_iterator>(m_flat_tree.crend()); }

   //! <b>Effects</b>: Returns true if the container contains no elements.
   //! 
   //! <b>Throws</b>: Nothing.
   //! 
   //! <b>Complexity</b>: Constant.
   bool empty() const 
      { return m_flat_tree.empty(); }

   //! <b>Effects</b>: Returns the number of the elements contained in the container.
   //! 
   //! <b>Throws</b>: Nothing.
   //! 
   //! <b>Complexity</b>: Constant.
   size_type size() const 
      { return m_flat_tree.size(); }

   //! <b>Effects</b>: Returns the largest possible size of the container.
   //! 
   //! <b>Throws</b>: Nothing.
   //! 
   //! <b>Complexity</b>: Constant.
   size_type max_size() const 
      { return m_flat_tree.max_size(); }

   //! Effects: If there is no key equivalent to x in the flat_map, inserts 
   //! value_type(detail::move_impl(x), T()) into the flat_map (the key is move-constructed)
   //! 
   //! Returns: A reference to the mapped_type corresponding to x in *this.
   //! 
   //! Complexity: Logarithmic.
   #ifndef BOOST_INTERPROCESS_RVALUE_REFERENCE
   //! Effects: If there is no key equivalent to x in the flat_map, inserts 
   //!   value_type(x, T()) into the flat_map.
   //! 
   //! Returns: A reference to the mapped_type corresponding to x in *this.
   //! 
   //! Complexity: Logarithmic.
   T &operator[](const key_type& k) 
   {
      iterator i = lower_bound(k);
      // i->first is greater than or equivalent to k.
      if (i == end() || key_comp()(k, (*i).first))
         i = insert(i, value_type(k, T()));
      return (*i).second;
   }

   T &operator[](const detail::moved_object<key_type>& mk) 
   {
      key_type &k = mk.get();
      iterator i = lower_bound(k);
      // i->first is greater than or equivalent to k.
      if (i == end() || key_comp()(k, (*i).first))
         i = insert(i, value_type(k, detail::move_impl(T())));
      return (*i).second;
   }
   #else
   //! Effects: If there is no key equivalent to x in the flat_map, inserts 
   //!   value_type(x, T()) into the flat_map.
   //! 
   //! Returns: A reference to the mapped_type corresponding to x in *this.
   //! 
   //! Complexity: Logarithmic.
   T &operator[](key_type &&mk) 
   {
      key_type &k = mk;
      iterator i = lower_bound(k);
      // i->first is greater than or equivalent to k.
      if (i == end() || key_comp()(k, (*i).first))
         i = insert(i, value_type(detail::forward_impl<key_type>(k), detail::move_impl(T())));
      return (*i).second;
   }
   #endif

   //! <b>Effects</b>: Swaps the contents of *this and x.
   //!   If this->allocator_type() != x.allocator_type() allocators are also swapped.
   //!
   //! <b>Throws</b>: Nothing.
   //!
   //! <b>Complexity</b>: Constant.
   void swap(flat_map<Key,T,Pred,Alloc>& x) 
      { m_flat_tree.swap(x.m_flat_tree); }

   //! <b>Effects</b>: Swaps the contents of *this and x.
   //!   If this->allocator_type() != x.allocator_type() allocators are also swapped.
   //!
   //! <b>Throws</b>: Nothing.
   //!
   //! <b>Complexity</b>: Constant.
   #ifndef BOOST_INTERPROCESS_RVALUE_REFERENCE
   void swap(const detail::moved_object<flat_map<Key,T,Pred,Alloc> >& x) 
      { m_flat_tree.swap(x.get().m_flat_tree); }
   #else
   void swap(flat_map<Key,T,Pred,Alloc> && x) 
      { m_flat_tree.swap(x.m_flat_tree); }
   #endif

   //! <b>Effects</b>: Inserts x if and only if there is no element in the container 
   //!   with key equivalent to the key of x.
   //!
   //! <b>Returns</b>: The bool component of the returned pair is true if and only 
   //!   if the insertion takes place, and the iterator component of the pair
   //!   points to the element with key equivalent to the key of x.
   //!
   //! <b>Complexity</b>: Logarithmic search time plus linear insertion
   //!   to the elements with bigger keys than x.
   //!
   //! <b>Note</b>: If an element it's inserted it might invalidate elements.
   std::pair<iterator,bool> insert(const value_type& x) 
      { return force<std::pair<iterator,bool> >(
         m_flat_tree.insert_unique(force<impl_value_type>(x))); }

   //! <b>Effects</b>: Inserts a new value_type move constructed from the pair if and
   //! only if there is no element in the container with key equivalent to the key of x.
   //!
   //! <b>Returns</b>: The bool component of the returned pair is true if and only 
   //!   if the insertion takes place, and the iterator component of the pair
   //!   points to the element with key equivalent to the key of x.
   //!
   //! <b>Complexity</b>: Logarithmic search time plus linear insertion
   //!   to the elements with bigger keys than x.
   //!
   //! <b>Note</b>: If an element it's inserted it might invalidate elements.
   #ifndef BOOST_INTERPROCESS_RVALUE_REFERENCE
   std::pair<iterator,bool> insert(const detail::moved_object<value_type>& x) 
      { return force<std::pair<iterator,bool> >(
         m_flat_tree.insert_unique(force<impl_moved_value_type>(x))); }
   #else
   std::pair<iterator,bool> insert(value_type &&x) 
   {  return force<std::pair<iterator,bool> >(
      m_flat_tree.insert_unique(detail::move_impl(force<impl_value_type>(x)))); }
   #endif

   //! <b>Effects</b>: Inserts a copy of x in the container if and only if there is 
   //!   no element in the container with key equivalent to the key of x.
   //!   p is a hint pointing to where the insert should start to search.
   //!
   //! <b>Returns</b>: An iterator pointing to the element with key equivalent
   //!   to the key of x.
   //!
   //! <b>Complexity</b>: Logarithmic search time (constant if x is inserted
   //!   right before p) plus insertion linear to the elements with bigger keys than x.
   //!
   //! <b>Note</b>: If an element it's inserted it might invalidate elements.
   iterator insert(const_iterator position, const value_type& x)
      { return force_copy<iterator>(
         m_flat_tree.insert_unique(force<impl_const_iterator>(position), force<impl_value_type>(x))); }

   //! <b>Effects</b>: Inserts an element move constructed from x in the container.
   //!   p is a hint pointing to where the insert should start to search.
   //!
   //! <b>Returns</b>: An iterator pointing to the element with key equivalent to the key of x.
   //!
   //! <b>Complexity</b>: Logarithmic search time (constant if x is inserted
   //!   right before p) plus insertion linear to the elements with bigger keys than x.
   //!
   //! <b>Note</b>: If an element it's inserted it might invalidate elements.
   #ifndef BOOST_INTERPROCESS_RVALUE_REFERENCE
   iterator insert(const_iterator position, const detail::moved_object<value_type>& x)
      { return force_copy<iterator>(
         m_flat_tree.insert_unique(force<impl_const_iterator>(position), force<impl_moved_value_type>(x))); }
   #else
   iterator insert(const_iterator position, value_type &&x)
      { return force_copy<iterator>(
         m_flat_tree.insert_unique(force<impl_const_iterator>(position), detail::move_impl(force<impl_value_type>(x)))); }
   #endif

   //! <b>Requires</b>: i, j are not iterators into *this.
   //!
   //! <b>Effects</b>: inserts each element from the range [i,j) if and only 
   //!   if there is no element with key equivalent to the key of that element.
   //!
   //! <b>Complexity</b>: N log(size()+N) (N is the distance from i to j)
   //!   search time plus N*size() insertion time.
   //!
   //! <b>Note</b>: If an element it's inserted it might invalidate elements.
   template <class InputIterator>
   void insert(InputIterator first, InputIterator last) 
   {  m_flat_tree.insert_unique(first, last);  }

   #ifdef BOOST_INTERPROCESS_PERFECT_FORWARDING

   //! <b>Effects</b>: Inserts an object of type T constructed with
   //!   std::forward<Args>(args)... if and only if there is no element in the container 
   //!   with key equivalent to the key of x.
   //!
   //! <b>Returns</b>: The bool component of the returned pair is true if and only 
   //!   if the insertion takes place, and the iterator component of the pair
   //!   points to the element with key equivalent to the key of x.
   //!
   //! <b>Complexity</b>: Logarithmic search time plus linear insertion
   //!   to the elements with bigger keys than x.
   //!
   //! <b>Note</b>: If an element it's inserted it might invalidate elements.
   template <class... Args>
   iterator emplace(Args&&... args)
   {  return force_copy<iterator>(m_flat_tree.emplace_unique(detail::forward_impl<Args>(args)...)); }

   //! <b>Effects</b>: Inserts an object of type T constructed with
   //!   std::forward<Args>(args)... in the container if and only if there is 
   //!   no element in the container with key equivalent to the key of x.
   //!   p is a hint pointing to where the insert should start to search.
   //!
   //! <b>Returns</b>: An iterator pointing to the element with key equivalent
   //!   to the key of x.
   //!
   //! <b>Complexity</b>: Logarithmic search time (constant if x is inserted
   //!   right before p) plus insertion linear to the elements with bigger keys than x.
   //!
   //! <b>Note</b>: If an element it's inserted it might invalidate elements.
   template <class... Args>
   iterator emplace_hint(const_iterator hint, Args&&... args)
   {  return force_copy<iterator>(m_flat_tree.emplace_hint_unique(force<impl_const_iterator>(hint), detail::forward_impl<Args>(args)...)); }

   #else //#ifdef BOOST_INTERPROCESS_PERFECT_FORWARDING

   iterator emplace()
   {  return force_copy<iterator>(m_flat_tree.emplace_unique()); }

   iterator emplace_hint(const_iterator hint)
   {  return force_copy<iterator>(m_flat_tree.emplace_hint_unique(force<impl_const_iterator>(hint))); }

   #define BOOST_PP_LOCAL_MACRO(n)                                                                    \
   template<BOOST_PP_ENUM_PARAMS(n, class P)>                                                         \
   iterator emplace(BOOST_PP_ENUM(n, BOOST_INTERPROCESS_PP_PARAM_LIST, _))                            \
   {                                                                                                  \
      return force_copy<iterator>(m_flat_tree.emplace_unique                                               \
         (BOOST_PP_ENUM(n, BOOST_INTERPROCESS_PP_PARAM_FORWARD, _)));                                 \
   }                                                                                                  \
                                                                                                      \
   template<BOOST_PP_ENUM_PARAMS(n, class P)>                                                         \
   iterator emplace_hint(const_iterator hint, BOOST_PP_ENUM(n, BOOST_INTERPROCESS_PP_PARAM_LIST, _))  \
   {                                                                                                  \
      return force_copy<iterator>(m_flat_tree.emplace_hint_unique                                          \
         (force<impl_const_iterator>(hint),                                                           \
         BOOST_PP_ENUM(n, BOOST_INTERPROCESS_PP_PARAM_FORWARD, _)));                                  \
   }                                                                                                  \
   //!
   #define BOOST_PP_LOCAL_LIMITS (1, BOOST_INTERPROCESS_MAX_CONSTRUCTOR_PARAMETERS)
   #include BOOST_PP_LOCAL_ITERATE()

   #endif   //#ifdef BOOST_INTERPROCESS_PERFECT_FORWARDING

   //! <b>Effects</b>: Erases the element pointed to by position.
   //!
   //! <b>Returns</b>: Returns an iterator pointing to the element immediately
   //!   following q prior to the element being erased. If no such element exists, 
   //!   returns end().
   //!
   //! <b>Complexity</b>: Linear to the elements with keys bigger than position
   //!
   //! <b>Note</b>: Invalidates elements with keys
   //!   not less than the erased element.
   iterator erase(const_iterator position) 
      { return force_copy<iterator>(m_flat_tree.erase(force<impl_const_iterator>(position))); }

   //! <b>Effects</b>: Erases all elements in the container with key equivalent to x.
   //!
   //! <b>Returns</b>: Returns the number of erased elements.
   //!
   //! <b>Complexity</b>: Logarithmic search time plus erasure time
   //!   linear to the elements with bigger keys.
   size_type erase(const key_type& x) 
      { return m_flat_tree.erase(x); }

   //! <b>Effects</b>: Erases all the elements in the range [first, last).
   //!
   //! <b>Returns</b>: Returns last.
   //!
   //! <b>Complexity</b>: size()*N where N is the distance from first to last.
   //!
   //! <b>Complexity</b>: Logarithmic search time plus erasure time
   //!   linear to the elements with bigger keys.
   iterator erase(const_iterator first, const_iterator last)
      { return force_copy<iterator>(m_flat_tree.erase(force<impl_const_iterator>(first), force<impl_const_iterator>(last))); }

   //! <b>Effects</b>: erase(a.begin(),a.end()).
   //!
   //! <b>Postcondition</b>: size() == 0.
   //!
   //! <b>Complexity</b>: linear in size().
   void clear() 
      { m_flat_tree.clear(); }

   //! <b>Effects</b>: Tries to deallocate the excess of memory created
   //    with previous allocations. The size of the vector is unchanged
   //!
   //! <b>Throws</b>: If memory allocation throws, or T's copy constructor throws.
   //!
   //! <b>Complexity</b>: Linear to size().
   void shrink_to_fit()
      { m_flat_tree.shrink_to_fit(); }

   //! <b>Returns</b>: An iterator pointing to an element with the key
   //!   equivalent to x, or end() if such an element is not found.
   //!
   //! <b>Complexity</b>: Logarithmic.
   iterator find(const key_type& x) 
      { return force_copy<iterator>(m_flat_tree.find(x)); }

   //! <b>Returns</b>: A const_iterator pointing to an element with the key
   //!   equivalent to x, or end() if such an element is not found.
   //!
   //! <b>Complexity</b>: Logarithmic.s
   const_iterator find(const key_type& x) const 
      { return force<const_iterator>(m_flat_tree.find(x)); }

   //! <b>Returns</b>: The number of elements with key equivalent to x.
   //!
   //! <b>Complexity</b>: log(size())+count(k)
   size_type count(const key_type& x) const 
      {  return m_flat_tree.find(x) == m_flat_tree.end() ? 0 : 1;  }

   //! <b>Returns</b>: An iterator pointing to the first element with key not less
   //!   than k, or a.end() if such an element is not found.
   //!
   //! <b>Complexity</b>: Logarithmic
   iterator lower_bound(const key_type& x) 
      {  return force_copy<iterator>(m_flat_tree.lower_bound(x)); }

   //! <b>Returns</b>: A const iterator pointing to the first element with key not
   //!   less than k, or a.end() if such an element is not found.
   //!
   //! <b>Complexity</b>: Logarithmic
   const_iterator lower_bound(const key_type& x) const 
      {  return force<const_iterator>(m_flat_tree.lower_bound(x)); }

   //! <b>Returns</b>: An iterator pointing to the first element with key not less
   //!   than x, or end() if such an element is not found.
   //!
   //! <b>Complexity</b>: Logarithmic
   iterator upper_bound(const key_type& x) 
      {  return force_copy<iterator>(m_flat_tree.upper_bound(x)); }

   //! <b>Returns</b>: A const iterator pointing to the first element with key not
   //!   less than x, or end() if such an element is not found.
   //!
   //! <b>Complexity</b>: Logarithmic
   const_iterator upper_bound(const key_type& x) const 
      {  return force<const_iterator>(m_flat_tree.upper_bound(x)); }

   //! <b>Effects</b>: Equivalent to std::make_pair(this->lower_bound(k), this->upper_bound(k)).
   //!
   //! <b>Complexity</b>: Logarithmic
   std::pair<iterator,iterator> equal_range(const key_type& x) 
      {  return force<std::pair<iterator,iterator> >(m_flat_tree.equal_range(x)); }

   //! <b>Effects</b>: Equivalent to std::make_pair(this->lower_bound(k), this->upper_bound(k)).
   //!
   //! <b>Complexity</b>: Logarithmic
   std::pair<const_iterator,const_iterator> equal_range(const key_type& x) const 
      {  return force<std::pair<const_iterator,const_iterator> >(m_flat_tree.equal_range(x)); }

   //! <b>Effects</b>: Number of elements for which memory has been allocated.
   //!   capacity() is always greater than or equal to size().
   //! 
   //! <b>Throws</b>: Nothing.
   //! 
   //! <b>Complexity</b>: Constant.
   size_type capacity() const           
      { return m_flat_tree.capacity(); }

   //! <b>Effects</b>: If n is less than or equal to capacity(), this call has no
   //!   effect. Otherwise, it is a request for allocation of additional memory.
   //!   If the request is successful, then capacity() is greater than or equal to
   //!   n; otherwise, capacity() is unchanged. In either case, size() is unchanged.
   //! 
   //! <b>Throws</b>: If memory allocation allocation throws or T's copy constructor throws.
   //!
   //! <b>Note</b>: If capacity() is less than "count", iterators and references to
   //!   to values might be invalidated.
   void reserve(size_type count)       
      { m_flat_tree.reserve(count);   }

   /// @cond
   template <class K1, class T1, class C1, class A1>
   friend bool operator== (const flat_map<K1, T1, C1, A1>&,
                           const flat_map<K1, T1, C1, A1>&);
   template <class K1, class T1, class C1, class A1>
   friend bool operator< (const flat_map<K1, T1, C1, A1>&,
                           const flat_map<K1, T1, C1, A1>&);
   /// @endcond
};

template <class Key, class T, class Pred, class Alloc>
inline bool operator==(const flat_map<Key,T,Pred,Alloc>& x, 
                       const flat_map<Key,T,Pred,Alloc>& y) 
   {  return x.m_flat_tree == y.m_flat_tree;  }

template <class Key, class T, class Pred, class Alloc>
inline bool operator<(const flat_map<Key,T,Pred,Alloc>& x, 
                      const flat_map<Key,T,Pred,Alloc>& y) 
   {  return x.m_flat_tree < y.m_flat_tree;   }

template <class Key, class T, class Pred, class Alloc>
inline bool operator!=(const flat_map<Key,T,Pred,Alloc>& x, 
                       const flat_map<Key,T,Pred,Alloc>& y) 
   {  return !(x == y); }

template <class Key, class T, class Pred, class Alloc>
inline bool operator>(const flat_map<Key,T,Pred,Alloc>& x, 
                      const flat_map<Key,T,Pred,Alloc>& y) 
   {  return y < x;  }

template <class Key, class T, class Pred, class Alloc>
inline bool operator<=(const flat_map<Key,T,Pred,Alloc>& x, 
                       const flat_map<Key,T,Pred,Alloc>& y) 
   {  return !(y < x);  }

template <class Key, class T, class Pred, class Alloc>
inline bool operator>=(const flat_map<Key,T,Pred,Alloc>& x, 
                       const flat_map<Key,T,Pred,Alloc>& y) 
   {  return !(x < y);  }

#ifndef BOOST_INTERPROCESS_RVALUE_REFERENCE
template <class Key, class T, class Pred, class Alloc>
inline void swap(flat_map<Key,T,Pred,Alloc>& x, 
                 flat_map<Key,T,Pred,Alloc>& y) 
   {  x.swap(y);  }

template <class Key, class T, class Pred, class Alloc>
inline void swap(const detail::moved_object<flat_map<Key,T,Pred,Alloc> >& x, 
                 flat_map<Key,T,Pred,Alloc>& y) 
   {  x.get().swap(y);  }

template <class Key, class T, class Pred, class Alloc>
inline void swap(flat_map<Key,T,Pred,Alloc>& x, 
                 const detail::moved_object<flat_map<Key,T,Pred,Alloc> >& y) 
   {  x.swap(y.get());  }
#else
template <class Key, class T, class Pred, class Alloc>
inline void swap(flat_map<Key,T,Pred,Alloc>&&x, 
                 flat_map<Key,T,Pred,Alloc>&&y) 
   {  x.swap(y);  }
#endif

/// @cond

//!This class is movable
template <class K, class T, class C, class A>
struct is_movable<flat_map<K, T, C, A> >
{
   enum {   value = true };
};

//!has_trivial_destructor_after_move<> == true_type
//!specialization for optimizations
template <class K, class T, class C, class A>
struct has_trivial_destructor_after_move<flat_map<K, T, C, A> >
{
   enum {   value = 
               has_trivial_destructor<A>::value &&
               has_trivial_destructor<C>::value  };
};

// Forward declaration of operators < and ==, needed for friend declaration.
template <class Key, class T, 
          class Pred,
          class Alloc>
class flat_multimap;

template <class Key, class T, class Pred, class Alloc>
inline bool operator==(const flat_multimap<Key,T,Pred,Alloc>& x, 
                       const flat_multimap<Key,T,Pred,Alloc>& y);

template <class Key, class T, class Pred, class Alloc>
inline bool operator<(const flat_multimap<Key,T,Pred,Alloc>& x, 
                      const flat_multimap<Key,T,Pred,Alloc>& y);
/// @endcond

//! A flat_multimap is a kind of associative container that supports equivalent keys 
//! (possibly containing multiple copies of the same key value) and provides for 
//! fast retrieval of values of another type T based on the keys. The flat_multimap 
//! class supports random-access iterators.
//! 
//! A flat_multimap satisfies all of the requirements of a container and of a reversible 
//! container and of an associative container. For a 
//! flat_multimap<Key,T> the key_type is Key and the value_type is std::pair<Key,T>
//! (unlike std::multimap<Key, T> which value_type is std::pair<<b>const</b> Key, T>).
//!
//! Pred is the ordering function for Keys (e.g. <i>std::less<Key></i>).
//!
//! Alloc is the allocator to allocate the value_types
//! (e.g. <i>boost::interprocess:allocator< std::pair<Key, T></i>).
template <class Key, class T, class Pred, class Alloc>
class flat_multimap 
{
   /// @cond
   private:
   typedef detail::flat_tree<Key, 
                           std::pair<Key, T>, 
                           detail::select1st< std::pair<Key, T> >, 
                           Pred, 
                           Alloc> tree_t;
   //#ifndef BOOST_INTERPROCESS_RVALUE_REFERENCE
   //This is the real tree stored here. It's based on a movable pair
   typedef detail::flat_tree<Key, 
                           detail::pair<Key, T>, 
                           detail::select1st< detail::pair<Key, T> >, 
                           Pred, 
                           typename Alloc::template
                              rebind<detail::pair<Key, T> >::other> impl_tree_t;
/*
   #else
   typedef tree_t    impl_tree_t;
   #endif   
*/
   impl_tree_t m_flat_tree;  // flat tree representing flat_map

   typedef typename impl_tree_t::value_type             impl_value_type;
   typedef typename impl_tree_t::pointer                impl_pointer;
   typedef typename impl_tree_t::const_pointer          impl_const_pointer;
   typedef typename impl_tree_t::reference              impl_reference;
   typedef typename impl_tree_t::const_reference        impl_const_reference;
   typedef typename impl_tree_t::value_compare          impl_value_compare;
   typedef typename impl_tree_t::iterator               impl_iterator;
   typedef typename impl_tree_t::const_iterator         impl_const_iterator;
   typedef typename impl_tree_t::reverse_iterator       impl_reverse_iterator;
   typedef typename impl_tree_t::const_reverse_iterator impl_const_reverse_iterator;
   typedef typename impl_tree_t::allocator_type         impl_allocator_type;
   #ifndef BOOST_INTERPROCESS_RVALUE_REFERENCE
   typedef detail::moved_object<impl_value_type>        impl_moved_value_type;
   #endif

   //#ifndef BOOST_INTERPROCESS_RVALUE_REFERENCE
   template<class D, class S>
   static D &force(const S &s)
   {  return *const_cast<D*>((reinterpret_cast<const D*>(&s))); }

   template<class D, class S>
   static D force_copy(S s)
   {
      value_type *vp = reinterpret_cast<value_type *>(&*s);
      return D(vp);
   }
   /// @endcond

   public:
   // typedefs:
   typedef typename tree_t::key_type               key_type;
   typedef typename tree_t::value_type             value_type;
   typedef typename tree_t::pointer                pointer;
   typedef typename tree_t::const_pointer          const_pointer;
   typedef typename tree_t::reference              reference;
   typedef typename tree_t::const_reference        const_reference;
   typedef typename tree_t::value_compare          value_compare;
   typedef T                                       mapped_type;
   typedef typename tree_t::key_compare            key_compare;
   typedef typename tree_t::iterator               iterator;
   typedef typename tree_t::const_iterator         const_iterator;
   typedef typename tree_t::reverse_iterator       reverse_iterator;
   typedef typename tree_t::const_reverse_iterator const_reverse_iterator;
   typedef typename tree_t::size_type              size_type;
   typedef typename tree_t::difference_type        difference_type;
   typedef typename tree_t::allocator_type         allocator_type;
   typedef typename tree_t::stored_allocator_type  stored_allocator_type;

   //! <b>Effects</b>: Constructs an empty flat_multimap using the specified comparison
   //!   object and allocator.
   //! 
   //! <b>Complexity</b>: Constant.
   explicit flat_multimap(const Pred& comp = Pred(),
                          const allocator_type& a = allocator_type())
      : m_flat_tree(comp, force<impl_allocator_type>(a)) { }

   //! <b>Effects</b>: Constructs an empty flat_multimap using the specified comparison object
   //!   and allocator, and inserts elements from the range [first ,last ).
   //! 
   //! <b>Complexity</b>: Linear in N if the range [first ,last ) is already sorted using 
   //! comp and otherwise N logN, where N is last - first.
   template <class InputIterator>
   flat_multimap(InputIterator first, InputIterator last,
            const Pred& comp        = Pred(),
            const allocator_type& a = allocator_type())
      : m_flat_tree(comp, force<impl_allocator_type>(a)) 
      { m_flat_tree.insert_equal(first, last); }

   //! <b>Effects</b>: Copy constructs a flat_multimap.
   //! 
   //! <b>Complexity</b>: Linear in x.size().
   flat_multimap(const flat_multimap<Key,T,Pred,Alloc>& x) 
      : m_flat_tree(x.m_flat_tree) { }

   //! <b>Effects</b>: Move constructs a flat_multimap. Constructs *this using x's resources.
   //! 
   //! <b>Complexity</b>: Construct.
   //! 
   //! <b>Postcondition</b>: x is emptied.
   #ifndef BOOST_INTERPROCESS_RVALUE_REFERENCE
   flat_multimap(const detail::moved_object<flat_multimap<Key,T,Pred,Alloc> >& x) 
      : m_flat_tree(detail::move_impl(x.get().m_flat_tree)) { }
   #else
   flat_multimap(flat_multimap<Key,T,Pred,Alloc> && x) 
      : m_flat_tree(detail::move_impl(x.m_flat_tree)) { }
   #endif

   //! <b>Effects</b>: Makes *this a copy of x.
   //! 
   //! <b>Complexity</b>: Linear in x.size().
   flat_multimap<Key,T,Pred,Alloc>&
   operator=(const flat_multimap<Key,T,Pred,Alloc>& x) 
      {  m_flat_tree = x.m_flat_tree;   return *this;  }

   //! <b>Effects</b>: this->swap(x.get()).
   //! 
   //! <b>Complexity</b>: Constant.
   #ifndef BOOST_INTERPROCESS_RVALUE_REFERENCE
   flat_multimap<Key,T,Pred,Alloc>&
   operator=(const detail::moved_object<flat_multimap<Key,T,Pred,Alloc> >& mx) 
      {  m_flat_tree = detail::move_impl(mx.get().m_flat_tree);   return *this;  }
   #else
   flat_multimap<Key,T,Pred,Alloc>&
   operator=(flat_multimap<Key,T,Pred,Alloc> && mx) 
      {  m_flat_tree = detail::move_impl(mx.m_flat_tree);   return *this;  }
   #endif

   //! <b>Effects</b>: Returns the comparison object out
   //!   of which a was constructed.
   //! 
   //! <b>Complexity</b>: Constant.
   key_compare key_comp() const 
      { return force<key_compare>(m_flat_tree.key_comp()); }

   //! <b>Effects</b>: Returns an object of value_compare constructed out
   //!   of the comparison object.
   //! 
   //! <b>Complexity</b>: Constant.
   value_compare value_comp() const 
      { return value_compare(force<key_compare>(m_flat_tree.key_comp())); }

   //! <b>Effects</b>: Returns a copy of the Allocator that
   //!   was passed to the object's constructor.
   //! 
   //! <b>Complexity</b>: Constant.
   allocator_type get_allocator() const 
      { return force<allocator_type>(m_flat_tree.get_allocator()); }

   const stored_allocator_type &get_stored_allocator() const 
      { return force<stored_allocator_type>(m_flat_tree.get_stored_allocator()); }

   stored_allocator_type &get_stored_allocator()
      { return force<stored_allocator_type>(m_flat_tree.get_stored_allocator()); }

   //! <b>Effects</b>: Returns an iterator to the first element contained in the container.
   //! 
   //! <b>Throws</b>: Nothing.
   //! 
   //! <b>Complexity</b>: Constant.
   iterator begin() 
      { return force_copy<iterator>(m_flat_tree.begin()); }

   //! <b>Effects</b>: Returns a const_iterator to the first element contained in the container.
   //! 
   //! <b>Throws</b>: Nothing.
   //! 
   //! <b>Complexity</b>: Constant.
   const_iterator begin() const 
      { return force<const_iterator>(m_flat_tree.begin()); }

   //! <b>Effects</b>: Returns an iterator to the end of the container.
   //! 
   //! <b>Throws</b>: Nothing.
   //! 
   //! <b>Complexity</b>: Constant.
   iterator end() 
      { return force_copy<iterator>(m_flat_tree.end()); }

   //! <b>Effects</b>: Returns a const_iterator to the end of the container.
   //! 
   //! <b>Throws</b>: Nothing.
   //! 
   //! <b>Complexity</b>: Constant.
   const_iterator end() const 
      { return force<const_iterator>(m_flat_tree.end()); }

   //! <b>Effects</b>: Returns a reverse_iterator pointing to the beginning 
   //! of the reversed container. 
   //! 
   //! <b>Throws</b>: Nothing.
   //! 
   //! <b>Complexity</b>: Constant.
   reverse_iterator rbegin() 
      { return force<reverse_iterator>(m_flat_tree.rbegin()); }

   //! <b>Effects</b>: Returns a const_reverse_iterator pointing to the beginning 
   //! of the reversed container. 
   //! 
   //! <b>Throws</b>: Nothing.
   //! 
   //! <b>Complexity</b>: Constant.
   const_reverse_iterator rbegin() const 
      { return force<const_reverse_iterator>(m_flat_tree.rbegin()); }

   //! <b>Effects</b>: Returns a reverse_iterator pointing to the end
   //! of the reversed container. 
   //! 
   //! <b>Throws</b>: Nothing.
   //! 
   //! <b>Complexity</b>: Constant.
   reverse_iterator rend() 
      { return force<reverse_iterator>(m_flat_tree.rend()); }

   //! <b>Effects</b>: Returns a const_reverse_iterator pointing to the end
   //! of the reversed container. 
   //! 
   //! <b>Throws</b>: Nothing.
   //! 
   //! <b>Complexity</b>: Constant.
   const_reverse_iterator rend() const 
      { return force<const_reverse_iterator>(m_flat_tree.rend()); }

   //! <b>Effects</b>: Returns true if the container contains no elements.
   //! 
   //! <b>Throws</b>: Nothing.
   //! 
   //! <b>Complexity</b>: Constant.
   bool empty() const 
      { return m_flat_tree.empty(); }

   //! <b>Effects</b>: Returns the number of the elements contained in the container.
   //! 
   //! <b>Throws</b>: Nothing.
   //! 
   //! <b>Complexity</b>: Constant.
   size_type size() const 
      { return m_flat_tree.size(); }

   //! <b>Effects</b>: Returns the largest possible size of the container.
   //! 
   //! <b>Throws</b>: Nothing.
   //! 
   //! <b>Complexity</b>: Constant.
   size_type max_size() const 
      { return m_flat_tree.max_size(); }

   //! <b>Effects</b>: Swaps the contents of *this and x.
   //!   If this->allocator_type() != x.allocator_type() allocators are also swapped.
   //!
   //! <b>Throws</b>: Nothing.
   //!
   //! <b>Complexity</b>: Constant.
   void swap(flat_multimap<Key,T,Pred,Alloc>& x) 
      { m_flat_tree.swap(x.m_flat_tree); }

   //! <b>Effects</b>: Swaps the contents of *this and x.
   //!   If this->allocator_type() != x.allocator_type() allocators are also swapped.
   //!
   //! <b>Throws</b>: Nothing.
   //!
   //! <b>Complexity</b>: Constant.
   #ifndef BOOST_INTERPROCESS_RVALUE_REFERENCE
   void swap(const detail::moved_object<flat_multimap<Key,T,Pred,Alloc> >& x) 
      { m_flat_tree.swap(x.get().m_flat_tree); }
   #else
   void swap(flat_multimap<Key,T,Pred,Alloc> && x) 
      { m_flat_tree.swap(x.m_flat_tree); }
   #endif

   //! <b>Effects</b>: Inserts x and returns the iterator pointing to the
   //!   newly inserted element. 
   //!
   //! <b>Complexity</b>: Logarithmic search time plus linear insertion
   //!   to the elements with bigger keys than x.
   //!
   //! <b>Note</b>: If an element it's inserted it might invalidate elements.
   iterator insert(const value_type& x) 
      { return force_copy<iterator>(m_flat_tree.insert_equal(force<impl_value_type>(x))); }

   //! <b>Effects</b>: Inserts a new value move-constructed from x and returns 
   //!   the iterator pointing to the newly inserted element. 
   //!
   //! <b>Complexity</b>: Logarithmic search time plus linear insertion
   //!   to the elements with bigger keys than x.
   //!
   //! <b>Note</b>: If an element it's inserted it might invalidate elements.
   #ifndef BOOST_INTERPROCESS_RVALUE_REFERENCE
   iterator insert(const detail::moved_object<value_type>& x) 
      { return force_copy<iterator>(m_flat_tree.insert_equal(force<impl_moved_value_type>(x))); }
   #else
   iterator insert(value_type &&x) 
      { return force_copy<iterator>(m_flat_tree.insert_equal(detail::move_impl(x))); }
   #endif

   //! <b>Effects</b>: Inserts a copy of x in the container.
   //!   p is a hint pointing to where the insert should start to search.
   //!
   //! <b>Returns</b>: An iterator pointing to the element with key equivalent
   //!   to the key of x.
   //!
   //! <b>Complexity</b>: Logarithmic search time (constant time if the value
   //!   is to be inserted before p) plus linear insertion
   //!   to the elements with bigger keys than x.
   //!
   //! <b>Note</b>: If an element it's inserted it might invalidate elements.
   iterator insert(const_iterator position, const value_type& x) 
      { return force_copy<iterator>(m_flat_tree.insert_equal(force<impl_const_iterator>(position), force<impl_value_type>(x))); }

   //! <b>Effects</b>: Inserts a value move constructed from x in the container.
   //!   p is a hint pointing to where the insert should start to search.
   //!
   //! <b>Returns</b>: An iterator pointing to the element with key equivalent
   //!   to the key of x.
   //!
   //! <b>Complexity</b>: Logarithmic search time (constant time if the value
   //!   is to be inserted before p) plus linear insertion
   //!   to the elements with bigger keys than x.
   //!
   //! <b>Note</b>: If an element it's inserted it might invalidate elements.
   #ifndef BOOST_INTERPROCESS_RVALUE_REFERENCE
   iterator insert(const_iterator position, const detail::moved_object<value_type>& x) 
      { return force_copy<iterator>(m_flat_tree.insert_equal(force<impl_const_iterator>(position), force<impl_moved_value_type>(x))); }
   #else
   iterator insert(const_iterator position, value_type &&x) 
      { return force_copy<iterator>(m_flat_tree.insert_equal(force<impl_const_iterator>(position), detail::move_impl(x))); }
   #endif

   //! <b>Requires</b>: i, j are not iterators into *this.
   //!
   //! <b>Effects</b>: inserts each element from the range [i,j) .
   //!
   //! <b>Complexity</b>: N log(size()+N) (N is the distance from i to j)
   //!   search time plus N*size() insertion time.
   //!
   //! <b>Note</b>: If an element it's inserted it might invalidate elements.
   template <class InputIterator>
   void insert(InputIterator first, InputIterator last) 
      {  m_flat_tree.insert_equal(first, last); }

   #ifdef BOOST_INTERPROCESS_PERFECT_FORWARDING

   //! <b>Effects</b>: Inserts an object of type T constructed with
   //!   std::forward<Args>(args)... and returns the iterator pointing to the
   //!   newly inserted element. 
   //!
   //! <b>Complexity</b>: Logarithmic search time plus linear insertion
   //!   to the elements with bigger keys than x.
   //!
   //! <b>Note</b>: If an element it's inserted it might invalidate elements.
   template <class... Args>
   iterator emplace(Args&&... args)
   {  return force_copy<iterator>(m_flat_tree.emplace_equal(detail::forward_impl<Args>(args)...)); }

   //! <b>Effects</b>: Inserts an object of type T constructed with
   //!   std::forward<Args>(args)... in the container.
   //!   p is a hint pointing to where the insert should start to search.
   //!
   //! <b>Returns</b>: An iterator pointing to the element with key equivalent
   //!   to the key of x.
   //!
   //! <b>Complexity</b>: Logarithmic search time (constant time if the value
   //!   is to be inserted before p) plus linear insertion
   //!   to the elements with bigger keys than x.
   //!
   //! <b>Note</b>: If an element it's inserted it might invalidate elements.
   template <class... Args>
   iterator emplace_hint(const_iterator hint, Args&&... args)
   {
      return force_copy<iterator>(m_flat_tree.emplace_hint_equal
         (force<impl_const_iterator>(hint), detail::forward_impl<Args>(args)...));
   }

   #else //#ifdef BOOST_INTERPROCESS_PERFECT_FORWARDING

   iterator emplace()
   {  return force_copy<iterator>(m_flat_tree.emplace_equal()); }

   iterator emplace_hint(const_iterator hint)
   {  return force_copy<iterator>(m_flat_tree.emplace_hint_equal(force<impl_const_iterator>(hint))); }

   #define BOOST_PP_LOCAL_MACRO(n)                                                                    \
   template<BOOST_PP_ENUM_PARAMS(n, class P)>                                                         \
   iterator emplace(BOOST_PP_ENUM(n, BOOST_INTERPROCESS_PP_PARAM_LIST, _))                            \
   {                                                                                                  \
      return force_copy<iterator>(m_flat_tree.emplace_equal                                                \
         (BOOST_PP_ENUM(n, BOOST_INTERPROCESS_PP_PARAM_FORWARD, _)));                                 \
   }                                                                                                  \
                                                                                                      \
   template<BOOST_PP_ENUM_PARAMS(n, class P)>                                                         \
   iterator emplace_hint(const_iterator hint, BOOST_PP_ENUM(n, BOOST_INTERPROCESS_PP_PARAM_LIST, _))  \
   {                                                                                                  \
      return force_copy<iterator>(m_flat_tree.emplace_hint_equal                                           \
         (force<impl_const_iterator>(hint),                                                           \
            BOOST_PP_ENUM(n, BOOST_INTERPROCESS_PP_PARAM_FORWARD, _)));                               \
   }                                                                                                  \
   //!
   #define BOOST_PP_LOCAL_LIMITS (1, BOOST_INTERPROCESS_MAX_CONSTRUCTOR_PARAMETERS)
   #include BOOST_PP_LOCAL_ITERATE()

   #endif   //#ifdef BOOST_INTERPROCESS_PERFECT_FORWARDING

   //! <b>Effects</b>: Erases the element pointed to by position.
   //!
   //! <b>Returns</b>: Returns an iterator pointing to the element immediately
   //!   following q prior to the element being erased. If no such element exists, 
   //!   returns end().
   //!
   //! <b>Complexity</b>: Linear to the elements with keys bigger than position
   //!
   //! <b>Note</b>: Invalidates elements with keys
   //!   not less than the erased element.
   iterator erase(const_iterator position) 
      { return force_copy<iterator>(m_flat_tree.erase(force<impl_const_iterator>(position))); }

   //! <b>Effects</b>: Erases all elements in the container with key equivalent to x.
   //!
   //! <b>Returns</b>: Returns the number of erased elements.
   //!
   //! <b>Complexity</b>: Logarithmic search time plus erasure time
   //!   linear to the elements with bigger keys.
   size_type erase(const key_type& x) 
      { return m_flat_tree.erase(x); }

   //! <b>Effects</b>: Erases all the elements in the range [first, last).
   //!
   //! <b>Returns</b>: Returns last.
   //!
   //! <b>Complexity</b>: size()*N where N is the distance from first to last.
   //!
   //! <b>Complexity</b>: Logarithmic search time plus erasure time
   //!   linear to the elements with bigger keys.
   iterator erase(const_iterator first, const_iterator last)
      { return force_copy<iterator>(m_flat_tree.erase(force<impl_const_iterator>(first), force<impl_const_iterator>(last))); }

   //! <b>Effects</b>: erase(a.begin(),a.end()).
   //!
   //! <b>Postcondition</b>: size() == 0.
   //!
   //! <b>Complexity</b>: linear in size().
   void clear() 
      { m_flat_tree.clear(); }

   //! <b>Effects</b>: Tries to deallocate the excess of memory created
   //    with previous allocations. The size of the vector is unchanged
   //!
   //! <b>Throws</b>: If memory allocation throws, or T's copy constructor throws.
   //!
   //! <b>Complexity</b>: Linear to size().
   void shrink_to_fit()
      { m_flat_tree.shrink_to_fit(); }

   //! <b>Returns</b>: An iterator pointing to an element with the key
   //!   equivalent to x, or end() if such an element is not found.
   //!
   //! <b>Complexity</b>: Logarithmic.
   iterator find(const key_type& x)
      { return force_copy<iterator>(m_flat_tree.find(x)); }

   //! <b>Returns</b>: An const_iterator pointing to an element with the key
   //!   equivalent to x, or end() if such an element is not found.
   //!
   //! <b>Complexity</b>: Logarithmic.
   const_iterator find(const key_type& x) const 
      { return force<const_iterator>(m_flat_tree.find(x)); }

   //! <b>Returns</b>: The number of elements with key equivalent to x.
   //!
   //! <b>Complexity</b>: log(size())+count(k)
   size_type count(const key_type& x) const 
      { return m_flat_tree.count(x); }

   //! <b>Returns</b>: An iterator pointing to the first element with key not less
   //!   than k, or a.end() if such an element is not found.
   //!
   //! <b>Complexity</b>: Logarithmic
   iterator lower_bound(const key_type& x) 
      {return force_copy<iterator>(m_flat_tree.lower_bound(x)); }

   //! <b>Returns</b>: A const iterator pointing to the first element with key
   //!   not less than k, or a.end() if such an element is not found.
   //!
   //! <b>Complexity</b>: Logarithmic
   const_iterator lower_bound(const key_type& x) const 
      {  return force<const_iterator>(m_flat_tree.lower_bound(x));  }

   //! <b>Returns</b>: An iterator pointing to the first element with key not less
   //!   than x, or end() if such an element is not found.
   //!
   //! <b>Complexity</b>: Logarithmic
   iterator upper_bound(const key_type& x) 
      {return force_copy<iterator>(m_flat_tree.upper_bound(x)); }

   //! <b>Returns</b>: A const iterator pointing to the first element with key
   //!   not less than x, or end() if such an element is not found.
   //!
   //! <b>Complexity</b>: Logarithmic
   const_iterator upper_bound(const key_type& x) const 
      {  return force<const_iterator>(m_flat_tree.upper_bound(x)); }

   //! <b>Effects</b>: Equivalent to std::make_pair(this->lower_bound(k), this->upper_bound(k)).
   //!
   //! <b>Complexity</b>: Logarithmic
   std::pair<iterator,iterator> equal_range(const key_type& x) 
      {  return force_copy<std::pair<iterator,iterator> >(m_flat_tree.equal_range(x));   }

   //! <b>Effects</b>: Equivalent to std::make_pair(this->lower_bound(k), this->upper_bound(k)).
   //!
   //! <b>Complexity</b>: Logarithmic
   std::pair<const_iterator,const_iterator> 
      equal_range(const key_type& x) const 
      {  return force_copy<std::pair<const_iterator,const_iterator> >(m_flat_tree.equal_range(x));   }

   //! <b>Effects</b>: Number of elements for which memory has been allocated.
   //!   capacity() is always greater than or equal to size().
   //! 
   //! <b>Throws</b>: Nothing.
   //! 
   //! <b>Complexity</b>: Constant.
   size_type capacity() const           
      { return m_flat_tree.capacity(); }

   //! <b>Effects</b>: If n is less than or equal to capacity(), this call has no
   //!   effect. Otherwise, it is a request for allocation of additional memory.
   //!   If the request is successful, then capacity() is greater than or equal to
   //!   n; otherwise, capacity() is unchanged. In either case, size() is unchanged.
   //! 
   //! <b>Throws</b>: If memory allocation allocation throws or T's copy constructor throws.
   //!
   //! <b>Note</b>: If capacity() is less than "count", iterators and references to
   //!   to values might be invalidated.
   void reserve(size_type count)       
      { m_flat_tree.reserve(count);   }

   /// @cond
   template <class K1, class T1, class C1, class A1>
   friend bool operator== (const flat_multimap<K1, T1, C1, A1>& x,
                           const flat_multimap<K1, T1, C1, A1>& y);

   template <class K1, class T1, class C1, class A1>
   friend bool operator< (const flat_multimap<K1, T1, C1, A1>& x,
                          const flat_multimap<K1, T1, C1, A1>& y);
   /// @endcond
};

template <class Key, class T, class Pred, class Alloc>
inline bool operator==(const flat_multimap<Key,T,Pred,Alloc>& x, 
                       const flat_multimap<Key,T,Pred,Alloc>& y) 
   {  return x.m_flat_tree == y.m_flat_tree;  }

template <class Key, class T, class Pred, class Alloc>
inline bool operator<(const flat_multimap<Key,T,Pred,Alloc>& x, 
                      const flat_multimap<Key,T,Pred,Alloc>& y) 
   {  return x.m_flat_tree < y.m_flat_tree;   }

template <class Key, class T, class Pred, class Alloc>
inline bool operator!=(const flat_multimap<Key,T,Pred,Alloc>& x, 
                       const flat_multimap<Key,T,Pred,Alloc>& y) 
   {  return !(x == y);  }

template <class Key, class T, class Pred, class Alloc>
inline bool operator>(const flat_multimap<Key,T,Pred,Alloc>& x, 
                      const flat_multimap<Key,T,Pred,Alloc>& y) 
   {  return y < x;  }

template <class Key, class T, class Pred, class Alloc>
inline bool operator<=(const flat_multimap<Key,T,Pred,Alloc>& x, 
                       const flat_multimap<Key,T,Pred,Alloc>& y) 
   {  return !(y < x);  }

template <class Key, class T, class Pred, class Alloc>
inline bool operator>=(const flat_multimap<Key,T,Pred,Alloc>& x, 
                       const flat_multimap<Key,T,Pred,Alloc>& y) 
   {  return !(x < y);  }

#ifndef BOOST_INTERPROCESS_RVALUE_REFERENCE
template <class Key, class T, class Pred, class Alloc>
inline void swap(flat_multimap<Key,T,Pred,Alloc>& x, 
                 flat_multimap<Key,T,Pred,Alloc>& y) 
   {  x.swap(y);  }

template <class Key, class T, class Pred, class Alloc>
inline void swap(const detail::moved_object<flat_multimap<Key,T,Pred,Alloc> >& x, 
                 flat_multimap<Key,T,Pred,Alloc>& y) 
   {  x.get().swap(y);  }


template <class Key, class T, class Pred, class Alloc>
inline void swap(flat_multimap<Key,T,Pred,Alloc>& x, 
                 const detail::moved_object<flat_multimap<Key,T,Pred,Alloc> > & y) 
   {  x.swap(y.get());  }
#else
template <class Key, class T, class Pred, class Alloc>
inline void swap(flat_multimap<Key,T,Pred,Alloc>&&x, 
                 flat_multimap<Key,T,Pred,Alloc>&&y) 
   {  x.swap(y);  }
#endif

/// @cond

//!This class is movable
template <class K, class T, class C, class A>
struct is_movable<flat_multimap<K, T, C, A> >
{
   enum {   value = true };
};

//!has_trivial_destructor_after_move<> == true_type
//!specialization for optimizations
template <class K, class T, class C, class A>
struct has_trivial_destructor_after_move<flat_multimap<K, T, C, A> >
{
   enum {   value = 
               has_trivial_destructor<A>::value &&
               has_trivial_destructor<C>::value  };
};
/// @endcond

}} //namespace boost { namespace interprocess {

#include <boost/interprocess/detail/config_end.hpp>

#endif /* BOOST_INTERPROCESS_FLAT_MAP_HPP */
