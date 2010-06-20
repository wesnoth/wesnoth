//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2008. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/interprocess for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_INTERPROCESS_DETAIL_NODE_ALLOCATOR_COMMON_HPP
#define BOOST_INTERPROCESS_DETAIL_NODE_ALLOCATOR_COMMON_HPP

#include <boost/interprocess/detail/config_begin.hpp>
#include <boost/interprocess/detail/workaround.hpp>
#include <boost/interprocess/segment_manager.hpp>
#include <boost/interprocess/interprocess_fwd.hpp>
#include <boost/interprocess/detail/utilities.hpp> //pointer_to_other, get_pointer
#include <utility>   //std::pair
#include <boost/utility/addressof.hpp> //boost::addressof
#include <boost/assert.hpp>   //BOOST_ASSERT
#include <boost/interprocess/exceptions.hpp> //bad_alloc
#include <boost/interprocess/sync/scoped_lock.hpp> //scoped_lock
#include <boost/interprocess/allocators/allocation_type.hpp> //allocation_type
#include <algorithm> //std::swap


namespace boost {
namespace interprocess {
namespace detail {

//!Object function that creates the node allocator if it is not created and
//!increments reference count if it is already created
template<class NodePool>
struct get_or_create_node_pool_func
{
   
   //!This connects or constructs the unique instance of node_pool_t
   //!Can throw boost::interprocess::bad_alloc
   void operator()()
   {
      //Find or create the node_pool_t
      mp_node_pool =    mp_segment_manager->template find_or_construct
                        <NodePool>(unique_instance)(mp_segment_manager);
      //If valid, increment link count
      if(mp_node_pool != 0)
         mp_node_pool->inc_ref_count();
   }

   //!Constructor. Initializes function
   //!object parameters
   get_or_create_node_pool_func(typename NodePool::segment_manager *mngr)
      : mp_segment_manager(mngr){}
   
   NodePool                            *mp_node_pool;
   typename NodePool::segment_manager  *mp_segment_manager;
};

template<class NodePool>
inline NodePool *get_or_create_node_pool(typename NodePool::segment_manager *mgnr)
{
   detail::get_or_create_node_pool_func<NodePool> func(mgnr);
   mgnr->atomic_func(func);
   return func.mp_node_pool;
}

//!Object function that decrements the reference count. If the count 
//!reaches to zero destroys the node allocator from memory. 
//!Never throws
template<class NodePool>
struct destroy_if_last_link_func
{
   //!Decrements reference count and destroys the object if there is no 
   //!more attached allocators. Never throws
   void operator()()
   {
      //If not the last link return
      if(mp_node_pool->dec_ref_count() != 0) return;

      //Last link, let's destroy the segment_manager
      mp_node_pool->get_segment_manager()->template destroy<NodePool>(unique_instance); 
   }  

   //!Constructor. Initializes function
   //!object parameters
   destroy_if_last_link_func(NodePool *pool) 
      : mp_node_pool(pool)
   {}

   NodePool                           *mp_node_pool;
};

//!Destruction function, initializes and executes destruction function 
//!object. Never throws
template<class NodePool>
inline void destroy_node_pool_if_last_link(NodePool *pool)
{
   //Get segment manager
   typename NodePool::segment_manager *mngr = pool->get_segment_manager();
   //Execute destruction functor atomically
   destroy_if_last_link_func<NodePool>func(pool);
   mngr->atomic_func(func);
}

template<class NodePool>
class cache_impl
{
   typedef typename NodePool::segment_manager::
      void_pointer                                    void_pointer;
   typedef typename pointer_to_other
      <void_pointer, NodePool>::type                  node_pool_ptr;
   typedef typename NodePool::multiallocation_chain   multiallocation_chain;
   node_pool_ptr           mp_node_pool;
   multiallocation_chain   m_cached_nodes;
   std::size_t             m_max_cached_nodes;

   public:
   typedef typename NodePool::multiallocation_iterator   multiallocation_iterator;
   typedef typename NodePool::segment_manager            segment_manager;

   cache_impl(segment_manager *segment_mngr, std::size_t max_cached_nodes)
      : mp_node_pool(get_or_create_node_pool<NodePool>(segment_mngr))
      , m_max_cached_nodes(max_cached_nodes)
   {}

