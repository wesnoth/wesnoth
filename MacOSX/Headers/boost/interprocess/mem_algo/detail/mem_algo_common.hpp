//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2005-2008. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/interprocess for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_INTERPROCESS_DETAIL_MEM_ALGO_COMMON_HPP
#define BOOST_INTERPROCESS_DETAIL_MEM_ALGO_COMMON_HPP

#if (defined _MSC_VER) && (_MSC_VER >= 1200)
#  pragma once
#endif

#include <boost/interprocess/detail/config_begin.hpp>
#include <boost/interprocess/detail/workaround.hpp>

#include <boost/interprocess/interprocess_fwd.hpp>
#include <boost/interprocess/allocators/allocation_type.hpp>
#include <boost/interprocess/detail/utilities.hpp>
#include <boost/interprocess/detail/type_traits.hpp>
#include <boost/interprocess/detail/iterators.hpp>
#include <boost/interprocess/detail/math_functions.hpp>
#include <boost/interprocess/detail/utilities.hpp>
#include <boost/assert.hpp>
#include <boost/static_assert.hpp>

//!\file
//!Implements common operations for memory algorithms.

namespace boost {
namespace interprocess {
namespace detail {

template<class VoidPointer>
struct multi_allocation_next
{
   typedef typename detail::
      pointer_to_other<VoidPointer, multi_allocation_next>::type
         multi_allocation_next_ptr;

   multi_allocation_next(multi_allocation_next_ptr n)
      :  next_(n)
   {}
   multi_allocation_next_ptr next_;
};

//!This iterator is returned by "allocate_many" functions so that
//!the user can access the multiple buffers allocated in a single call
template<class VoidPointer>
class basic_multiallocation_iterator
   :  public std::iterator<std::input_iterator_tag, char>
{
   void unspecified_bool_type_func() const {}
   typedef void (basic_multiallocation_iterator::*unspecified_bool_type)() const;
   typedef typename detail::
      pointer_to_other
         <VoidPointer, multi_allocation_next<VoidPointer> >::type
            multi_allocation_next_ptr;

   public:
   typedef char         value_type;
   typedef value_type & reference;
   typedef value_type * pointer;

   basic_multiallocation_iterator()
      : next_alloc_(0)
   {}

   basic_multiallocation_iterator(multi_allocation_next_ptr next)
      : next_alloc_(next)
   {}

   basic_multiallocation_iterator &operator=(const basic_multiallocation_iterator &other)
   {  next_alloc_ = other.next_alloc_;  return *this;  }

   public:
   basic_multiallocation_iterator& operator++() 
   {  next_alloc_.next_ = detail::get_pointer(next_alloc_.next_->next_); return *this;  }
   
   basic_multiallocation_iterator operator++(int)
   {
      basic_multiallocation_iterator result(next_alloc_.next_);
      ++*this;
      return result;
   }

   bool operator== (const basic_multiallocation_iterator& other) const
   { return next_alloc_.next_ == other.next_alloc_.next_; }

   bool operator!= (const basic_multiallocation_iterator& other) const
   { return !operator== (other); }

   reference operator*() const
   {  return *reinterpret_cast<char*>(detail::get_pointer(next_alloc_.next_)); }

   operator unspecified_bool_type() const  
   {  return next_alloc_.next_? &basic_multiallocation_iterator::unspecified_bool_type_func : 0;   }

   pointer operator->() const
   { return &(*(*this)); }

   static basic_multiallocation_iterator create_simple_range(void *mem)
   {
      basic_multiallocation_iterator it;
      typedef multi_allocation_next<VoidPointer> next_impl_t;
      next_impl_t * tmp_mem = static_cast<next_impl_t*>(mem);
      it = basic_multiallocation_iterator<VoidPointer>(tmp_mem);
      tmp_mem->next_ = 0;
      return it;
   }

   multi_allocation_next<VoidPointer> &get_multi_allocation_next()
   {  return *next_alloc_.next_;  }

   private:
   multi_allocation_next<VoidPointer> next_alloc_;
};

template<class VoidPointer>
class basic_multiallocation_chain
{
   private:
   basic_multiallocation_iterator<VoidPointer> it_;
   VoidPointer last_mem_;
   std::size_t num_mem_;

