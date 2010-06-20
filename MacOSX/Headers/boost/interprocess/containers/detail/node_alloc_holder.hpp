//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2005-2008. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/interprocess for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_INTERPROCESS_DETAIL_NODE_ALLOC_HPP_
#define BOOST_INTERPROCESS_DETAIL_NODE_ALLOC_HPP_

#if (defined _MSC_VER) && (_MSC_VER >= 1200)
#  pragma once
#endif

#include <boost/interprocess/detail/config_begin.hpp>
#include <boost/interprocess/detail/workaround.hpp>

#include <boost/interprocess/interprocess_fwd.hpp>
#include <boost/interprocess/detail/version_type.hpp>
#include <boost/interprocess/detail/move.hpp>
#include <boost/interprocess/detail/algorithms.hpp>
#include <boost/interprocess/detail/type_traits.hpp>
#include <boost/interprocess/detail/utilities.hpp>
#include <boost/interprocess/detail/mpl.hpp>
#include <boost/intrusive/options.hpp>

#include <utility>
#include <functional>


namespace boost {
namespace interprocess {
namespace detail {

template<class ValueCompare, class Node>
struct node_compare
   :  private ValueCompare
{
   typedef typename ValueCompare::key_type     key_type;
   typedef typename ValueCompare::value_type   value_type;
   typedef typename ValueCompare::key_of_value key_of_value;

   node_compare(const ValueCompare &pred)
      :  ValueCompare(pred)
   {}

   ValueCompare &value_comp()
   {  return static_cast<ValueCompare &>(*this);  }

   ValueCompare &value_comp() const
   {  return static_cast<const ValueCompare &>(*this);  }

   bool operator()(const Node &a, const Node &b) const
   {  return ValueCompare::operator()(a.get_data(), b.get_data());  }
};

template<class A, class ICont>
struct node_alloc_holder
{
   typedef node_alloc_holder<A, ICont>                self_t;
   typedef typename A::value_type                     value_type;
   typedef typename ICont::value_type                 Node;
   typedef typename A::template rebind<Node>::other   NodeAlloc;
   typedef A                                          ValAlloc;
   typedef typename NodeAlloc::pointer                NodePtr;
   typedef detail::scoped_deallocator<NodeAlloc>      Deallocator;
   typedef typename NodeAlloc::size_type              size_type;
   typedef typename NodeAlloc::difference_type        difference_type;
   typedef detail::integral_constant<unsigned, 1>     allocator_v1;
   typedef detail::integral_constant<unsigned, 2>     allocator_v2;
   typedef detail::integral_constant<unsigned,
      boost::interprocess::detail::
         version<NodeAlloc>::value>                   alloc_version;
   typedef typename ICont::iterator                   icont_iterator;
   typedef typename ICont::const_iterator             icont_citerator;
   typedef allocator_destroyer<NodeAlloc>             Destroyer;

   node_alloc_holder(const ValAlloc &a) 
      : members_(a)
   {}

   node_alloc_holder(const node_alloc_holder &other)
      : members_(other.node_alloc())
   {}

   #ifndef BOOST_INTERPROCESS_RVALUE_REFERENCE
   node_alloc_holder(const detail::moved_object<node_alloc_holder> &other)
      : members_(detail::move_impl(other.get().node_alloc()))
   {  this->swap(other.get());  }
   #else
   node_alloc_holder(node_alloc_holder &&other)
      : members_(detail::move_impl(other.node_alloc()))
   {  this->swap(other);  }
   #endif

   template<class Pred>
   node_alloc_holder(const ValAlloc &a, const Pred &c) 
      : members_(a, typename ICont::value_compare(c))
   {}

   #ifndef BOOST_INTERPROCESS_RVALUE_REFERENCE
   template<class Pred>
   node_alloc_holder(const detail::moved_object<ValAlloc> &a, const Pred &c) 
      : members_(a.get(), typename ICont::value_compare(c))
   {}
   #else
   template<class Pred>
   node_alloc_holder(ValAlloc &&a, const Pred &c) 
      : members_(a, typename ICont::value_compare(c))
   {}
   #endif

   template<class Pred>
   node_alloc_holder(const node_alloc_holder &other, const Pred &c)
      : members_(other.node_alloc(), typename ICont::value_compare(c))
   {}

   ~node_alloc_holder()
   {}

   size_type max_size() const
   {  return this->node_alloc().max_size();  }

   NodePtr allocate_one()
   {  return this->allocate_one(alloc_version());   }

   NodePtr allocate_one(allocator_v1)
   {  return this->node_alloc().allocate(1);   }

   NodePtr allocate_one(allocator_v2)
   {  return this->node_alloc().allocate_one();   }

   void deallocate_one(NodePtr p)
   {  return this->deallocate_one(p, alloc_version());   }

   void deallocate_one(NodePtr p, allocator_v1)
   {  this->node_alloc().deallocate(p, 1);   }

   void deallocate_one(NodePtr p, allocator_v2)
   {  this->node_alloc().deallocate_one(p);   }

