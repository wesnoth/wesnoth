/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman
    Copyright (c) 2005-2007 Dan Marsden

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_FUSION_VALUE_AT_KEY_IMPL_20070508_2300)
#define BOOST_FUSION_VALUE_AT_KEY_IMPL_20070508_2300

#include <boost/mpl/if.hpp>

namespace boost { namespace fusion
{
    struct struct_tag;

    namespace extension
    {
        template<typename T>
        struct value_at_key_impl;

        template <typename Struct, typename Key>
        struct struct_assoc_member;

        template <>
        struct value_at_key_impl<struct_tag>
        {
            template <typename Sequence, typename Key>
            struct apply
            {
                typedef typename
                    extension::struct_assoc_member<Sequence, Key>::type
                type;
            };
        };
    }
}}

#endif
