/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_SPIRIT_RANGE_MAY_16_2006_0720_PM)
#define BOOST_SPIRIT_RANGE_MAY_16_2006_0720_PM

namespace boost { namespace spirit { namespace qi { namespace detail
{
    ///////////////////////////////////////////////////////////////////////////
    //  A closed range (first, last)
    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    struct range
    {
        typedef T value_type;

        range()
            : first(), last()
        {
        }

        range(T first, T last)
            : first(first), last(last)
        {
        }

        T first;
        T last;
    };
}}}}

#endif
