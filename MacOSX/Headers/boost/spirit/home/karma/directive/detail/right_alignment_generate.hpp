//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_KARMA_RIGHT_ALIGNMENT_GENERATE_FEB_27_2007_1216PM)
#define BOOST_SPIRIT_KARMA_RIGHT_ALIGNMENT_GENERATE_FEB_27_2007_1216PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/support/component.hpp>
#include <boost/spirit/home/support/unused.hpp>
#include <boost/mpl/assert.hpp>

namespace boost { namespace spirit { namespace karma { namespace detail 
{
    ///////////////////////////////////////////////////////////////////////////
    //  The right_align_generate template function is used for all the 
    //  different flavors of the right_align[] directive. 
    ///////////////////////////////////////////////////////////////////////////
    template <typename OutputIterator, typename Context, typename Delimiter, 
        typename Parameter, typename Embedded, typename Padding>
    inline static bool 
    right_align_generate(OutputIterator& sink, Context& ctx, 
        Delimiter const& d, Parameter const& param, Embedded const& e, 
        unsigned int const width, Padding const& p) 
    {
        // make sure all generator parameters are valid
        BOOST_MPL_ASSERT_MSG(
            (spirit::traits::is_component<karma::domain, Embedded>::value), 
            embedded_is_not_convertible_to_a_generator, (Context, Embedded));

        BOOST_MPL_ASSERT_MSG(
            (spirit::traits::is_component<karma::domain, Padding>::value), 
            padding_is_not_convertible_to_a_generator, (Context, Padding));
            
        typedef 
            typename result_of::as_component<karma::domain, Embedded>::type 
        embedded;
        typedef 
            typename result_of::as_component<karma::domain, Padding>::type 
        padding;
        
        // wrap the given output iterator to allow left padding
        detail::enable_buffering<OutputIterator> buffering(sink, width);
    
        // first generate the embedded output 
        embedded ec = spirit::as_component(karma::domain(), e);
        typedef typename embedded::director director;
        bool r = director::generate(ec, sink, ctx, d, param);
        
        buffering.disable();    // do not perform buffering any more
        
        // generate the left padding
        detail::enable_counting<OutputIterator> counting(sink, sink.buffer_size());

        padding pc = spirit::as_component(karma::domain(), p);
        while(r && sink.count() < width) {
            typedef typename padding::director padding_director;
            r = padding_director::generate(pc, sink, ctx, unused, unused);
        }
        
        // copy the embedded output to the target output iterator
        if (r) 
            sink.buffer_copy();
        return r;
    }

}}}}   // namespace boost::spirit::karma::detail

#endif


