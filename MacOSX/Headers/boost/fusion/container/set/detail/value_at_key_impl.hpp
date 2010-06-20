/*=============================================================================
    Copyright (c) 2001-2006 Joel de Guzman
    Copyright (c) 2006 Dan Marsden

    Distributed under the Boost Software License, Version 1.0. (See accompanying 
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(FUSION_VALUE_AT_KEY_IMPL_09162005_1123)
#define FUSION_VALUE_AT_KEY_IMPL_09162005_1123

#include <boost/mpl/at.hpp>

namespace boost { namespace fusion
{
    struct set_tag;

    namespace extension
    {
        template <typename Tag>
        struct value_at_key_impl;

        template <>
        struct value_at_key_impl<set_tag>
        {
            template <typename Sequence, typename Key>
            struct apply 
            {
                typedef typename Sequence::
                    template meta_at_impl<Key>::type type;
            };
        };
    }
}}

#endif