   basic_multiallocation_chain(const basic_multiallocation_chain &);
   basic_multiallocation_chain &operator=(const basic_multiallocation_chain &);

   public:
   typedef basic_multiallocation_iterator<VoidPointer> multiallocation_iterator;

   basic_multiallocation_chain()
      :  it_(0), last_mem_(0), num_mem_(0)
   {}

   void reset()
   {
      this->it_ = multiallocation_iterator();
      this->last_mem_ = 0;
      this->num_mem_ = 0;
   }

   void push_back(void *mem)
   {
      typedef multi_allocation_next<VoidPointer> next_impl_t;
      next_impl_t * tmp_mem = static_cast<next_impl_t*>(mem);
      
      if(!this->last_mem_){
         this->it_ = basic_multiallocation_iterator<VoidPointer>(tmp_mem);
      }
      else{
         static_cast<next_impl_t*>(detail::get_pointer(this->last_mem_))->next_ = tmp_mem;
      }
      tmp_mem->next_ = 0;
      this->last_mem_ = tmp_mem;
      ++num_mem_;
   }

   void push_front(void *mem)
   {
      typedef multi_allocation_next<VoidPointer> next_impl_t;
      next_impl_t * tmp_mem   = static_cast<next_impl_t*>(mem);      
      ++num_mem_;

      if(!this->last_mem_){
         this->it_ = basic_multiallocation_iterator<VoidPointer>(tmp_mem);
         tmp_mem->next_ = 0;
         this->last_mem_ = tmp_mem;
      }
      else{
         next_impl_t * old_first = &this->it_.get_multi_allocation_next();
         tmp_mem->next_          = old_first;
         this->it_ = basic_multiallocation_iterator<VoidPointer>(tmp_mem);
      }
   }

   void swap(basic_multiallocation_chain &other_chain)
   {
      std::swap(this->it_, other_chain.it_);
      std::swap(this->last_mem_, other_chain.last_mem_);
      std::swap(this->num_mem_, other_chain.num_mem_);
   }

   void splice_back(basic_multiallocation_chain &other_chain)
   {
      typedef multi_allocation_next<VoidPointer> next_impl_t;
      multiallocation_iterator end_it;
      multiallocation_iterator other_it = other_chain.get_it();
      multiallocation_iterator this_it  = this->get_it();
      if(end_it == other_it){
         return;
      }
      else if(end_it == this_it){
         this->swap(other_chain);
      }
      else{
         static_cast<next_impl_t*>(detail::get_pointer(this->last_mem_))->next_
            = &other_chain.it_.get_multi_allocation_next();
         this->last_mem_ = other_chain.last_mem_;
         this->num_mem_ += other_chain.num_mem_;
      }
   }

   void *pop_front()
   {
      multiallocation_iterator itend;
      if(this->it_ == itend){
         this->last_mem_= 0;
         this->num_mem_ = 0;
         return 0;
      }
      else{
         void *addr = &*it_;
         ++it_;
         --num_mem_;
         if(!num_mem_){
            this->last_mem_ = 0;
            this->it_ = multiallocation_iterator();
         }
         return addr;
      }
   }

   bool empty() const
   {  return !num_mem_; }

   multiallocation_iterator get_it() const
   {  return it_;  }

   std::size_t size() const
   {  return num_mem_;  }
};


//!This class implements several allocation functions shared by different algorithms
//!(aligned allocation, multiple allocation...).
template<class MemoryAlgorithm>
class memory_algorithm_common
{
   public:
   typedef typename MemoryAlgorithm::void_pointer              void_pointer;
   typedef typename MemoryAlgorithm::block_ctrl                block_ctrl;
   typedef typename MemoryAlgorithm::multiallocation_iterator  multiallocation_iterator;
   typedef multi_allocation_next<void_pointer>                 multi_allocation_next_t;
   typedef typename multi_allocation_next_t::
      multi_allocation_next_ptr                                multi_allocation_next_ptr;
   typedef memory_algorithm_common<MemoryAlgorithm>            this_type;