   cache_impl(const cache_impl &other)
      : mp_node_pool(other.get_node_pool())
      , m_max_cached_nodes(other.get_max_cached_nodes())
   {
      mp_node_pool->inc_ref_count();
   }

   ~cache_impl()
   {
      this->deallocate_all_cached_nodes();
      detail::destroy_node_pool_if_last_link(detail::get_pointer(mp_node_pool));   
   }

   NodePool *get_node_pool() const
   {  return detail::get_pointer(mp_node_pool); }

   segment_manager *get_segment_manager() const
   {  return mp_node_pool->get_segment_manager(); }

   std::size_t get_max_cached_nodes() const
   {  return m_max_cached_nodes; }

   void *cached_allocation()
   {
      //If don't have any cached node, we have to get a new list of free nodes from the pool
      if(m_cached_nodes.empty()){
         mp_node_pool->allocate_nodes(m_cached_nodes, m_max_cached_nodes/2);
      }
      return m_cached_nodes.pop_front();
   }

   multiallocation_iterator cached_allocation(std::size_t n)
   {
      multiallocation_chain chain;
      std::size_t count = n;
      BOOST_TRY{
         //If don't have any cached node, we have to get a new list of free nodes from the pool
         while(!m_cached_nodes.empty() && count--){
            void *ret = m_cached_nodes.pop_front();
            chain.push_back(ret);
         }

         if(chain.size() != n){
            mp_node_pool->allocate_nodes(chain, n - chain.size());
         }
         assert(chain.size() == n);
         chain.splice_back(m_cached_nodes);
         return multiallocation_iterator(chain.get_it());
      }
      BOOST_CATCH(...){
         this->cached_deallocation(multiallocation_iterator(chain.get_it()));
         BOOST_RETHROW
      }
      BOOST_CATCH_END
   }

   void cached_deallocation(void *ptr)
   {
      //Check if cache is full
      if(m_cached_nodes.size() >= m_max_cached_nodes){
         //This only occurs if this allocator deallocate memory allocated
         //with other equal allocator. Since the cache is full, and more 
         //deallocations are probably coming, we'll make some room in cache
         //in a single, efficient multi node deallocation.
         this->priv_deallocate_n_nodes(m_cached_nodes.size() - m_max_cached_nodes/2);
      }
      m_cached_nodes.push_front(ptr);
   }

   void cached_deallocation(multiallocation_iterator it)
   {
      multiallocation_iterator itend;

      while(it != itend){
         void *addr = &*it;
         ++it;
         m_cached_nodes.push_front(addr);
      }

      //Check if cache is full
      if(m_cached_nodes.size() >= m_max_cached_nodes){
         //This only occurs if this allocator deallocate memory allocated
         //with other equal allocator. Since the cache is full, and more 
         //deallocations are probably coming, we'll make some room in cache
         //in a single, efficient multi node deallocation.
         this->priv_deallocate_n_nodes(m_cached_nodes.size() - m_max_cached_nodes/2);
      }
   }

   //!Sets the new max cached nodes value. This can provoke deallocations
   //!if "newmax" is less than current cached nodes. Never throws
   void set_max_cached_nodes(std::size_t newmax)
   {
      m_max_cached_nodes = newmax;
      this->priv_deallocate_remaining_nodes();
   }

   //!Frees all cached nodes.
   //!Never throws
   void deallocate_all_cached_nodes()
   {
      if(m_cached_nodes.empty()) return;
      mp_node_pool->deallocate_nodes(m_cached_nodes);
   }

   private:
   //!Frees all cached nodes at once.
   //!Never throws
   void priv_deallocate_remaining_nodes()
   {
      if(m_cached_nodes.size() > m_max_cached_nodes){
         priv_deallocate_n_nodes(m_cached_nodes.size()-m_max_cached_nodes);
      }
   }

   //!Frees n cached nodes at once. Never throws
   void priv_deallocate_n_nodes(std::size_t n)
   {
      //Deallocate all new linked list at once
      mp_node_pool->deallocate_nodes(m_cached_nodes, n);
   }
};

template<class Derived, class T, class SegmentManager>
class array_allocation_impl
{
   const Derived *derived() const
   {  return static_cast<const Derived*>(this); }
   Derived *derived()
   {  return static_cast<Derived*>(this); }

