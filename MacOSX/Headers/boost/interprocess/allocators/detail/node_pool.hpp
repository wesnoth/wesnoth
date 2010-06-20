//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2005-2008. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/interprocess for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_INTERPROCESS_DETAIL_NODE_POOL_HPP
#define BOOST_INTERPROCESS_DETAIL_NODE_POOL_HPP

#if (defined _MSC_VER) && (_MSC_VER >= 1200)
#  pragma once
#endif

#include <boost/interprocess/detail/config_begin.hpp>
#include <boost/interprocess/detail/workaround.hpp>

#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/detail/utilities.hpp>
#include <boost/interprocess/exceptions.hpp>
#include <boost/intrusive/slist.hpp>
#include <boost/math/common_factor_ct.hpp>
#include <boost/interprocess/detail/math_functions.hpp>
#include <boost/interprocess/detail/type_traits.hpp>
#include <boost/interprocess/allocators/detail/node_tools.hpp>
#include <boost/interprocess/allocators/detail/allocator_common.hpp>
#include <cstddef>
#include <functional>
#include <algorithm>
#include <cassert>

//!\file
//!Describes the real adaptive pool shared by many Interprocess adaptive pool allocators

namespace boost {
namespace interprocess {
namespace detail {

template<class SegmentManagerBase>
class private_node_pool_impl
{
   //Non-copyable
   private_node_pool_impl();
   private_node_pool_impl(const private_node_pool_impl &);
   private_node_pool_impl &operator=(const private_node_pool_impl &);

   //A node object will hold node_t when it's not allocated
   public:
   typedef typename SegmentManagerBase::void_pointer              void_pointer;
   typedef typename node_slist<void_pointer>::slist_hook_t        slist_hook_t;
   typedef typename node_slist<void_pointer>::node_t              node_t;
   typedef typename node_slist<void_pointer>::node_slist_t        free_nodes_t;
   typedef typename SegmentManagerBase::multiallocation_iterator  multiallocation_iterator;
   typedef typename SegmentManagerBase::multiallocation_chain     multiallocation_chain;

   private:
   typedef typename bi::make_slist
      < node_t, bi::base_hook<slist_hook_t>
      , bi::linear<true>
      , bi::constant_time_size<false> >::type      blockslist_t;
   public:

   //!Segment manager typedef
   typedef SegmentManagerBase segment_manager_base_type;

   //!Constructor from a segment manager. Never throws
   private_node_pool_impl(segment_manager_base_type *segment_mngr_base, std::size_t node_size, std::size_t nodes_per_block)
   :  m_nodes_per_block(nodes_per_block)
   ,  m_real_node_size(detail::lcm(node_size, std::size_t(alignment_of<node_t>::value)))
      //General purpose allocator
   ,  mp_segment_mngr_base(segment_mngr_base)
   ,  m_blocklist()
   ,  m_freelist()
      //Debug node count
   ,  m_allocated(0)
   {}

   //!Destructor. Deallocates all allocated blocks. Never throws
   ~private_node_pool_impl()
   {  this->purge_blocks();  }

   std::size_t get_real_num_node() const
   {  return m_nodes_per_block; }

   //!Returns the segment manager. Never throws
   segment_manager_base_type* get_segment_manager_base()const
   {  return detail::get_pointer(mp_segment_mngr_base);  }

   //!Allocates array of count elements. Can throw boost::interprocess::bad_alloc
   void *allocate_node()
   {
      //If there are no free nodes we allocate a new block
      if (m_freelist.empty())
         priv_alloc_block();
      //We take the first free node
      node_t *n = &m_freelist.front();
      m_freelist.pop_front();
      ++m_allocated;
      return n;
   }
   
   //!Deallocates an array pointed by ptr. Never throws
   void deallocate_node(void *ptr)
   {
      //We put the node at the beginning of the free node list
      node_t * to_deallocate = static_cast<node_t*>(ptr);
      m_freelist.push_front(*to_deallocate);
      assert(m_allocated>0);
      --m_allocated;
   }

   //!Allocates a singly linked list of n nodes ending in null pointer and pushes them in the chain. 
   //!can throw boost::interprocess::bad_alloc
   void allocate_nodes(multiallocation_chain &nodes, const std::size_t n)
   {
      std::size_t i = 0;
      try{
         for(; i < n; ++i){
            nodes.push_front(this->allocate_node());
         }
      }
      catch(...){
         this->deallocate_nodes(nodes, i);
         throw;
      }
   }

   //!Allocates a singly linked list of n nodes ending in null pointer 
   //!can throw boost::interprocess::bad_alloc
   multiallocation_iterator allocate_nodes(const std::size_t n)
   {
      multiallocation_chain nodes;
      std::size_t i = 0;
      try{
         for(; i < n; ++i){
            nodes.push_front(this->allocate_node());
         }
      }
      catch(...){
         this->deallocate_nodes(nodes, i);
         throw;
      }
      return nodes.get_it();
   }