   static const std::size_t Alignment           = MemoryAlgorithm::Alignment;
   static const std::size_t MinBlockUnits       = MemoryAlgorithm::MinBlockUnits;
   static const std::size_t AllocatedCtrlBytes  = MemoryAlgorithm::AllocatedCtrlBytes;
   static const std::size_t AllocatedCtrlUnits  = MemoryAlgorithm::AllocatedCtrlUnits;
   static const std::size_t BlockCtrlBytes      = MemoryAlgorithm::BlockCtrlBytes;
   static const std::size_t BlockCtrlUnits      = MemoryAlgorithm::BlockCtrlUnits;
   static const std::size_t UsableByPreviousChunk   = MemoryAlgorithm::UsableByPreviousChunk;

   static void assert_alignment(const void *ptr)
   {  assert_alignment((std::size_t)ptr); }

   static void assert_alignment(std::size_t uint_ptr)
   {
      (void)uint_ptr;
      BOOST_ASSERT(uint_ptr % Alignment == 0);
   }

   static bool check_alignment(const void *ptr)
   {  return (((std::size_t)ptr) % Alignment == 0);   }

   static std::size_t ceil_units(std::size_t size)
   {  return detail::get_rounded_size(size, Alignment)/Alignment; }

   static std::size_t floor_units(std::size_t size)
   {  return size/Alignment;  }

   static std::size_t multiple_of_units(std::size_t size)
   {  return detail::get_rounded_size(size, Alignment);  }

   static multiallocation_iterator allocate_many
      (MemoryAlgorithm *memory_algo, std::size_t elem_bytes, std::size_t n_elements)
   {
      return this_type::priv_allocate_many(memory_algo, &elem_bytes, n_elements, 0);
   }

   static bool calculate_lcm_and_needs_backwards_lcmed
      (std::size_t backwards_multiple, std::size_t received_size, std::size_t size_to_achieve,
      std::size_t &lcm_out, std::size_t &needs_backwards_lcmed_out)
   {
      // Now calculate lcm
      std::size_t max = backwards_multiple;
      std::size_t min = Alignment;
      std::size_t needs_backwards;
      std::size_t needs_backwards_lcmed;
      std::size_t lcm;
      std::size_t current_forward;
      //Swap if necessary
      if(max < min){
         std::size_t tmp = min;
         min = max;
         max = tmp;
      }
      //Check if it's power of two
      if((backwards_multiple & (backwards_multiple-1)) == 0){
         if(0 != (size_to_achieve & ((backwards_multiple-1)))){
            return false;
         }

         lcm = max;
         //If we want to use minbytes data to get a buffer between maxbytes
         //and minbytes if maxbytes can't be achieved, calculate the 
         //biggest of all possibilities
         current_forward = detail::get_truncated_size_po2(received_size, backwards_multiple);
         needs_backwards = size_to_achieve - current_forward;
         assert((needs_backwards % backwards_multiple) == 0);
         needs_backwards_lcmed = detail::get_rounded_size_po2(needs_backwards, lcm);
         lcm_out = lcm;
         needs_backwards_lcmed_out = needs_backwards_lcmed;
         return true;
      }
      //Check if it's multiple of alignment
      else if((backwards_multiple & (Alignment - 1u)) == 0){
         lcm = backwards_multiple;
         current_forward = detail::get_truncated_size(received_size, backwards_multiple);
         //No need to round needs_backwards because backwards_multiple == lcm
         needs_backwards_lcmed = needs_backwards = size_to_achieve - current_forward;
         assert((needs_backwards_lcmed & (Alignment - 1u)) == 0);
         lcm_out = lcm;
         needs_backwards_lcmed_out = needs_backwards_lcmed;
         return true;
      }
      //Check if it's multiple of the half of the alignmment
      else if((backwards_multiple & ((Alignment/2u) - 1u)) == 0){
         lcm = backwards_multiple*2u;
         current_forward = detail::get_truncated_size(received_size, backwards_multiple);
         needs_backwards_lcmed = needs_backwards = size_to_achieve - current_forward;
         if(0 != (needs_backwards_lcmed & (Alignment-1)))
         //while(0 != (needs_backwards_lcmed & (Alignment-1)))
            needs_backwards_lcmed += backwards_multiple;
         assert((needs_backwards_lcmed % lcm) == 0);
         lcm_out = lcm;
         needs_backwards_lcmed_out = needs_backwards_lcmed;
         return true;
      }
      //Check if it's multiple of the half of the alignmment
      else if((backwards_multiple & ((Alignment/4u) - 1u)) == 0){
         std::size_t remainder;
         lcm = backwards_multiple*4u;
         current_forward = detail::get_truncated_size(received_size, backwards_multiple);
         needs_backwards_lcmed = needs_backwards = size_to_achieve - current_forward;
         //while(0 != (needs_backwards_lcmed & (Alignment-1)))
            //needs_backwards_lcmed += backwards_multiple;
         if(0 != (remainder = ((needs_backwards_lcmed & (Alignment-1))>>(Alignment/8u)))){
            if(backwards_multiple & Alignment/2u){
               needs_backwards_lcmed += (remainder)*backwards_multiple;
            }
            else{
               needs_backwards_lcmed += (4-remainder)*backwards_multiple;
            }
         }
         assert((needs_backwards_lcmed % lcm) == 0);
         lcm_out = lcm;
         needs_backwards_lcmed_out = needs_backwards_lcmed;
         return true;
      }
      else{
         lcm = detail::lcm(max, min);
      }
      //If we want to use minbytes data to get a buffer between maxbytes
      //and minbytes if maxbytes can't be achieved, calculate the 
      //biggest of all possibilities
      current_forward = detail::get_truncated_size(received_size, backwards_multiple);
      needs_backwards = size_to_achieve - current_forward;
      assert((needs_backwards % backwards_multiple) == 0);
      needs_backwards_lcmed = detail::get_rounded_size(needs_backwards, lcm);
      lcm_out = lcm;
      needs_backwards_lcmed_out = needs_backwards_lcmed;
      return true;
   }

