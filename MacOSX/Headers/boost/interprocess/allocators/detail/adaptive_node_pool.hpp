//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2005-2008. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/interprocess for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_INTERPROCESS_DETAIL_ADAPTIVE_NODE_POOL_HPP
#define BOOST_INTERPROCESS_DETAIL_ADAPTIVE_NODE_POOL_HPP

#if (defined _MSC_VER) && (_MSC_VER >= 1200)
#  pragma once
#endif

#include <boost/interprocess/detail/config_begin.hpp>
#include <boost/interprocess/detail/workaround.hpp>
#include <boost/interprocess/interprocess_fwd.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/detail/utilities.hpp>
#include <boost/interprocess/detail/min_max.hpp>
#include <boost/interprocess/detail/math_functions.hpp>
#include <boost/interprocess/exceptions.hpp>
#include <boost/intrusive/set.hpp>
#include <boost/intrusive/slist.hpp>
#include <boost/math/common_factor_ct.hpp>
#include <boost/interprocess/detail/type_traits.hpp>
#include <boost/interprocess/allocators/detail/node_tools.hpp>
#include <boost/interprocess/allocators/detail/allocator_common.hpp>
#include <cstddef>
#include <boost/config/no_tr1/cmath.hpp>
#include <cassert>

//!\file
//!Describes the real adaptive pool shared by many Interprocess pool allocators

namespace boost {
namespace interprocess {
namespace detail {

template<class SegmentManagerBase>
class private_adaptive_node_pool_impl
{
   //Non-copyable
   private_adaptive_node_pool_impl();
   private_adaptive_node_pool_impl(const private_adaptive_node_pool_impl &);
   private_adaptive_node_pool_impl &operator=(const private_adaptive_node_pool_impl &);

   typedef typename SegmentManagerBase::void_pointer void_pointer;
   static const std::size_t PayloadPerAllocation = SegmentManagerBase::PayloadPerAllocation;
   public:
   typedef typename node_slist<void_pointer>::node_t node_t;
   typedef typename node_slist<void_pointer>::node_slist_t free_nodes_t;
   typedef typename SegmentManagerBase::multiallocation_iterator  multiallocation_iterator;
   typedef typename SegmentManagerBase::multiallocation_chain     multiallocation_chain;

   private:
   typedef typename bi::make_set_base_hook
      < bi::void_pointer<void_pointer>
      , bi::optimize_size<true>
      , bi::constant_time_size<false>
      , bi::link_mode<bi::normal_link> >::type multiset_hook_t;

   struct hdr_offset_holder
   {
      hdr_offset_holder(std::size_t offset = 0)
         : hdr_offset(offset)
      {}
      std::size_t hdr_offset;
   };

   struct block_info_t
      :  
         public hdr_offset_holder,
         public multiset_hook_t
   {
      //An intrusive list of free node from this block
      free_nodes_t free_nodes;
      friend bool operator <(const block_info_t &l, const block_info_t &r)
      {
//      {  return l.free_nodes.size() < r.free_nodes.size();   }
         //Let's order blocks first by free nodes and then by address
         //so that highest address fully free blocks are deallocated.
         //This improves returning memory to the OS (trimming).
         const bool is_less  = l.free_nodes.size() < r.free_nodes.size();
         const bool is_equal = l.free_nodes.size() == r.free_nodes.size();
         return is_less || (is_equal && (&l < &r));
      }
   };
   typedef typename bi::make_multiset
      <block_info_t, bi::base_hook<multiset_hook_t> >::type  block_multiset_t;
   typedef typename block_multiset_t::iterator               block_iterator;

   static const std::size_t MaxAlign = alignment_of<node_t>::value;
   static const std::size_t HdrSize  = ((sizeof(block_info_t)-1)/MaxAlign+1)*MaxAlign;
   static const std::size_t HdrOffsetSize = ((sizeof(hdr_offset_holder)-1)/MaxAlign+1)*MaxAlign;
   static std::size_t calculate_alignment
      (std::size_t overhead_percent, std::size_t real_node_size)
   {
      //to-do: handle real_node_size != node_size
      const std::size_t divisor  = overhead_percent*real_node_size;
      const std::size_t dividend = HdrOffsetSize*100;
      std::size_t elements_per_subblock = (dividend - 1)/divisor + 1;
      std::size_t candidate_power_of_2 = 
         upper_power_of_2(elements_per_subblock*real_node_size + HdrOffsetSize);
      bool overhead_satisfied = false;
      //Now calculate the wors-case overhead for a subblock
      const std::size_t max_subblock_overhead  = HdrSize + PayloadPerAllocation;
      while(!overhead_satisfied){
         elements_per_subblock = (candidate_power_of_2 - max_subblock_overhead)/real_node_size;
         const std::size_t overhead_size = candidate_power_of_2 - elements_per_subblock*real_node_size;
         if(overhead_size*100/candidate_power_of_2 < overhead_percent){
            overhead_satisfied = true;
         }
         else{
            candidate_power_of_2 <<= 1;
         }
      }
      return candidate_power_of_2;
   }

