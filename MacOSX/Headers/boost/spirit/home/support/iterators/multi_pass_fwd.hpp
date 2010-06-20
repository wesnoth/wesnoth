/*=============================================================================
    Copyright (c) 2007 Tobias Schwinger
    http://spirit.sourceforge.net/

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(BOOST_SPIRIT_ITERATOR_MULTI_PASS_FWD_APR_18_2008_1102AM)
#define BOOST_SPIRIT_ITERATOR_MULTI_PASS_FWD_APR_18_2008_1102AM

#include <cstddef>

namespace boost { namespace spirit {

    namespace multi_pass_policies
    {
        // input policies
        struct input_iterator;
        struct lex_input;
        struct functor_input;
        struct split_functor_input;
        
        // ownership policies
        struct ref_counted;
        struct first_owner;
        
        // checking policies
        class illegal_backtracking;
        struct buf_id_check;
        struct no_check;
        
        // storage policies
        struct std_deque;
        template<std::size_t N> struct fixed_size_queue;
    }

    template <typename T, typename Policies>
    class multi_pass;

    template <typename T, typename Policies>
    void swap(multi_pass<T, Policies> &x, multi_pass<T, Policies> &y);

    namespace detail 
    {
        template <typename T> void swap(T& t1, T& t2);
    }
    
}} // namespace boost::spirit

#endif