   static multiallocation_iterator allocate_many
      ( MemoryAlgorithm *memory_algo
      , const std::size_t *elem_sizes
      , std::size_t n_elements
      , std::size_t sizeof_element)
   {
      return this_type::priv_allocate_many(memory_algo, elem_sizes, n_elements, sizeof_element);
   }

   static void* allocate_aligned
      (MemoryAlgorithm *memory_algo, std::size_t nbytes, std::size_t alignment)
   {
      
      //Ensure power of 2
      if ((alignment & (alignment - std::size_t(1u))) != 0){
         //Alignment is not power of two
         BOOST_ASSERT((alignment & (alignment - std::size_t(1u))) == 0);
         return 0;
      }

      std::size_t real_size;
      if(alignment <= Alignment){
         return memory_algo->priv_allocate(allocate_new, nbytes, nbytes, real_size).first;
      }

      if(nbytes > UsableByPreviousChunk)
         nbytes -= UsableByPreviousChunk;
      
      //We can find a aligned portion if we allocate a block that has alignment
      //nbytes + alignment bytes or more.
      std::size_t minimum_allocation = max_value
         (nbytes + alignment, std::size_t(MinBlockUnits*Alignment));
      //Since we will split that block, we must request a bit more memory
      //if the alignment is near the beginning of the buffer, because otherwise,
      //there is no space for a new block before the alignment.
      // 
      //            ____ Aligned here
      //           |
      //  -----------------------------------------------------
      // | MBU | 
      //  -----------------------------------------------------
      std::size_t request = 
         minimum_allocation + (2*MinBlockUnits*Alignment - AllocatedCtrlBytes
         //prevsize - UsableByPreviousChunk
         );

      //Now allocate the buffer
      void *buffer = memory_algo->priv_allocate(allocate_new, request, request, real_size).first;
      if(!buffer){
         return 0;
      }
      else if ((((std::size_t)(buffer)) % alignment) == 0){
         //If we are lucky and the buffer is aligned, just split it and
         //return the high part
         block_ctrl *first  = memory_algo->priv_get_block(buffer);
         std::size_t old_size = first->m_size;
         const std::size_t first_min_units =
            max_value(ceil_units(nbytes) + AllocatedCtrlUnits, std::size_t(MinBlockUnits));
         //We can create a new block in the end of the segment
         if(old_size >= (first_min_units + MinBlockUnits)){
            block_ctrl *second =  reinterpret_cast<block_ctrl *>
               (reinterpret_cast<char*>(first) + Alignment*first_min_units);
            first->m_size  = first_min_units;
            second->m_size = old_size - first->m_size;
            BOOST_ASSERT(second->m_size >= MinBlockUnits);
            memory_algo->priv_mark_new_allocated_block(first);
            //memory_algo->priv_tail_size(first, first->m_size);
            memory_algo->priv_mark_new_allocated_block(second);
            memory_algo->priv_deallocate(memory_algo->priv_get_user_buffer(second));
         }
         return buffer;
      }

      //Buffer not aligned, find the aligned part.
      // 
      //                    ____ Aligned here
      //                   |
      //  -----------------------------------------------------
      // | MBU +more | ACB |
      //  -----------------------------------------------------
      char *pos = reinterpret_cast<char*>
         (reinterpret_cast<std::size_t>(static_cast<char*>(buffer) +
            //This is the minimum size of (2)
            (MinBlockUnits*Alignment - AllocatedCtrlBytes) +
            //This is the next MBU for the aligned memory
            AllocatedCtrlBytes +
            //This is the alignment trick
            alignment - 1) & -alignment);

      //Now obtain the address of the blocks
      block_ctrl *first  = memory_algo->priv_get_block(buffer);
      block_ctrl *second = memory_algo->priv_get_block(pos);
      assert(pos <= (reinterpret_cast<char*>(first) + first->m_size*Alignment));
      assert(first->m_size >= 2*MinBlockUnits);
      assert((pos + MinBlockUnits*Alignment - AllocatedCtrlBytes + nbytes*Alignment/Alignment) <=
             (reinterpret_cast<char*>(first) + first->m_size*Alignment));
      //Set the new size of the first block
      std::size_t old_size = first->m_size;
      first->m_size  = (reinterpret_cast<char*>(second) - reinterpret_cast<char*>(first))/Alignment;
      memory_algo->priv_mark_new_allocated_block(first);

      //Now check if we can create a new buffer in the end
      //
      //              __"second" block
      //             |      __Aligned here
      //             |     |      __"third" block
      //  -----------|-----|-----|------------------------------
      // | MBU +more | ACB | (3) | BCU |
      //  -----------------------------------------------------
      //This size will be the minimum size to be able to create a
      //new block in the end.
      const std::size_t second_min_units = max_value(std::size_t(MinBlockUnits),
                        ceil_units(nbytes) + AllocatedCtrlUnits );

      //Check if we can create a new block (of size MinBlockUnits) in the end of the segment
      if((old_size - first->m_size) >= (second_min_units + MinBlockUnits)){
         //Now obtain the address of the end block
         block_ctrl *third = new (reinterpret_cast<char*>(second) + Alignment*second_min_units)block_ctrl;
         second->m_size = second_min_units;
         third->m_size  = old_size - first->m_size - second->m_size;
         BOOST_ASSERT(third->m_size >= MinBlockUnits);
         memory_algo->priv_mark_new_allocated_block(second);
         memory_algo->priv_mark_new_allocated_block(third);
         memory_algo->priv_deallocate(memory_algo->priv_get_user_buffer(third));
      }
      else{
         second->m_size = old_size - first->m_size;
         assert(second->m_size >= MinBlockUnits);
         memory_algo->priv_mark_new_allocated_block(second);
      }

      memory_algo->priv_deallocate(memory_algo->priv_get_user_buffer(first));
      return memory_algo->priv_get_user_buffer(second);
   }

