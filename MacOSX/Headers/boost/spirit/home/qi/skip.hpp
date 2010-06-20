/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying 
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_SPIRIT_SKIP_APR_16_2006_0625PM)
#define BOOST_SPIRIT_SKIP_APR_16_2006_0625PM

#include <boost/spirit/home/qi/meta_grammar.hpp>
#include <boost/spirit/home/support/unused.hpp>

namespace boost { namespace spirit { namespace qi
{ 
    ///////////////////////////////////////////////////////////////////////////
    // Move the /first/ iterator to the first non-matching position
    // given a skip-parser. The function is a no-op if unused_type is 
    // passed as the skip-parser.
    ///////////////////////////////////////////////////////////////////////////
    template <typename Iterator, typename T>
    inline void skip(Iterator& first, Iterator const& last, T const& skipper)
    {
        while (first != last && 
               T::director::parse(skipper, first, last, unused, unused, unused))
            ;
    }

    template <typename Iterator>
    inline void skip(Iterator&, Iterator const&, unused_type)
    {
    }
}}}

#endif