   static void calculate_num_subblocks
      (std::size_t alignment, std::size_t real_node_size, std::size_t elements_per_block
      ,std::size_t &num_subblocks, std::size_t &real_num_node, std::size_t overhead_percent)
   {
      std::size_t elements_per_subblock = (alignment - HdrOffsetSize)/real_node_size;
      std::size_t possible_num_subblock = (elements_per_block - 1)/elements_per_subblock + 1;
      std::size_t hdr_subblock_elements   = (alignment - HdrSize - PayloadPerAllocation)/real_node_size;
      while(((possible_num_subblock-1)*elements_per_subblock + hdr_subblock_elements) < elements_per_block){
         ++possible_num_subblock;
      }
      elements_per_subblock = (alignment - HdrOffsetSize)/real_node_size;
      bool overhead_satisfied = false;
      while(!overhead_satisfied){
         const std::size_t total_data = (elements_per_subblock*(possible_num_subblock-1) + hdr_subblock_elements)*real_node_size;
         const std::size_t total_size = alignment*possible_num_subblock;
         if((total_size - total_data)*100/total_size < overhead_percent){
            overhead_satisfied = true;
         }
         else{
            ++possible_num_subblock;
         }
      }
      num_subblocks = possible_num_subblock;
      real_num_node = (possible_num_subblock-1)*elements_per_subblock + hdr_subblock_elements;
   }

   public:
   //!Segment manager typedef
   typedef SegmentManagerBase                 segment_manager_base_type;

   //!Constructor from a segment manager. Never throws
   private_adaptive_node_pool_impl
      ( segment_manager_base_type *segment_mngr_base, std::size_t node_size
      , std::size_t nodes_per_block, std::size_t max_free_blocks
      , unsigned char overhead_percent
      )
   :  m_max_free_blocks(max_free_blocks)
   ,  m_real_node_size(lcm(node_size, std::size_t(alignment_of<node_t>::value)))
   //Round the size to a power of two value.
   //This is the total memory size (including payload) that we want to
   //allocate from the general-purpose allocator
   ,  m_real_block_alignment(calculate_alignment(overhead_percent, m_real_node_size))
      //This is the real number of nodes per block
   ,  m_num_subblocks(0)
   ,  m_real_num_node(0)
      //General purpose allocator
   ,  mp_segment_mngr_base(segment_mngr_base)
   ,  m_block_multiset()
   ,  m_totally_free_blocks(0)
   {
      calculate_num_subblocks(m_real_block_alignment, m_real_node_size, nodes_per_block, m_num_subblocks, m_real_num_node, overhead_percent);
   }

   //!Destructor. Deallocates all allocated blocks. Never throws
   ~private_adaptive_node_pool_impl()
   {  priv_clear();  }

   std::size_t get_real_num_node() const
   {  return m_real_num_node; }

   //!Returns the segment manager. Never throws
   segment_manager_base_type* get_segment_manager_base()const
   {  return detail::get_pointer(mp_segment_mngr_base);  }

   //!Allocates array of count elements. Can throw boost::interprocess::bad_alloc
   void *allocate_node()
   {
      priv_invariants();
      //If there are no free nodes we allocate a new block
      if (m_block_multiset.empty()){ 
         priv_alloc_block(1);
      }
      //We take the first free node the multiset can't be empty
      return priv_take_first_node();
   }

   //!Deallocates an array pointed by ptr. Never throws
   void deallocate_node(void *pElem)
   {
      this->priv_reinsert_nodes_in_block
         (multiallocation_iterator::create_simple_range(pElem));
      //Update free block count
      if(m_totally_free_blocks > m_max_free_blocks){
         this->priv_deallocate_free_blocks(m_max_free_blocks);
      }
      priv_invariants();
   }