   static bool try_shrink 
      (MemoryAlgorithm *memory_algo, void *ptr
      ,const std::size_t max_size,   const std::size_t preferred_size
      ,std::size_t &received_size)
   {
      (void)memory_algo;
      //Obtain the real block
      block_ctrl *block = memory_algo->priv_get_block(ptr);
      std::size_t old_block_units = block->m_size;

      //The block must be marked as allocated
      BOOST_ASSERT(memory_algo->priv_is_allocated_block(block));

      //Check if alignment and block size are right
      assert_alignment(ptr);

      //Put this to a safe value
      received_size = (old_block_units - AllocatedCtrlUnits)*Alignment + UsableByPreviousChunk;

      //Now translate it to Alignment units
      const std::size_t max_user_units       = floor_units(max_size - UsableByPreviousChunk);
      const std::size_t preferred_user_units = ceil_units(preferred_size - UsableByPreviousChunk);

      //Check if rounded max and preferred are possible correct
      if(max_user_units < preferred_user_units)
         return false;

      //Check if the block is smaller than the requested minimum
      std::size_t old_user_units = old_block_units - AllocatedCtrlUnits;

      if(old_user_units < preferred_user_units)
         return false;

      //If the block is smaller than the requested minimum
      if(old_user_units == preferred_user_units)
         return true;

      std::size_t shrunk_user_units = 
         ((BlockCtrlUnits - AllocatedCtrlUnits) > preferred_user_units)
         ? (BlockCtrlUnits - AllocatedCtrlUnits)
         : preferred_user_units;

      //Some parameter checks
      if(max_user_units < shrunk_user_units)
         return false;

      //We must be able to create at least a new empty block
      if((old_user_units - shrunk_user_units) < BlockCtrlUnits ){
         return false;
      }

      //Update new size
      received_size = shrunk_user_units*Alignment + UsableByPreviousChunk;
      return true;
   }

