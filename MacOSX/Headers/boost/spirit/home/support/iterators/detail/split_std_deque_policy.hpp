//  Copyright (c) 2001, Daniel C. Nuffer
//  Copyright (c) 2001-2008, Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_ITERATOR_SPLIT_DEQUE_POLICY_APR_06_2008_0138PM)
#define BOOST_SPIRIT_ITERATOR_SPLIT_DEQUE_POLICY_APR_06_2008_0138PM

#include <boost/spirit/home/support/iterators/multi_pass_fwd.hpp>
#include <boost/spirit/home/support/iterators/detail/multi_pass.hpp>
#include <boost/assert.hpp>
#include <vector>

namespace boost { namespace spirit { namespace multi_pass_policies
{
    ///////////////////////////////////////////////////////////////////////////
    //  class split_std_deque
    //
    //  Implementation of the StoragePolicy used by multi_pass
    //  This stores all data in a std::vector (despite its name), and keeps an 
    //  offset to the current position. It stores all the data unless there is 
    //  only one iterator using the queue.
    // 
    ///////////////////////////////////////////////////////////////////////////
    struct split_std_deque
    {
        enum { threshold = 16 };
        
        ///////////////////////////////////////////////////////////////////////
        template <typename Value>
        class unique //: public detail::default_storage_policy
        {
        private:
            typedef std::vector<Value> queue_type;

        protected:
            unique()
              : queued_position(0)
            {}

            unique(unique const& x)
              : queued_position(x.queued_position)
            {}

            void swap(unique& x)
            {
                spirit::detail::swap(queued_position, x.queued_position);
            }

            // This is called when the iterator is dereferenced.  It's a 
            // template method so we can recover the type of the multi_pass 
            // iterator and call advance_input and input_is_valid.
            template <typename MultiPass>
            static typename MultiPass::reference 
            dereference(MultiPass const& mp)
            {
                queue_type& queue = mp.shared->queued_elements;
                if (0 == mp.queued_position) 
                {
                    if (queue.empty())
                    {
                        queue.push_back(Value());
                        return MultiPass::advance_input(mp, queue[mp.queued_position++]);
                    }
                    return queue[mp.queued_position++];
                }
                else if (!MultiPass::input_is_valid(mp, queue[mp.queued_position-1]))
                {
                    MultiPass::advance_input(mp, queue[mp.queued_position-1]);
                }
                return queue[mp.queued_position-1];
            }

            // This is called when the iterator is incremented. It's a template
            // method so we can recover the type of the multi_pass iterator
            // and call is_unique and advance_input.
            template <typename MultiPass>
            static void increment(MultiPass& mp)
            {
                queue_type& queue = mp.shared->queued_elements;
                typename queue_type::size_type size = queue.size();
                BOOST_ASSERT(0 != size && mp.queued_position <= size);
                if (mp.queued_position == size)
                {
                    // check if this is the only iterator
                    if (size >= threshold && MultiPass::is_unique(mp))
                    {
                        // free up the memory used by the queue. we avoid 
                        // clearing the queue on every increment, though, 
                        // because this would be too time consuming

                        // erase all but first item in queue
                        queue.erase(queue.begin()+1, queue.end());
                        mp.queued_position = 0;
                        
                        // reuse first entry in the queue and initialize 
                        // it from the input
                        MultiPass::advance_input(mp, queue[mp.queued_position++]);
                    }
                    else
                    {
                        // create a new entry in the queue and initialize 
                        // it from the input
                        queue.push_back(Value());
                        MultiPass::advance_input(mp, queue[mp.queued_position++]);
                    }
                }
                else
                {
                    ++mp.queued_position;
                }
            }

            // called to forcibly clear the queue
            template <typename MultiPass>
            static void clear_queue(MultiPass& mp)
            {
                mp.shared->queued_elements.clear();
                mp.queued_position = 0;
            }

            // called to determine whether the iterator is an eof iterator
            template <typename MultiPass>
            static bool is_eof(MultiPass const& mp)
            {
                queue_type& queue = mp.shared->queued_elements;
                return 0 != mp.queued_position && 
                    mp.queued_position == queue.size() && 
                    MultiPass::input_at_eof(mp, queue[mp.queued_position-1]);
            }

            // called by operator==
            template <typename MultiPass>
            static bool equal_to(MultiPass const& mp, MultiPass const& x) 
            {
                return mp.queued_position == x.queued_position;
            }

            // called by operator<
            template <typename MultiPass>
            static bool less_than(MultiPass const& mp, MultiPass const& x)
            {
                return mp.queued_position < x.queued_position;
            }
            
            template <typename MultiPass>
            static void destroy(MultiPass&) 
            {}

        protected:
            mutable typename queue_type::size_type queued_position;
        }; 

        ///////////////////////////////////////////////////////////////////////
        template <typename Value>
        struct shared
        {
            shared() { queued_elements.reserve(threshold); }
            
            typedef std::vector<Value> queue_type;
            queue_type queued_elements;
        }; 

    }; // split_std_deque

}}}

#endif

