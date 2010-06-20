//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_KARMA_DETAIL_EXTRACT_FROM_FEB_20_2007_0417PM)
#define BOOST_SPIRIT_KARMA_DETAIL_EXTRACT_FROM_FEB_20_2007_0417PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/support/unused.hpp>
#include <boost/spirit/home/support/char_class.hpp>

namespace boost { namespace spirit { namespace karma { namespace detail 
{
    ///////////////////////////////////////////////////////////////////////////
    //  These utility functions insert the given parameter into the supplied 
    //  output iterator.
    //  If the parameter is spirit's unused_type, this is a no_op.
    ///////////////////////////////////////////////////////////////////////////
    template <typename OutputIterator, typename Parameter, typename Tag>
    inline bool 
    generate_to(OutputIterator& sink, Parameter const& p, Tag)
    {
        typedef typename Tag::char_set char_set;
        typedef typename Tag::char_class char_class;
        
        *sink = spirit::char_class::convert<char_set>::to(char_class(), p);
        ++sink;
        return true;
    }

    template <typename OutputIterator, typename Parameter>
    inline bool 
    generate_to(OutputIterator& sink, Parameter const& p, unused_type = unused)
    {
        *sink = p;
        ++sink;
        return true;
    }

    template <typename OutputIterator, typename Tag>
    inline bool generate_to(OutputIterator& sink, unused_type, Tag)
    {
        return true;
    }
    
    template <typename OutputIterator>
    inline bool generate_to(OutputIterator& sink, unused_type)
    {
        return true;
    }
    
}}}}   // namespace boost::spirit::karma::detail

#endif  // KARMA_CORE_DETAIL_INSERT_TO_HPP
