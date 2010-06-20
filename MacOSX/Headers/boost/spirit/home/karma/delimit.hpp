//  Copyright (c) 2001-2008 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_KARMA_DELIMIT_FEB_20_2007_1208PM)
#define BOOST_SPIRIT_KARMA_DELIMIT_FEB_20_2007_1208PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/support/unused.hpp>

namespace boost { namespace spirit { namespace karma
{
    ///////////////////////////////////////////////////////////////////////////
    //  Do delimiting. This is equivalent to p << d. The function is a
    //  no-op if spirit::unused is passed as the delimiter-generator.
    ///////////////////////////////////////////////////////////////////////////
    template <typename OutputIterator, typename Delimiter>
    inline void delimit(OutputIterator& sink, Delimiter const& d)
    {
        Delimiter::director::generate(d, sink, unused, unused, unused);
    }

    template <typename OutputIterator>
    inline void delimit(OutputIterator&, unused_type)
    {
    }

}}}

#endif

