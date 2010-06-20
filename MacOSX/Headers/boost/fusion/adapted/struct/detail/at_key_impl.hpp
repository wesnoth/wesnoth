/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman
    Copyright (c) 2005-2007 Dan Marsden

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_FUSION_AT_KEY_IMPL_20070508_2248)
#define BOOST_FUSION_AT_KEY_IMPL_20070508_2248

#include <boost/fusion/support/detail/access.hpp>

namespace boost { namespace fusion
{
    struct struct_tag;

    namespace extension
    {
        template<typename T>
        struct at_key_impl;

        template <typename Struct, typename Key>
        struct struct_assoc_member;

        template <>
        struct at_key_impl<struct_tag>
        {
            template <typename Sequence, typename Key>
            struct apply
            {
                typedef typename
                extension::struct_assoc_member<Sequence, Key>
                element;

                typedef typename
                    mpl::eval_if<
                        is_const<Sequence>
                      , detail::cref_result<element>
                      , detail::ref_result<element>
                    >::type
                type;

                static type
                call(Sequence& seq)
                {
                    return extension::
                        struct_assoc_member<Sequence, Key>::call(seq);
                }
            };
        };
    }
}}

#endif