   //!Allocates a singly linked list of n nodes ending in null pointer. 
   //!can throw boost::interprocess::bad_alloc
   void allocate_nodes(multiallocation_chain &nodes, const std::size_t n)
   {
      try{
         priv_invariants();
         std::size_t i = 0;
         while(i != n){
            //If there are no free nodes we allocate all needed blocks
            if (m_block_multiset.empty()){
               priv_alloc_block(((n - i) - 1)/m_real_num_node + 1);
            }
            free_nodes_t &free_nodes = m_block_multiset.begin()->free_nodes;
            const std::size_t free_nodes_count_before = free_nodes.size();
            if(free_nodes_count_before == m_real_num_node){
               --m_totally_free_blocks;
            }
            const std::size_t num_elems = ((n-i) < free_nodes_count_before) ? (n-i) : free_nodes_count_before;
            for(std::size_t j = 0; j != num_elems; ++j){
               void *new_node = &free_nodes.front();
               free_nodes.pop_front();
               nodes.push_back(new_node);
            }
            
            if(free_nodes.empty()){
               m_block_multiset.erase(m_block_multiset.begin());
            }
            i += num_elems;
         }
      }
      catch(...){
         this->deallocate_nodes(nodes, nodes.size());
         throw;
      }
      priv_invariants();
   }

   //!Allocates n nodes, pointed by the multiallocation_iterator. 
   //!Can throw boost::interprocess::bad_alloc
   multiallocation_iterator allocate_nodes(const std::size_t n)
   {
      multiallocation_chain chain;
      this->allocate_nodes(chain, n);
      return chain.get_it();
   }

   //!Deallocates a linked list of nodes. Never throws
   void deallocate_nodes(multiallocation_chain &nodes)
   {
      this->deallocate_nodes(nodes.get_it());
      nodes.reset();
   }

   //!Deallocates the first n nodes of a linked list of nodes. Never throws
   void deallocate_nodes(multiallocation_chain &nodes, std::size_t n)
   {
      assert(nodes.size() >= n);
      for(std::size_t i = 0; i < n; ++i){
         this->deallocate_node(nodes.pop_front());
      }
   }

   //!Deallocates the nodes pointed by the multiallocation iterator. Never throws
   void deallocate_nodes(multiallocation_iterator it)
   {
      this->priv_reinsert_nodes_in_block(it);
      if(m_totally_free_blocks > m_max_free_blocks){
         this->priv_deallocate_free_blocks(m_max_free_blocks);
      }
   }

   void deallocate_free_blocks()
   {  this->priv_deallocate_free_blocks(0);   }

   std::size_t num_free_nodes()
   {
      typedef typename block_multiset_t::const_iterator citerator;
      std::size_t count = 0;
      citerator it (m_block_multiset.begin()), itend(m_block_multiset.end());
      for(; it != itend; ++it){
         count += it->free_nodes.size();
      }
      return count;
   }

   void swap(private_adaptive_node_pool_impl &other)
   {
      assert(m_max_free_blocks == other.m_max_free_blocks);
      assert(m_real_node_size == other.m_real_node_size);
      assert(m_real_block_alignment == other.m_real_block_alignment);
      assert(m_real_num_node == other.m_real_num_node);
      std::swap(mp_segment_mngr_base, other.mp_segment_mngr_base);
      std::swap(m_totally_free_blocks, other.m_totally_free_blocks);
      m_block_multiset.swap(other.m_block_multiset);
   }

   //Deprecated, use deallocate_free_blocks
   void deallocate_free_chunks()
   {  this->priv_deallocate_free_blocks(0);   }

   private:
   void priv_deallocate_free_blocks(std::size_t max_free_blocks)
   {
      priv_invariants();
      //Now check if we've reached the free nodes limit
      //and check if we have free blocks. If so, deallocate as much
      //as we can to stay below the limit
      for( block_iterator itend = m_block_multiset.end()
         ; m_totally_free_blocks > max_free_blocks
         ; --m_totally_free_blocks
         ){
         assert(!m_block_multiset.empty());
         block_iterator it = itend;
         --it;
         std::size_t num_nodes = it->free_nodes.size();
         assert(num_nodes == m_real_num_node);
         (void)num_nodes;
         m_block_multiset.erase_and_dispose
            (it, block_destroyer(this));
      }
   }