   #ifndef BOOST_INTERPROCESS_RVALUE_REFERENCE
   template<class Convertible1, class Convertible2>
   static void construct(const NodePtr &ptr, const detail::moved_object<std::pair<Convertible1, Convertible2> > &value)
   {
      typedef typename Node::hook_type                hook_type;
      typedef typename Node::value_type::first_type   first_type;
      typedef typename Node::value_type::second_type  second_type;
      Node *nodeptr = detail::get_pointer(ptr);

      //Hook constructor does not throw
      new(static_cast<hook_type*>(nodeptr))hook_type();
      //Now construct pair members_holder
      value_type *valueptr = &nodeptr->get_data();
      new((void*)&valueptr->first) first_type(detail::move_impl(value.get().first));
      BOOST_TRY{
         new((void*)&valueptr->second) second_type(detail::move_impl(value.get().second));
      }
      BOOST_CATCH(...){
         valueptr->first.~first_type();
         static_cast<hook_type*>(nodeptr)->~hook_type();
         BOOST_RETHROW
      }
      BOOST_CATCH_END
   }
   #else
   template<class Convertible1, class Convertible2>
   static void construct(const NodePtr &ptr, std::pair<Convertible1, Convertible2> &&value)
   {  
      typedef typename Node::hook_type                hook_type;
      typedef typename Node::value_type::first_type   first_type;
      typedef typename Node::value_type::second_type  second_type;
      Node *nodeptr = detail::get_pointer(ptr);

      //Hook constructor does not throw
      new(static_cast<hook_type*>(nodeptr))hook_type();
      //Now construct pair members_holder
      value_type *valueptr = &nodeptr->get_data();
      new((void*)&valueptr->first) first_type(detail::move_impl(value.first));
      BOOST_TRY{
         new((void*)&valueptr->second) second_type(detail::move_impl(value.second));
      }
      BOOST_CATCH(...){
         valueptr->first.~first_type();
         static_cast<hook_type*>(nodeptr)->~hook_type();
         BOOST_RETHROW
      }
      BOOST_CATCH_END
   }
   #endif

   static void destroy(const NodePtr &ptr)
   {  detail::get_pointer(ptr)->~Node();  }


   #ifdef BOOST_INTERPROCESS_RVALUE_REFERENCE
   Deallocator
   #else
   move_return<Deallocator>
   #endif
   create_node_and_deallocator()
   {
      NodePtr p = this->allocate_one();
      Deallocator node_deallocator(p, this->node_alloc());
      return node_deallocator;
   }

   #ifdef BOOST_INTERPROCESS_PERFECT_FORWARDING

   template<class ...Args>
   static void construct(const NodePtr &ptr, Args &&...args)
   {  new((void*)detail::get_pointer(ptr)) Node(detail::forward_impl<Args>(args)...);  }

   template<class ...Args>
   NodePtr create_node(Args &&...args)
   {
      NodePtr p = this->allocate_one();
      Deallocator node_deallocator(p, this->node_alloc());
      self_t::construct(p, detail::forward_impl<Args>(args)...);
      node_deallocator.release();
      return (p);
   }

   #else //#ifdef BOOST_INTERPROCESS_PERFECT_FORWARDING

   static void construct(const NodePtr &ptr)
   {  new((void*)detail::get_pointer(ptr)) Node();  }

   NodePtr create_node()
   {
      NodePtr p = this->allocate_one();
      Deallocator node_deallocator(p, this->node_alloc());
      self_t::construct(p);
      node_deallocator.release();
      return (p);
   }

   #define BOOST_PP_LOCAL_MACRO(n)                                                           \
   template<BOOST_PP_ENUM_PARAMS(n, class P)>                                                \
   void construct(const NodePtr &ptr, BOOST_PP_ENUM(n, BOOST_INTERPROCESS_PP_PARAM_LIST, _)) \
   {                                                                                         \
      new((void*)detail::get_pointer(ptr))                                                   \
      Node(BOOST_PP_ENUM(n, BOOST_INTERPROCESS_PP_PARAM_FORWARD, _));                        \
   }                                                                                         \
   //!
   #define BOOST_PP_LOCAL_LIMITS (1, BOOST_INTERPROCESS_MAX_CONSTRUCTOR_PARAMETERS)
   #include BOOST_PP_LOCAL_ITERATE()

   #define BOOST_PP_LOCAL_MACRO(n)                                                        \
   template<BOOST_PP_ENUM_PARAMS(n, class P)>                                             \
   NodePtr create_node(BOOST_PP_ENUM(n, BOOST_INTERPROCESS_PP_PARAM_LIST, _))             \
   {                                                                                      \
      NodePtr p = this->allocate_one();                                                   \
      Deallocator node_deallocator(p, this->node_alloc());                                \
      self_t::construct(p, BOOST_PP_ENUM(n, BOOST_INTERPROCESS_PP_PARAM_FORWARD, _));     \
      node_deallocator.release();                                                         \
      return (p);                                                                         \
   }                                                                                      \
   //!
   #define BOOST_PP_LOCAL_LIMITS (1, BOOST_INTERPROCESS_MAX_CONSTRUCTOR_PARAMETERS)
   #include BOOST_PP_LOCAL_ITERATE()