   static bool shrink 
      (MemoryAlgorithm *memory_algo, void *ptr
      ,const std::size_t max_size,   const std::size_t preferred_size
      ,std::size_t &received_size)
   {
      //Obtain the real block
      block_ctrl *block = memory_algo->priv_get_block(ptr);
      std::size_t old_block_units = block->m_size;

      if(!try_shrink 
         (memory_algo, ptr, max_size, preferred_size, received_size)){
         return false;
      }

      //Check if the old size was just the shrunk size (no splitting)
      if((old_block_units - AllocatedCtrlUnits) == ceil_units(preferred_size - UsableByPreviousChunk))
         return true;

      //Now we can just rewrite the size of the old buffer
      block->m_size = (received_size-UsableByPreviousChunk)/Alignment + AllocatedCtrlUnits;
      BOOST_ASSERT(block->m_size >= BlockCtrlUnits);

      //We create the new block
      block_ctrl *new_block = reinterpret_cast<block_ctrl*>
                  (reinterpret_cast<char*>(block) + block->m_size*Alignment);
      //Write control data to simulate this new block was previously allocated
      //and deallocate it
      new_block->m_size = old_block_units - block->m_size;
      BOOST_ASSERT(new_block->m_size >= BlockCtrlUnits);
      memory_algo->priv_mark_new_allocated_block(block);
      memory_algo->priv_mark_new_allocated_block(new_block);
      memory_algo->priv_deallocate(memory_algo->priv_get_user_buffer(new_block));
      return true;
   }

