/*=============================================================================
    Copyright (c) 2001-2006 Joel de Guzman
    Copyright (c) 2005-2006 Dan Marsden

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_FUSION_BEGIN_IMPL_24122005_1752)
#define BOOST_FUSION_BEGIN_IMPL_24122005_1752

#include <boost/fusion/adapted/struct/struct_iterator.hpp>

namespace boost { namespace fusion
{
    struct struct_tag;

    namespace extension
    {
        template<typename T>
        struct begin_impl;

        template <>
        struct begin_impl<struct_tag>
        {
            template <typename Sequence>
            struct apply
            {
                typedef struct_iterator<Sequence, 0> type;

                static type
                call(Sequence& v)
                {
                    return type(v);
                }
            };
        };
    }
}}

#endif