   void priv_reinsert_nodes_in_block(multiallocation_iterator it)
   {
      multiallocation_iterator itend;
      block_iterator block_it(m_block_multiset.end());
      while(it != itend){
         void *pElem = &*it;
         ++it;
         priv_invariants();
         block_info_t *block_info = this->priv_block_from_node(pElem);
         assert(block_info->free_nodes.size() < m_real_num_node);
         //We put the node at the beginning of the free node list
         node_t * to_deallocate = static_cast<node_t*>(pElem);
         block_info->free_nodes.push_front(*to_deallocate);

         block_iterator this_block(block_multiset_t::s_iterator_to(*block_info));
         block_iterator next_block(this_block);
         ++next_block;

         //Cache the free nodes from the block
         std::size_t this_block_free_nodes = this_block->free_nodes.size();

         if(this_block_free_nodes == 1){
            m_block_multiset.insert(m_block_multiset.begin(), *block_info);
         }
         else{
            block_iterator next_block(this_block);
            ++next_block;
            if(next_block != block_it){
               std::size_t next_free_nodes = next_block->free_nodes.size();
               if(this_block_free_nodes > next_free_nodes){
                  //Now move the block to the new position
                  m_block_multiset.erase(this_block);
                  m_block_multiset.insert(*block_info);
               }
            }
         }
         //Update free block count
         if(this_block_free_nodes == m_real_num_node){
            ++m_totally_free_blocks;
         }
         priv_invariants();
      }
   }

   node_t *priv_take_first_node()
   {
      assert(m_block_multiset.begin() != m_block_multiset.end());
      //We take the first free node the multiset can't be empty
      free_nodes_t &free_nodes = m_block_multiset.begin()->free_nodes;
      node_t *first_node = &free_nodes.front();
      const std::size_t free_nodes_count = free_nodes.size();
      assert(0 != free_nodes_count);
      free_nodes.pop_front();
      if(free_nodes_count == 1){
         m_block_multiset.erase(m_block_multiset.begin());
      }
      else if(free_nodes_count == m_real_num_node){
         --m_totally_free_blocks;
      }
      priv_invariants();
      return first_node;
   }

   class block_destroyer;
   friend class block_destroyer;

   class block_destroyer
   {
      public:
      block_destroyer(const private_adaptive_node_pool_impl *impl)
         :  mp_impl(impl)
      {}

      void operator()(typename block_multiset_t::pointer to_deallocate)
      {
         std::size_t free_nodes = to_deallocate->free_nodes.size();
         (void)free_nodes;
         assert(free_nodes == mp_impl->m_real_num_node);
         assert(0 == to_deallocate->hdr_offset);
         hdr_offset_holder *hdr_off_holder = mp_impl->priv_first_subblock_from_block(detail::get_pointer(to_deallocate));
         mp_impl->mp_segment_mngr_base->deallocate(hdr_off_holder);
      }
      const private_adaptive_node_pool_impl *mp_impl;
   };

   //This macro will activate invariant checking. Slow, but helpful for debugging the code.
   //#define BOOST_INTERPROCESS_ADAPTIVE_NODE_POOL_CHECK_INVARIANTS
   void priv_invariants()
   #ifdef BOOST_INTERPROCESS_ADAPTIVE_NODE_POOL_CHECK_INVARIANTS
   #undef BOOST_INTERPROCESS_ADAPTIVE_NODE_POOL_CHECK_INVARIANTS
   {
      //We iterate through the block list to free the memory
      block_iterator it(m_block_multiset.begin()), 
                     itend(m_block_multiset.end()), to_deallocate;
      if(it != itend){
         for(++it; it != itend; ++it){
            block_iterator prev(it);
            --prev;
            std::size_t sp = prev->free_nodes.size(),
                        si = it->free_nodes.size();
            assert(sp <= si);
            (void)sp;   (void)si;
         }
      }

      {
         //Check that the total free nodes are correct
         it    = m_block_multiset.begin();
         itend = m_block_multiset.end();
         std::size_t total_free_nodes = 0;
         for(; it != itend; ++it){
            total_free_nodes += it->free_nodes.size();
         }
         assert(total_free_nodes >= m_totally_free_blocks*m_real_num_node);
      }

      {
         //Check that the total totally free blocks are correct
         it    = m_block_multiset.begin();
         itend = m_block_multiset.end();
         std::size_t total_free_blocks = 0;
         for(; it != itend; ++it){
            total_free_blocks += (it->free_nodes.size() == m_real_num_node);
         }
         assert(total_free_blocks == m_totally_free_blocks);
      }
      {
      //Check that header offsets are correct
      it = m_block_multiset.begin();
      for(; it != itend; ++it){
         hdr_offset_holder *hdr_off_holder = priv_first_subblock_from_block(&*it);
         for(std::size_t i = 0, max = m_num_subblocks; i < max; ++i){
            assert(hdr_off_holder->hdr_offset == std::size_t(reinterpret_cast<char*>(&*it)- reinterpret_cast<char*>(hdr_off_holder)));
            assert(0 == ((std::size_t)hdr_off_holder & (m_real_block_alignment - 1)));
            assert(0 == (hdr_off_holder->hdr_offset & (m_real_block_alignment - 1)));
            hdr_off_holder = reinterpret_cast<hdr_offset_holder *>(reinterpret_cast<char*>(hdr_off_holder) + m_real_block_alignment);
         }
      }
      }
   }
   #else
   {} //empty
   #endif