   //!Deallocates a linked list of nodes. Never throws
   void deallocate_nodes(multiallocation_chain &nodes)
   {
      this->deallocate_nodes(nodes.get_it());
      nodes.reset();
   }

   //!Deallocates the first n nodes of a linked list of nodes. Never throws
   void deallocate_nodes(multiallocation_chain &nodes, std::size_t num)
   {
      assert(nodes.size() >= num);
      for(std::size_t i = 0; i < num; ++i){
         deallocate_node(nodes.pop_front());
      }
   }

   //!Deallocates the nodes pointed by the multiallocation iterator. Never throws
   void deallocate_nodes(multiallocation_iterator it)
   {
      multiallocation_iterator itend;
      while(it != itend){
         void *addr = &*it;
         ++it;
         deallocate_node(addr);
      }
   }

   //!Deallocates all the free blocks of memory. Never throws
   void deallocate_free_blocks()
   {
      typedef typename free_nodes_t::iterator nodelist_iterator;
      typename blockslist_t::iterator bit(m_blocklist.before_begin()),
                                      it(m_blocklist.begin()),
                                      itend(m_blocklist.end());
      free_nodes_t backup_list;
      nodelist_iterator backup_list_last = backup_list.before_begin();

      //Execute the algorithm and get an iterator to the last value
      std::size_t blocksize = detail::get_rounded_size
         (m_real_node_size*m_nodes_per_block, alignment_of<node_t>::value);

      while(it != itend){
         //Collect all the nodes from the block pointed by it
         //and push them in the list
         free_nodes_t free_nodes;
         nodelist_iterator last_it = free_nodes.before_begin();
         const void *addr = get_block_from_hook(&*it, blocksize);

         m_freelist.remove_and_dispose_if
            (is_between(addr, blocksize), push_in_list(free_nodes, last_it));

         //If the number of nodes is equal to m_nodes_per_block
         //this means that the block can be deallocated
         if(free_nodes.size() == m_nodes_per_block){
            //Unlink the nodes
            free_nodes.clear();
            it = m_blocklist.erase_after(bit);
            mp_segment_mngr_base->deallocate((void*)addr);
         }
         //Otherwise, insert them in the backup list, since the
         //next "remove_if" does not need to check them again.
         else{
            //Assign the iterator to the last value if necessary
            if(backup_list.empty() && !m_freelist.empty()){
               backup_list_last = last_it;
            }
            //Transfer nodes. This is constant time.
            backup_list.splice_after
               ( backup_list.before_begin()
               , free_nodes
               , free_nodes.before_begin()
               , last_it
               , free_nodes.size());
            bit = it;
            ++it;
         }
      }
      //We should have removed all the nodes from the free list
      assert(m_freelist.empty());

      //Now pass all the node to the free list again
      m_freelist.splice_after
         ( m_freelist.before_begin()
         , backup_list
         , backup_list.before_begin()
         , backup_list_last
         , backup_list.size());
   }

   std::size_t num_free_nodes()
   {  return m_freelist.size();  }

   //!Deallocates all used memory. Precondition: all nodes allocated from this pool should
   //!already be deallocated. Otherwise, undefined behaviour. Never throws
   void purge_blocks()
   {
      //check for memory leaks
      assert(m_allocated==0);
      std::size_t blocksize = detail::get_rounded_size
         (m_real_node_size*m_nodes_per_block, alignment_of<node_t>::value);
      typename blockslist_t::iterator
         it(m_blocklist.begin()), itend(m_blocklist.end()), aux;

      //We iterate though the NodeBlock list to free the memory
      while(!m_blocklist.empty()){
         void *addr = get_block_from_hook(&m_blocklist.front(), blocksize);
         m_blocklist.pop_front();
         mp_segment_mngr_base->deallocate((void*)addr);
      }
      //Just clear free node list
      m_freelist.clear();
   }

   void swap(private_node_pool_impl &other)
   {
      std::swap(mp_segment_mngr_base, other.mp_segment_mngr_base);
      m_blocklist.swap(other.m_blocklist);
      m_freelist.swap(other.m_freelist);
      std::swap(m_allocated, other.m_allocated);
   }

   private:

   struct push_in_list
   {
      push_in_list(free_nodes_t &l, typename free_nodes_t::iterator &it)
         :  slist_(l), last_it_(it)
      {}
      
      void operator()(typename free_nodes_t::pointer p) const
      {
         slist_.push_front(*p);
         if(slist_.size() == 1){ //Cache last element
            ++last_it_ = slist_.begin();
         }
      }

      private:
      free_nodes_t &slist_;
      typename free_nodes_t::iterator &last_it_;
   };

   struct is_between
      :  std::unary_function<typename free_nodes_t::value_type, bool>
   {
      is_between(const void *addr, std::size_t size)
         :  beg_(static_cast<const char *>(addr)), end_(beg_+size)
      {}
      
      bool operator()(typename free_nodes_t::const_reference v) const
      {
         return (beg_ <= reinterpret_cast<const char *>(&v) && 
                 end_ >  reinterpret_cast<const char *>(&v));
      }
      private:
      const char *      beg_;
      const char *      end_;
   };