   typedef typename SegmentManager::void_pointer         void_pointer;

   public:
   typedef typename detail::
      pointer_to_other<void_pointer, T>::type            pointer;
   typedef typename detail::
      pointer_to_other<void_pointer, const T>::type      const_pointer;
   typedef T                                             value_type;
   typedef typename detail::add_reference
                     <value_type>::type                  reference;
   typedef typename detail::add_reference
                     <const value_type>::type            const_reference;
   typedef std::size_t                                   size_type;
   typedef std::ptrdiff_t                                difference_type;
   typedef transform_iterator
      < typename SegmentManager::
         multiallocation_iterator
      , detail::cast_functor <T> >          multiallocation_iterator;
   typedef typename SegmentManager::
      multiallocation_chain                 multiallocation_chain;

   public:
   //!Returns maximum the number of objects the previously allocated memory
   //!pointed by p can hold. This size only works for memory allocated with
   //!allocate, allocation_command and allocate_many.
   size_type size(const pointer &p) const
   {  
      return (size_type)this->derived()->get_segment_manager()->size(detail::get_pointer(p))/sizeof(T);
   }

   std::pair<pointer, bool>
      allocation_command(allocation_type command,
                         size_type limit_size, 
                         size_type preferred_size,
                         size_type &received_size, const pointer &reuse = 0)
   {
      return this->derived()->get_segment_manager()->allocation_command
         (command, limit_size, preferred_size, received_size, detail::get_pointer(reuse));
   }

   //!Allocates many elements of size elem_size in a contiguous block
   //!of memory. The minimum number to be allocated is min_elements,
   //!the preferred and maximum number is
   //!preferred_elements. The number of actually allocated elements is
   //!will be assigned to received_size. The elements must be deallocated
   //!with deallocate(...)
   multiallocation_iterator allocate_many(size_type elem_size, std::size_t num_elements)
   {
      return multiallocation_iterator
         (this->derived()->get_segment_manager()->allocate_many(sizeof(T)*elem_size, num_elements));
   }

   //!Allocates n_elements elements, each one of size elem_sizes[i]in a
   //!contiguous block
   //!of memory. The elements must be deallocated
   multiallocation_iterator allocate_many(const size_type *elem_sizes, size_type n_elements)
   {
      return multiallocation_iterator
         (this->derived()->get_segment_manager()->allocate_many(elem_sizes, n_elements, sizeof(T)));
   }

   //!Allocates many elements of size elem_size in a contiguous block
   //!of memory. The minimum number to be allocated is min_elements,
   //!the preferred and maximum number is
   //!preferred_elements. The number of actually allocated elements is
   //!will be assigned to received_size. The elements must be deallocated
   //!with deallocate(...)
   void deallocate_many(multiallocation_iterator it)
   {  return this->derived()->get_segment_manager()->deallocate_many(it.base()); }

   //!Returns the number of elements that could be
   //!allocated. Never throws
   size_type max_size() const
   {  return this->derived()->get_segment_manager()->get_size()/sizeof(T);  }

   //!Returns address of mutable object.
   //!Never throws
   pointer address(reference value) const
   {  return pointer(boost::addressof(value));  }

   //!Returns address of non mutable object.
   //!Never throws
   const_pointer address(const_reference value) const
   {  return const_pointer(boost::addressof(value));  }

   //!Default construct an object. 
   //!Throws if T's default constructor throws
   void construct(const pointer &ptr)
   {  new((void*)detail::get_pointer(ptr)) value_type;  }

   //!Copy construct an object
   //!Throws if T's copy constructor throws
   void construct(const pointer &ptr, const_reference v)
   {  new((void*)detail::get_pointer(ptr)) value_type(v);  }

