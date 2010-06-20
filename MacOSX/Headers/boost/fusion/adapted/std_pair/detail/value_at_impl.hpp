/*=============================================================================
    Copyright (c) 2001-2006 Joel de Guzman
    Copyright (c) 2005-2006 Dan Marsden

    Distributed under the Boost Software License, Version 1.0. (See accompanying 
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_FUSION_VALUE_AT_IMPL_24122005_1917)
#define BOOST_FUSION_VALUE_AT_IMPL_24122005_1917

#include <boost/mpl/if.hpp>
#include <boost/static_assert.hpp>

namespace boost { namespace fusion {

    struct std_pair_tag;

    namespace extension
    {
        template<typename T>
        struct value_at_impl;

        template <>
        struct value_at_impl<std_pair_tag>
        {
            template <typename Sequence, typename N>
            struct apply 
            {
                static int const n_value = N::value;
                BOOST_STATIC_ASSERT((n_value >= 0 && n_value < 2));
                typedef typename
                    mpl::if_c<
                        (n_value == 0)
                      , typename Sequence::first_type
                      , typename Sequence::second_type
                    >::type
                type;
            };
        };
    }
}}

#endif