   //!Allocates a block of nodes. Can throw boost::interprocess::bad_alloc
   void priv_alloc_block()
   {
      //We allocate a new NodeBlock and put it as first
      //element in the free Node list
      std::size_t blocksize = 
         detail::get_rounded_size(m_real_node_size*m_nodes_per_block, alignment_of<node_t>::value);
      char *pNode = reinterpret_cast<char*>
         (mp_segment_mngr_base->allocate(blocksize + sizeof(node_t)));
      if(!pNode)  throw bad_alloc();
      char *pBlock = pNode;
      m_blocklist.push_front(get_block_hook(pBlock, blocksize));

      //We initialize all Nodes in Node Block to insert 
      //them in the free Node list
      for(std::size_t i = 0; i < m_nodes_per_block; ++i, pNode += m_real_node_size){
         m_freelist.push_front(*new (pNode) node_t);
      }
   }

   //!Deprecated, use deallocate_free_blocks
   void deallocate_free_chunks()
   {  this->deallocate_free_blocks(); }

   //!Deprecated, use purge_blocks
   void purge_chunks()
   {  this->purge_blocks(); }

   private:
   //!Returns a reference to the block hook placed in the end of the block
   static inline node_t & get_block_hook (void *block, std::size_t blocksize)
   {  
      return *reinterpret_cast<node_t*>(reinterpret_cast<char*>(block) + blocksize);  
   }

   //!Returns the starting address of the block reference to the block hook placed in the end of the block
   inline void *get_block_from_hook (node_t *hook, std::size_t blocksize)
   {  
      return (reinterpret_cast<char*>(hook) - blocksize);
   }

   private:
   typedef typename pointer_to_other
      <void_pointer, segment_manager_base_type>::type   segment_mngr_base_ptr_t;

   const std::size_t m_nodes_per_block;
   const std::size_t m_real_node_size;
   segment_mngr_base_ptr_t mp_segment_mngr_base;   //Segment manager
   blockslist_t      m_blocklist;      //Intrusive container of blocks
   free_nodes_t      m_freelist;       //Intrusive container of free nods
   std::size_t       m_allocated;      //Used nodes for debugging
};


//!Pooled shared memory allocator using single segregated storage. Includes
//!a reference count but the class does not delete itself, this is  
//!responsibility of user classes. Node size (NodeSize) and the number of
//!nodes allocated per block (NodesPerBlock) are known at compile time
template< class SegmentManager, std::size_t NodeSize, std::size_t NodesPerBlock >
class private_node_pool
   //Inherit from the implementation to avoid template bloat
   :  public private_node_pool_impl<typename SegmentManager::segment_manager_base_type>
{
   typedef private_node_pool_impl<typename SegmentManager::segment_manager_base_type> base_t;
   //Non-copyable
   private_node_pool();
   private_node_pool(const private_node_pool &);
   private_node_pool &operator=(const private_node_pool &);

   public:
   typedef SegmentManager segment_manager;

   static const std::size_t nodes_per_block = NodesPerBlock;
   //Deprecated, use nodes_per_block
   static const std::size_t nodes_per_chunk = NodesPerBlock;

   //!Constructor from a segment manager. Never throws
   private_node_pool(segment_manager *segment_mngr)
      :  base_t(segment_mngr, NodeSize, NodesPerBlock)
   {}

   //!Returns the segment manager. Never throws
   segment_manager* get_segment_manager() const
   {  return static_cast<segment_manager*>(base_t::get_segment_manager_base()); }
};


//!Pooled shared memory allocator using single segregated storage. Includes
//!a reference count but the class does not delete itself, this is  
//!responsibility of user classes. Node size (NodeSize) and the number of
//!nodes allocated per block (NodesPerBlock) are known at compile time
//!Pooled shared memory allocator using adaptive pool. Includes
//!a reference count but the class does not delete itself, this is  
//!responsibility of user classes. Node size (NodeSize) and the number of
//!nodes allocated per block (NodesPerBlock) are known at compile time
template< class SegmentManager
        , std::size_t NodeSize
        , std::size_t NodesPerBlock
        >
class shared_node_pool 
   :  public detail::shared_pool_impl
      < private_node_pool
         <SegmentManager, NodeSize, NodesPerBlock>
      >
{
   typedef detail::shared_pool_impl
      < private_node_pool
         <SegmentManager, NodeSize, NodesPerBlock>
      > base_t;
   public:
   shared_node_pool(SegmentManager *segment_mgnr)
      : base_t(segment_mgnr)
   {}
};

}  //namespace detail {
}  //namespace interprocess {
}  //namespace boost {

#include <boost/interprocess/detail/config_end.hpp>

#endif   //#ifndef BOOST_INTERPROCESS_DETAIL_NODE_POOL_HPP
