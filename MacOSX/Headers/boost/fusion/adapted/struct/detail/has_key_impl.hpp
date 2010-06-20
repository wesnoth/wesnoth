/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman
    Copyright (c) 2005-2007 Dan Marsden

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_FUSION_HAS_KEY_IMPL_20070508_2231)
#define BOOST_FUSION_HAS_KEY_IMPL_20070508_2231

#include <boost/type_traits/is_same.hpp>
#include <boost/mpl/not.hpp>

namespace boost { namespace fusion {

    struct struct_tag;

    namespace extension
    {
        struct no_such_member;

        template<typename T>
        struct has_key_impl;

        template<typename Struct, typename Key>
        struct struct_assoc_member;

        template<>
        struct has_key_impl<struct_tag>
        {
            template<typename Sequence, typename Key>
            struct apply
                : mpl::not_<is_same<no_such_member, typename struct_assoc_member<Sequence, Key>::type> >
            {
            };
        };
    }
}}

#endif