   private:
   static multiallocation_iterator priv_allocate_many
      ( MemoryAlgorithm *memory_algo
      , const std::size_t *elem_sizes
      , std::size_t n_elements
      , std::size_t sizeof_element)
   {
      //Note: sizeof_element == 0 indicates that we want to
      //allocate n_elements of the same size "*elem_sizes"

      //Calculate the total size of all requests
      std::size_t total_request_units = 0;
      std::size_t elem_units = 0;
      const std::size_t ptr_size_units = memory_algo->priv_get_total_units(sizeof(multi_allocation_next_ptr));
      if(!sizeof_element){
         elem_units = memory_algo->priv_get_total_units(*elem_sizes);
         elem_units = ptr_size_units > elem_units ? ptr_size_units : elem_units;
         total_request_units = n_elements*elem_units;
      }
      else{
         for(std::size_t i = 0; i < n_elements; ++i){
            elem_units = memory_algo->priv_get_total_units(elem_sizes[i]*sizeof_element);
            elem_units = ptr_size_units > elem_units ? ptr_size_units : elem_units;
            total_request_units += elem_units;
         }
      }

      multi_allocation_next_ptr first = 0, previous = 0;
      std::size_t low_idx = 0;
      while(low_idx < n_elements){
         std::size_t total_bytes = total_request_units*Alignment - AllocatedCtrlBytes + UsableByPreviousChunk;
         std::size_t min_allocation = (!sizeof_element)
            ?  elem_units
            :  memory_algo->priv_get_total_units(elem_sizes[low_idx]*sizeof_element);
         min_allocation = min_allocation*Alignment - AllocatedCtrlBytes + UsableByPreviousChunk;

         std::size_t received_size;
         std::pair<void *, bool> ret = memory_algo->priv_allocate
            (allocate_new, min_allocation, total_bytes, received_size, 0);
         if(!ret.first){
            break;
         }

         block_ctrl *block = memory_algo->priv_get_block(ret.first);
         std::size_t received_units = block->m_size;
         char *block_address = reinterpret_cast<char*>(block);

         std::size_t total_used_units = 0;
//         block_ctrl *prev_block = 0;
         while(total_used_units < received_units){
            if(sizeof_element){
               elem_units = memory_algo->priv_get_total_units(elem_sizes[low_idx]*sizeof_element);
               elem_units = ptr_size_units > elem_units ? ptr_size_units : elem_units;
            }
            if(total_used_units + elem_units > received_units)
               break;
            total_request_units -= elem_units;
            //This is the position where the new block must be created
            block_ctrl *new_block = reinterpret_cast<block_ctrl *>(block_address);
            assert_alignment(new_block);

            //The last block should take all the remaining space
            if((low_idx + 1) == n_elements ||
               (total_used_units + elem_units + 
               ((!sizeof_element)
                  ? elem_units
                  : memory_algo->priv_get_total_units(elem_sizes[low_idx+1]*sizeof_element))
               ) > received_units){
               //By default, the new block will use the rest of the buffer
               new_block->m_size = received_units - total_used_units;
               memory_algo->priv_mark_new_allocated_block(new_block);

               //If the remaining units are bigger than needed and we can
               //split it obtaining a new free memory block do it.
               if((received_units - total_used_units) >= (elem_units + MemoryAlgorithm::BlockCtrlUnits)){
                  std::size_t shrunk_received;
                  std::size_t shrunk_request = elem_units*Alignment - AllocatedCtrlBytes + UsableByPreviousChunk;
                  bool shrink_ok = shrink
                        (memory_algo
                        ,memory_algo->priv_get_user_buffer(new_block)
                        ,shrunk_request
                        ,shrunk_request
                        ,shrunk_received);
                  (void)shrink_ok;
                  //Shrink must always succeed with passed parameters
                  BOOST_ASSERT(shrink_ok);
                  //Some sanity checks
                  BOOST_ASSERT(shrunk_request == shrunk_received);
                  BOOST_ASSERT(elem_units == ((shrunk_request-UsableByPreviousChunk)/Alignment + AllocatedCtrlUnits));
                  //"new_block->m_size" must have been reduced to elem_units by "shrink"
                  BOOST_ASSERT(new_block->m_size == elem_units);
                  //Now update the total received units with the reduction
                  received_units = elem_units + total_used_units;
               }
            }
            else{
               new_block->m_size = elem_units;
               memory_algo->priv_mark_new_allocated_block(new_block);
            }

            block_address += new_block->m_size*Alignment;
            total_used_units += new_block->m_size;
            //Check we have enough room to overwrite the intrusive pointer
            assert((new_block->m_size*Alignment - AllocatedCtrlUnits) >= sizeof(multi_allocation_next_t));
            multi_allocation_next_ptr p = new(memory_algo->priv_get_user_buffer(new_block))multi_allocation_next_t(0);
      
            if(!first){
               first = p;
            }
            else{
               previous->next_ = p;
            }
            previous = p;
            ++low_idx;
            //prev_block = new_block;
         }
         //Sanity check
         BOOST_ASSERT(total_used_units == received_units);
      }

      if(low_idx != n_elements){
         while(first){
            multi_allocation_next_ptr prev = first;
            first = first->next_;
            memory_algo->priv_deallocate(detail::get_pointer(prev));
         }
         return multiallocation_iterator();
      }
      else{
         return multiallocation_iterator(first);
      }
   }
};

}  //namespace detail {
}  //namespace interprocess {
}  //namespace boost {

#include <boost/interprocess/detail/config_end.hpp>

#endif   //#ifndef BOOST_INTERPROCESS_DETAIL_MEM_ALGO_COMMON_HPP
