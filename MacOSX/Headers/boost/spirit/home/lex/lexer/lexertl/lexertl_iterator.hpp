//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_LEX_LEXER_ITERATOR_MAR_16_2007_0353PM)
#define BOOST_SPIRIT_LEX_LEXER_ITERATOR_MAR_16_2007_0353PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#if defined(BOOST_SPIRIT_LEXERTL_DEBUG)
#include <boost/spirit/home/support/iterators/detail/buf_id_check_policy.hpp>
#else
#include <boost/spirit/home/support/iterators/detail/no_check_policy.hpp>
#endif
#include <boost/spirit/home/support/iterators/detail/split_functor_input_policy.hpp>
#include <boost/spirit/home/support/iterators/detail/ref_counted_policy.hpp>
#include <boost/spirit/home/support/iterators/detail/split_std_deque_policy.hpp>
#include <boost/spirit/home/support/iterators/multi_pass.hpp>

namespace boost { namespace spirit { namespace lex 
{ 
    ///////////////////////////////////////////////////////////////////////////
    //  Divide the given functor type into its components (unique and shared) 
    //  and build a std::pair from these parts
    template <typename Functor>
    struct make_functor
    {
        typedef 
            std::pair<typename Functor::unique, typename Functor::shared> 
        type;
    };
    
    ///////////////////////////////////////////////////////////////////////////////
    //  Divide the given functor type into its components (unique and shared) 
    //  and build a std::pair from these parts
    template <typename FunctorData>
    struct make_multi_pass
    {
        typedef  
            std::pair<typename FunctorData::unique, typename FunctorData::shared> 
        functor_data_type;
        typedef typename FunctorData::result_type result_type;

        typedef multi_pass_policies::split_functor_input input_policy;
        typedef multi_pass_policies::ref_counted ownership_policy;
#if defined(BOOST_SPIRIT_LEXERTL_DEBUG)
        typedef multi_pass_policies::buf_id_check check_policy;
#else
        typedef multi_pass_policies::no_check check_policy;
#endif
        typedef multi_pass_policies::split_std_deque storage_policy;
        
        typedef multi_pass_policies::default_policy<
                ownership_policy, check_policy, input_policy, storage_policy>
            policy_type;
        typedef spirit::multi_pass<functor_data_type, policy_type> type;
    };

    ///////////////////////////////////////////////////////////////////////////
    //  lexer_iterator exposes an iterator for a lexertl based dfa (lexer) 
    //  The template parameters have the same semantics as described for the
    //  lexertl_functor above.
    ///////////////////////////////////////////////////////////////////////////
    template <typename Functor>
    class lexertl_iterator
      : public make_multi_pass<Functor>::type
    {
    public:
        typedef typename Functor::unique unique_functor_type;
        typedef typename Functor::shared shared_functor_type;
        
        typedef typename Functor::iterator_type base_iterator_type;
        typedef typename Functor::result_type token_type;
        
    private:
        typedef 
            typename make_multi_pass<Functor>::functor_data_type 
        functor_type;
        typedef typename make_multi_pass<Functor>::type base_type;
        typedef typename Functor::char_type char_type;
        
    public:
        // create a new iterator encapsulating the lexer object to be used
        // for tokenization
        template <typename IteratorData>
        lexertl_iterator(IteratorData const& iterdata_, 
                base_iterator_type& first, base_iterator_type const& last)
          : base_type(functor_type(unique_functor_type(), 
                shared_functor_type(iterdata_, first, last))
            )
        {
        }
        
        // create an end iterator usable for end of range checking
        lexertl_iterator()
        {}
        
        // set the new required state for the underlying lexer object
        std::size_t set_state(std::size_t state)
        {
            return unique_functor_type::set_state(*this, state);
        }
        
        // map the given state name to a corresponding state id as understood
        // by the underlying lexer object
        std::size_t map_state(char_type const* statename)
        {
            return unique_functor_type::map_state(*this, statename);
        }
    };
    
}}}

#endif