   //!Deallocates all used memory. Never throws
   void priv_clear()
   {
      #ifndef NDEBUG
      block_iterator it    = m_block_multiset.begin();
      block_iterator itend = m_block_multiset.end();
      std::size_t num_free_nodes = 0;
      for(; it != itend; ++it){
         //Check for memory leak
         assert(it->free_nodes.size() == m_real_num_node);
         ++num_free_nodes;
      }
      assert(num_free_nodes == m_totally_free_blocks);
      #endif
      priv_invariants();
      m_block_multiset.clear_and_dispose
         (block_destroyer(this));
      m_totally_free_blocks = 0;
   }

   block_info_t *priv_block_from_node(void *node) const
   {
      hdr_offset_holder *hdr_off_holder =
         reinterpret_cast<hdr_offset_holder*>((std::size_t)node & std::size_t(~(m_real_block_alignment - 1)));
      assert(0 == ((std::size_t)hdr_off_holder & (m_real_block_alignment - 1)));
      assert(0 == (hdr_off_holder->hdr_offset & (m_real_block_alignment - 1)));
      block_info_t *block = reinterpret_cast<block_info_t *>
         (reinterpret_cast<char*>(hdr_off_holder) + hdr_off_holder->hdr_offset);
      assert(block->hdr_offset == 0);
      return block;
   }

   hdr_offset_holder *priv_first_subblock_from_block(block_info_t *block) const
   {
      hdr_offset_holder *hdr_off_holder = reinterpret_cast<hdr_offset_holder*>
            (reinterpret_cast<char*>(block) - (m_num_subblocks-1)*m_real_block_alignment);
      assert(hdr_off_holder->hdr_offset == std::size_t(reinterpret_cast<char*>(block) - reinterpret_cast<char*>(hdr_off_holder)));
      assert(0 == ((std::size_t)hdr_off_holder & (m_real_block_alignment - 1)));
      assert(0 == (hdr_off_holder->hdr_offset & (m_real_block_alignment - 1)));
      return hdr_off_holder;
   }

   //!Allocates a several blocks of nodes. Can throw boost::interprocess::bad_alloc
   void priv_alloc_block(std::size_t n)
   {
      std::size_t real_block_size = m_real_block_alignment*m_num_subblocks - SegmentManagerBase::PayloadPerAllocation;
      std::size_t elements_per_subblock = (m_real_block_alignment - HdrOffsetSize)/m_real_node_size;
      std::size_t hdr_subblock_elements   = (m_real_block_alignment - HdrSize - SegmentManagerBase::PayloadPerAllocation)/m_real_node_size;

      for(std::size_t i = 0; i != n; ++i){
         //We allocate a new NodeBlock and put it the last
         //element of the tree
         char *mem_address = static_cast<char*>
            (mp_segment_mngr_base->allocate_aligned(real_block_size, m_real_block_alignment));
         if(!mem_address)   throw std::bad_alloc();
         ++m_totally_free_blocks;

         //First initialize header information on the last subblock
         char *hdr_addr = mem_address + m_real_block_alignment*(m_num_subblocks-1);
         block_info_t *c_info = new(hdr_addr)block_info_t;
         //Some structural checks
         assert(static_cast<void*>(&static_cast<hdr_offset_holder*>(c_info)->hdr_offset) ==
                static_cast<void*>(c_info));
         typename free_nodes_t::iterator prev_insert_pos = c_info->free_nodes.before_begin();
         for( std::size_t subblock = 0, maxsubblock = m_num_subblocks - 1
            ; subblock < maxsubblock
            ; ++subblock, mem_address += m_real_block_alignment){
            //Initialize header offset mark
            new(mem_address) hdr_offset_holder(std::size_t(hdr_addr - mem_address));
            char *pNode = mem_address + HdrOffsetSize;
            for(std::size_t i = 0; i < elements_per_subblock; ++i){
               prev_insert_pos = c_info->free_nodes.insert_after(prev_insert_pos, *new (pNode) node_t);
               pNode   += m_real_node_size;
            }
         }
         {
            char *pNode = hdr_addr + HdrSize;
            //We initialize all Nodes in Node Block to insert 
            //them in the free Node list
            for(std::size_t i = 0; i < hdr_subblock_elements; ++i){
               prev_insert_pos = c_info->free_nodes.insert_after(prev_insert_pos, *new (pNode) node_t);
               pNode   += m_real_node_size;
            }
         }
         //Insert the block after the free node list is full
         m_block_multiset.insert(m_block_multiset.end(), *c_info);
      }
   }