   //!Destroys object. Throws if object's
   //!destructor throws
   void destroy(const pointer &ptr)
   {  BOOST_ASSERT(ptr != 0); (*ptr).~value_type();  }
};


template<class Derived, unsigned int Version, class T, class SegmentManager>
class node_pool_allocation_impl
   :  public array_allocation_impl
      < Derived
      , T
      , SegmentManager>
{
   const Derived *derived() const
   {  return static_cast<const Derived*>(this); }
   Derived *derived()
   {  return static_cast<Derived*>(this); }

   typedef typename SegmentManager::void_pointer         void_pointer;
   typedef typename detail::
      pointer_to_other<void_pointer, const void>::type   cvoid_pointer;

   public:
   typedef typename detail::
      pointer_to_other<void_pointer, T>::type            pointer;
   typedef typename detail::
      pointer_to_other<void_pointer, const T>::type      const_pointer;
   typedef T                                             value_type;
   typedef typename detail::add_reference
                     <value_type>::type                  reference;
   typedef typename detail::add_reference
                     <const value_type>::type            const_reference;
   typedef std::size_t                                   size_type;
   typedef std::ptrdiff_t                                difference_type;
   typedef transform_iterator
      < typename SegmentManager::
         multiallocation_iterator
      , detail::cast_functor <T> >          multiallocation_iterator;
   typedef typename SegmentManager::
      multiallocation_chain                 multiallocation_chain;

   template <int Dummy>
   struct node_pool
   {
      typedef typename Derived::template node_pool<0>::type type;
      static type *get(void *p)
      {  return static_cast<type*>(p); }
   };

   public:
   //!Allocate memory for an array of count elements. 
   //!Throws boost::interprocess::bad_alloc if there is no enough memory
   pointer allocate(size_type count, cvoid_pointer hint = 0)
   {
      (void)hint;
      typedef typename node_pool<0>::type node_pool_t;
      node_pool_t *pool = node_pool<0>::get(this->derived()->get_node_pool());
      if(count > this->max_size())
         throw bad_alloc();
      else if(Version == 1 && count == 1)
         return pointer(static_cast<value_type*>
         (pool->allocate_node()));
      else
         return pointer(static_cast<value_type*>
            (pool->get_segment_manager()->allocate(sizeof(T)*count)));
   }

   //!Deallocate allocated memory. Never throws
   void deallocate(const pointer &ptr, size_type count)
   {
      (void)count;
      typedef typename node_pool<0>::type node_pool_t;
      node_pool_t *pool = node_pool<0>::get(this->derived()->get_node_pool());
      if(Version == 1 && count == 1)
         pool->deallocate_node(detail::get_pointer(ptr));
      else
         pool->get_segment_manager()->deallocate((void*)detail::get_pointer(ptr));
   }

   //!Allocates just one object. Memory allocated with this function
   //!must be deallocated only with deallocate_one().
   //!Throws boost::interprocess::bad_alloc if there is no enough memory
   pointer allocate_one()
   {
      typedef typename node_pool<0>::type node_pool_t;
      node_pool_t *pool = node_pool<0>::get(this->derived()->get_node_pool());
      return pointer(static_cast<value_type*>(pool->allocate_node()));
   }

   //!Allocates many elements of size == 1 in a contiguous block
   //!of memory. The minimum number to be allocated is min_elements,
   //!the preferred and maximum number is
   //!preferred_elements. The number of actually allocated elements is
   //!will be assigned to received_size. Memory allocated with this function
   //!must be deallocated only with deallocate_one().
   multiallocation_iterator allocate_individual(std::size_t num_elements)
   {
      typedef typename node_pool<0>::type node_pool_t;
      node_pool_t *pool = node_pool<0>::get(this->derived()->get_node_pool());
      return multiallocation_iterator(pool->allocate_nodes(num_elements));
   }

   //!Deallocates memory previously allocated with allocate_one().
   //!You should never use deallocate_one to deallocate memory allocated
   //!with other functions different from allocate_one(). Never throws
   void deallocate_one(const pointer &p)
   {
      typedef typename node_pool<0>::type node_pool_t;
      node_pool_t *pool = node_pool<0>::get(this->derived()->get_node_pool());
      pool->deallocate_node(detail::get_pointer(p));
   }

   //!Allocates many elements of size == 1 in a contiguous block
   //!of memory. The minimum number to be allocated is min_elements,
   //!the preferred and maximum number is
   //!preferred_elements. The number of actually allocated elements is
   //!will be assigned to received_size. Memory allocated with this function
   //!must be deallocated only with deallocate_one().
   void deallocate_individual(multiallocation_iterator it)
   {  node_pool<0>::get(this->derived()->get_node_pool())->deallocate_nodes(it.base());   }

   //!Deallocates all free blocks of the pool
   void deallocate_free_blocks()
   {  node_pool<0>::get(this->derived()->get_node_pool())->deallocate_free_blocks();  }

   //!Deprecated, use deallocate_free_blocks.
   //!Deallocates all free chunks of the pool.
   void deallocate_free_chunks()
   {  node_pool<0>::get(this->derived()->get_node_pool())->deallocate_free_blocks();  }
};

template<class T, class NodePool, unsigned int Version>
class cached_allocator_impl
   :  public array_allocation_impl
         <cached_allocator_impl<T, NodePool, Version>, T, typename NodePool::segment_manager>
{
   cached_allocator_impl & operator=(const cached_allocator_impl& other);
   typedef array_allocation_impl
         < cached_allocator_impl
            <T, NodePool, Version>
         , T
         , typename NodePool::segment_manager> base_t;

   public:
   typedef NodePool                                      node_pool_t;
   typedef typename NodePool::segment_manager            segment_manager;
   typedef typename segment_manager::void_pointer        void_pointer;
   typedef typename detail::
      pointer_to_other<void_pointer, const void>::type   cvoid_pointer;
   typedef typename base_t::pointer                      pointer;
   typedef typename base_t::size_type                    size_type;
   typedef typename base_t::multiallocation_iterator     multiallocation_iterator;
   typedef typename base_t::multiallocation_chain        multiallocation_chain;
   typedef typename base_t::value_type                   value_type;

   public:
   enum { DEFAULT_MAX_CACHED_NODES = 64 };

   cached_allocator_impl(segment_manager *segment_mngr, std::size_t max_cached_nodes)
      : m_cache(segment_mngr, max_cached_nodes)
   {}

   cached_allocator_impl(const cached_allocator_impl &other)
      : m_cache(other.m_cache)
   {}

   //!Copy constructor from related cached_adaptive_pool_base. If not present, constructs
   //!a node pool. Increments the reference count of the associated node pool.
   //!Can throw boost::interprocess::bad_alloc
   template<class T2, class NodePool2>
   cached_allocator_impl
      (const cached_allocator_impl
         <T2, NodePool2, Version> &other)
      : m_cache(other.get_segment_manager(), other.get_max_cached_nodes())
   {}

   //!Returns a pointer to the node pool.
   //!Never throws
   node_pool_t* get_node_pool() const
   {  return m_cache.get_node_pool();   }

   //!Returns the segment manager.
   //!Never throws
   segment_manager* get_segment_manager()const
   {  return m_cache.get_segment_manager();   }

   //!Sets the new max cached nodes value. This can provoke deallocations
   //!if "newmax" is less than current cached nodes. Never throws
   void set_max_cached_nodes(std::size_t newmax)
   {  m_cache.set_max_cached_nodes(newmax);   }

   //!Returns the max cached nodes parameter.
   //!Never throws
   std::size_t get_max_cached_nodes() const
   {  return m_cache.get_max_cached_nodes();   }

   //!Allocate memory for an array of count elements. 
   //!Throws boost::interprocess::bad_alloc if there is no enough memory
   pointer allocate(size_type count, cvoid_pointer hint = 0)
   {
      (void)hint;
      void * ret;
      if(count > this->max_size())
         throw bad_alloc();
      else if(Version == 1 && count == 1){
         ret = m_cache.cached_allocation();
      }
      else{
         ret = this->get_segment_manager()->allocate(sizeof(T)*count);
      }   
      return pointer(static_cast<T*>(ret));
   }

   //!Deallocate allocated memory. Never throws
   void deallocate(const pointer &ptr, size_type count)
   {
      (void)count;
      if(Version == 1 && count == 1){
         m_cache.cached_deallocation(detail::get_pointer(ptr));
      }
      else{
         this->get_segment_manager()->deallocate((void*)detail::get_pointer(ptr));
      }
   }

   //!Allocates just one object. Memory allocated with this function
   //!must be deallocated only with deallocate_one().
   //!Throws boost::interprocess::bad_alloc if there is no enough memory
   pointer allocate_one()
   {  return pointer(static_cast<value_type*>(this->m_cache.cached_allocation()));   }

   //!Allocates many elements of size == 1 in a contiguous block
   //!of memory. The minimum number to be allocated is min_elements,
   //!the preferred and maximum number is
   //!preferred_elements. The number of actually allocated elements is
   //!will be assigned to received_size. Memory allocated with this function
   //!must be deallocated only with deallocate_one().
   multiallocation_iterator allocate_individual(std::size_t num_elements)
   {  return multiallocation_iterator(this->m_cache.cached_allocation(num_elements));   }

   //!Deallocates memory previously allocated with allocate_one().
   //!You should never use deallocate_one to deallocate memory allocated
   //!with other functions different from allocate_one(). Never throws
   void deallocate_one(const pointer &p)
   {  this->m_cache.cached_deallocation(detail::get_pointer(p)); }

   //!Allocates many elements of size == 1 in a contiguous block
   //!of memory. The minimum number to be allocated is min_elements,
   //!the preferred and maximum number is
   //!preferred_elements. The number of actually allocated elements is
   //!will be assigned to received_size. Memory allocated with this function
   //!must be deallocated only with deallocate_one().
   void deallocate_individual(multiallocation_iterator it)
   {  m_cache.cached_deallocation(it.base());   }

   //!Deallocates all free blocks of the pool
   void deallocate_free_blocks()
   {  m_cache.get_node_pool()->deallocate_free_blocks();   }

   //!Swaps allocators. Does not throw. If each allocator is placed in a
   //!different shared memory segments, the result is undefined.
   friend void swap(cached_allocator_impl &alloc1, cached_allocator_impl &alloc2)
   {
      detail::do_swap(alloc1.mp_node_pool,       alloc2.mp_node_pool);
      alloc1.m_cached_nodes.swap(alloc2.m_cached_nodes);
      detail::do_swap(alloc1.m_max_cached_nodes, alloc2.m_max_cached_nodes);
   }

   void deallocate_cache()
   {  m_cache.deallocate_all_cached_nodes(); }

   //!Deprecated use deallocate_free_blocks.
   void deallocate_free_chunks()
   {  m_cache.get_node_pool()->deallocate_free_blocks();   }

   /// @cond
   private:
   cache_impl<node_pool_t> m_cache;
};

//!Equality test for same type of
//!cached_allocator_impl
template<class T, class N, unsigned int V> inline
bool operator==(const cached_allocator_impl<T, N, V> &alloc1, 
                const cached_allocator_impl<T, N, V> &alloc2)
   {  return alloc1.get_node_pool() == alloc2.get_node_pool(); }

//!Inequality test for same type of
//!cached_allocator_impl
template<class T, class N, unsigned int V> inline
bool operator!=(const cached_allocator_impl<T, N, V> &alloc1, 
                const cached_allocator_impl<T, N, V> &alloc2)
   {  return alloc1.get_node_pool() != alloc2.get_node_pool(); }


//!Pooled shared memory allocator using adaptive pool. Includes
//!a reference count but the class does not delete itself, this is  
//!responsibility of user classes. Node size (NodeSize) and the number of
//!nodes allocated per block (NodesPerBlock) are known at compile time
template<class private_node_allocator_t>
class shared_pool_impl
   : public private_node_allocator_t
{
 public:
   //!Segment manager typedef
   typedef typename private_node_allocator_t::segment_manager segment_manager;
   typedef typename private_node_allocator_t::
      multiallocation_iterator                  multiallocation_iterator;
   typedef typename private_node_allocator_t::
      multiallocation_chain                     multiallocation_chain;

 private:
   typedef typename segment_manager::mutex_family::mutex_type mutex_type;

 public:
   //!Constructor from a segment manager. Never throws
   shared_pool_impl(segment_manager *segment_mngr)
      : private_node_allocator_t(segment_mngr)
   {}

   //!Destructor. Deallocates all allocated blocks. Never throws
   ~shared_pool_impl()
   {}

   //!Allocates array of count elements. Can throw boost::interprocess::bad_alloc
   void *allocate_node()
   {
      //-----------------------
      boost::interprocess::scoped_lock<mutex_type> guard(m_header);
      //-----------------------
      return private_node_allocator_t::allocate_node();
   }
   
   //!Deallocates an array pointed by ptr. Never throws
   void deallocate_node(void *ptr)
   {
      //-----------------------
      boost::interprocess::scoped_lock<mutex_type> guard(m_header);
      //-----------------------
      private_node_allocator_t::deallocate_node(ptr);
   }

   //!Allocates a singly linked list of n nodes ending in null pointer.
   //!can throw boost::interprocess::bad_alloc
   void allocate_nodes(multiallocation_chain &nodes, std::size_t n)
   {
      //-----------------------
      boost::interprocess::scoped_lock<mutex_type> guard(m_header);
      //-----------------------
      return private_node_allocator_t::allocate_nodes(nodes, n);
   }

   //!Allocates n nodes, pointed by the multiallocation_iterator. 
   //!Can throw boost::interprocess::bad_alloc
   multiallocation_iterator allocate_nodes(const std::size_t n)
   {
      //-----------------------
      boost::interprocess::scoped_lock<mutex_type> guard(m_header);
      //-----------------------
      return private_node_allocator_t::allocate_nodes(n);
   }

   //!Deallocates a linked list of nodes ending in null pointer. Never throws
   void deallocate_nodes(multiallocation_chain &nodes, std::size_t num)
   {
      //-----------------------
      boost::interprocess::scoped_lock<mutex_type> guard(m_header);
      //-----------------------
      private_node_allocator_t::deallocate_nodes(nodes, num);
   }

   //!Deallocates a linked list of nodes ending in null pointer. Never throws
   void deallocate_nodes(multiallocation_chain &nodes)
   {
      //-----------------------
      boost::interprocess::scoped_lock<mutex_type> guard(m_header);
      //-----------------------
      private_node_allocator_t::deallocate_nodes(nodes);
   }

   //!Deallocates the nodes pointed by the multiallocation iterator. Never throws
   void deallocate_nodes(multiallocation_iterator it)
   {
      //-----------------------
      boost::interprocess::scoped_lock<mutex_type> guard(m_header);
      //-----------------------
      private_node_allocator_t::deallocate_nodes(it);
   }

   //!Deallocates all the free blocks of memory. Never throws
   void deallocate_free_blocks()
   {
      //-----------------------
      boost::interprocess::scoped_lock<mutex_type> guard(m_header);
      //-----------------------
      private_node_allocator_t::deallocate_free_blocks();
   }

   //!Deallocates all used memory from the common pool.
   //!Precondition: all nodes allocated from this pool should
   //!already be deallocated. Otherwise, undefined behavior. Never throws
   void purge_blocks()
   {
      //-----------------------
      boost::interprocess::scoped_lock<mutex_type> guard(m_header);
      //-----------------------
      private_node_allocator_t::purge_blocks();
   }

   //!Increments internal reference count and returns new count. Never throws
   std::size_t inc_ref_count()
   {
      //-----------------------
      boost::interprocess::scoped_lock<mutex_type> guard(m_header);
      //-----------------------
      return ++m_header.m_usecount;
   }

   //!Decrements internal reference count and returns new count. Never throws
   std::size_t dec_ref_count()
   {
      //-----------------------
      boost::interprocess::scoped_lock<mutex_type> guard(m_header);
      //-----------------------
      assert(m_header.m_usecount > 0);
      return --m_header.m_usecount;
   }

   //!Deprecated, use deallocate_free_blocks.
   void deallocate_free_chunks()
   {
      //-----------------------
      boost::interprocess::scoped_lock<mutex_type> guard(m_header);
      //-----------------------
      private_node_allocator_t::deallocate_free_blocks();
   }

   //!Deprecated, use purge_blocks.
   void purge_chunks()
   {
      //-----------------------
      boost::interprocess::scoped_lock<mutex_type> guard(m_header);
      //-----------------------
      private_node_allocator_t::purge_blocks();
   }

   private:
   //!This struct includes needed data and derives from
   //!interprocess_mutex to allow EBO when using null_mutex
   struct header_t : mutex_type
   {
      std::size_t m_usecount;    //Number of attached allocators

      header_t()
      :  m_usecount(0) {}
   } m_header;
};

}  //namespace detail {
}  //namespace interprocess {
}  //namespace boost {

#include <boost/interprocess/detail/config_end.hpp>

#endif   //#ifndef BOOST_INTERPROCESS_DETAIL_NODE_ALLOCATOR_COMMON_HPP