   #endif   //#ifdef BOOST_INTERPROCESS_PERFECT_FORWARDING

   template<class It>
   NodePtr create_node_from_it(It it)
   {
      NodePtr p = this->allocate_one();
      Deallocator node_deallocator(p, this->node_alloc());
      ::boost::interprocess::construct_in_place(detail::get_pointer(p), it);
      node_deallocator.release();
      return (p);
   }

   void destroy_node(NodePtr node)
   {
      self_t::destroy(node);
      this->deallocate_one(node);
   }

   void swap(node_alloc_holder &x)
   {
      NodeAlloc& this_alloc   = this->node_alloc();
      NodeAlloc& other_alloc  = x.node_alloc();

      if (this_alloc != other_alloc){
         detail::do_swap(this_alloc, other_alloc);
      }

      this->icont().swap(x.icont());
   }

   template<class FwdIterator, class Inserter>
   FwdIterator allocate_many_and_construct
      (FwdIterator beg, difference_type n, Inserter inserter)
   {
      typedef typename NodeAlloc::multiallocation_iterator multiallocation_iterator;

      //Try to allocate memory in a single block
      multiallocation_iterator itbeg =
         this->node_alloc().allocate_individual(n), itend, itold;
      int constructed = 0;
      Node *p = 0;
      BOOST_TRY{
         for(difference_type i = 0; i < n; ++i, ++beg, --constructed){
            p = &*itbeg;
            ++itbeg;
            //This can throw
            boost::interprocess::construct_in_place(p, beg);
            ++constructed;
            //This can throw in some containers (predicate might throw)
            inserter(*p);
         }
      }
      BOOST_CATCH(...){
         if(constructed){
            this->destroy(p);
         }
         this->node_alloc().deallocate_many(itbeg);
         BOOST_RETHROW
      }
      BOOST_CATCH_END
      return beg;
   }

   void clear(allocator_v1)
   {  this->icont().clear_and_dispose(Destroyer(this->node_alloc()));   }

   void clear(allocator_v2)
   {
      allocator_multialloc_chain_node_deallocator<NodeAlloc> chain_holder(this->node_alloc());
      this->icont().clear_and_dispose(chain_holder.get_chain_builder());
   }

   icont_iterator erase_range(icont_iterator first, icont_iterator last, allocator_v1)
   {  return this->icont().erase_and_dispose(first, last, Destroyer(this->node_alloc())); }

   icont_iterator erase_range(icont_iterator first, icont_iterator last, allocator_v2)
   {
      allocator_multialloc_chain_node_deallocator<NodeAlloc> chain_holder(this->node_alloc());
      return this->icont().erase_and_dispose(first, last, chain_holder.get_chain_builder());
   }

   template<class Key, class Comparator>
   size_type erase_key(const Key& k, const Comparator &comp, allocator_v1)
   {  return this->icont().erase_and_dispose(k, comp, Destroyer(this->node_alloc())); }

   template<class Key, class Comparator>
   size_type erase_key(const Key& k, const Comparator &comp, allocator_v2)
   {
      allocator_multialloc_chain_node_deallocator<NodeAlloc> chain_holder(this->node_alloc());
      return this->icont().erase_and_dispose(k, comp, chain_holder.get_chain_builder());
   }

   protected:
   struct cloner
   {
      cloner(node_alloc_holder &holder)
         :  m_holder(holder)
      {}

      NodePtr operator()(const Node &other) const
      {  return m_holder.create_node(other.get_data());  }

      node_alloc_holder &m_holder;
   };

   struct destroyer
   {
      destroyer(node_alloc_holder &holder)
         :  m_holder(holder)
      {}

      void operator()(NodePtr n) const
      {  m_holder.destroy_node(n);  }

      node_alloc_holder &m_holder;
   };

   struct members_holder
      :  public NodeAlloc
   {
      private:
      members_holder(const members_holder&);

      public:
      template<class ConvertibleToAlloc>
      members_holder(const ConvertibleToAlloc &c2alloc)
         :  NodeAlloc(c2alloc)
      {}

      template<class ConvertibleToAlloc, class Pred>
      members_holder(const ConvertibleToAlloc &c2alloc, const Pred &c)
         :  NodeAlloc(c2alloc), m_icont(c)
      {}
      //The intrusive container
      ICont m_icont;
   } members_;

   ICont &non_const_icont() const
   {  return const_cast<ICont&>(this->members_.m_icont);   }

   ICont &icont()
   {  return this->members_.m_icont;   }

   const ICont &icont() const
   {  return this->members_.m_icont;   }

   NodeAlloc &node_alloc()
   {  return static_cast<NodeAlloc &>(this->members_);   }

   const NodeAlloc &node_alloc() const
   {  return static_cast<const NodeAlloc &>(this->members_);   }
};

}  //namespace detail {
}  //namespace interprocess {
}  //namespace boost {

#include <boost/interprocess/detail/config_end.hpp>

#endif // BOOST_INTERPROCESS_DETAIL_NODE_ALLOC_HPP_
