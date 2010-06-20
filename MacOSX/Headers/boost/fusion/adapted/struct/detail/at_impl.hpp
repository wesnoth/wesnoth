/*=============================================================================
    Copyright (c) 2001-2006 Joel de Guzman
    Copyright (c) 2005-2006 Dan Marsden

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_FUSION_AT_IMPL_24122005_1807)
#define BOOST_FUSION_AT_IMPL_24122005_1807

#include <boost/fusion/support/detail/access.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/mpl/int.hpp>

namespace boost { namespace fusion
{
    struct struct_tag;

    namespace extension
    {
        template<typename T>
        struct at_impl;

        template <typename Struct, int N>
        struct struct_member;

        template <typename Struct>
        struct struct_size;

        template <>
        struct at_impl<struct_tag>
        {
            template <typename Sequence, typename N>
            struct apply
            {
                static int const n_value = N::value;
                BOOST_MPL_ASSERT_RELATION(
                    n_value, <=, extension::struct_size<Sequence>::value);

                typedef typename
                    extension::struct_member<Sequence, N::value>
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
                        struct_member<Sequence, N::value>::call(seq);
                }
            };
        };
    }
}}

#endif