   private:
   typedef typename pointer_to_other
      <void_pointer, segment_manager_base_type>::type   segment_mngr_base_ptr_t;

   const std::size_t m_max_free_blocks;
   const std::size_t m_real_node_size;
   //Round the size to a power of two value.
   //This is the total memory size (including payload) that we want to
   //allocate from the general-purpose allocator
   const std::size_t m_real_block_alignment;
   std::size_t m_num_subblocks;
   //This is the real number of nodes per block
   //const
   std::size_t m_real_num_node;
   segment_mngr_base_ptr_t                mp_segment_mngr_base;//Segment manager
   block_multiset_t                       m_block_multiset;    //Intrusive block list
   std::size_t                            m_totally_free_blocks;       //Free blocks
};

template< class SegmentManager
        , std::size_t NodeSize
        , std::size_t NodesPerBlock
        , std::size_t MaxFreeBlocks
        , unsigned char OverheadPercent
        >
class private_adaptive_node_pool
   :  public private_adaptive_node_pool_impl
         <typename SegmentManager::segment_manager_base_type>
{
   typedef private_adaptive_node_pool_impl
      <typename SegmentManager::segment_manager_base_type> base_t;
   //Non-copyable
   private_adaptive_node_pool();
   private_adaptive_node_pool(const private_adaptive_node_pool &);
   private_adaptive_node_pool &operator=(const private_adaptive_node_pool &);

   public:
   typedef SegmentManager segment_manager;

   static const std::size_t nodes_per_block = NodesPerBlock;

   //Deprecated, use node_per_block
   static const std::size_t nodes_per_chunk = NodesPerBlock;

   //!Constructor from a segment manager. Never throws
   private_adaptive_node_pool(segment_manager *segment_mngr)
      :  base_t(segment_mngr, NodeSize, NodesPerBlock, MaxFreeBlocks, OverheadPercent)
   {}

   //!Returns the segment manager. Never throws
   segment_manager* get_segment_manager() const
   {  return static_cast<segment_manager*>(base_t::get_segment_manager_base()); }
};

//!Pooled shared memory allocator using adaptive pool. Includes
//!a reference count but the class does not delete itself, this is  
//!responsibility of user classes. Node size (NodeSize) and the number of
//!nodes allocated per block (NodesPerBlock) are known at compile time
template< class SegmentManager
        , std::size_t NodeSize
        , std::size_t NodesPerBlock
        , std::size_t MaxFreeBlocks
        , unsigned char OverheadPercent
        >
class shared_adaptive_node_pool 
   :  public detail::shared_pool_impl
      < private_adaptive_node_pool
         <SegmentManager, NodeSize, NodesPerBlock, MaxFreeBlocks, OverheadPercent>
      >
{
   typedef detail::shared_pool_impl
      < private_adaptive_node_pool
         <SegmentManager, NodeSize, NodesPerBlock, MaxFreeBlocks, OverheadPercent>
      > base_t;
   public:
   shared_adaptive_node_pool(SegmentManager *segment_mgnr)
      : base_t(segment_mgnr)
   {}
};

}  //namespace detail {
}  //namespace interprocess {
}  //namespace boost {

#include <boost/interprocess/detail/config_end.hpp>

#endif   //#ifndef BOOST_INTERPROCESS_DETAIL_ADAPTIVE_NODE_POOL_HPP

