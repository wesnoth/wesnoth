/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_SPIRIT_UNUSED_APR_16_2006_0616PM)
#define BOOST_SPIRIT_UNUSED_APR_16_2006_0616PM

#include <boost/fusion/include/unused.hpp>
#include <boost/fusion/include/empty.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/mpl/not.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_same.hpp>

namespace boost { namespace spirit
{
    ///////////////////////////////////////////////////////////////////////////
    // since boost::fusion now supports exactly what we need, unused is simply
    // imported from the fusion namespace
    ///////////////////////////////////////////////////////////////////////////
    typedef boost::fusion::unused_type unused_type;
    using boost::fusion::unused;

    ///////////////////////////////////////////////////////////////////////////
    namespace traits
    {
        // We use this test to detect if the argument is not a unused_type
        template <typename T>
        struct is_not_unused
          : mpl::not_<is_same<T, unused_type> >
        {};

        // Return unused_type if Target is same as Actual, else
        // return Attribute (Attribute defaults to Actual).
        template <typename Target, typename Actual, typename Attribute = Actual>
        struct unused_if_same
          : mpl::if_<is_same<Target, Actual>, unused_type, Attribute>
        {};

        // Return unused_type if Sequence is empty, else return Attribute.
        //  (Attribute defaults to Sequence).
        template <typename Sequence, typename Attribute = Sequence>
        struct unused_if_empty
          : mpl::if_<fusion::result_of::empty<Sequence>, unused_type, Attribute>
        {};
    }

}}

#endif
