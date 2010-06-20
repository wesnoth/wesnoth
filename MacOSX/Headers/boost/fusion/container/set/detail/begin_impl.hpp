/*=============================================================================
    Copyright (c) 2001-2006 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying 
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(FUSION_BEGIN_IMPL_09162005_1120)
#define FUSION_BEGIN_IMPL_09162005_1120

#include <boost/fusion/sequence/intrinsic/begin.hpp>
#include <boost/type_traits/is_const.hpp>
#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/identity.hpp>

namespace boost { namespace fusion
{
    struct set_tag;

    namespace extension
    {
        template <typename Tag>
        struct begin_impl;

        template <>
        struct begin_impl<set_tag>
        {
            template <typename Sequence>
            struct apply 
            {
                typedef typename 
                    result_of::begin<typename Sequence::storage_type>::type
                iterator_type;

                typedef typename 
                    result_of::begin<typename Sequence::storage_type const>::type
                const_iterator_type;

                typedef typename 
                    mpl::eval_if<
                        is_const<Sequence>
                      , mpl::identity<const_iterator_type>
                      , mpl::identity<iterator_type>
                    >::type
                type;
    
                static type
                call(Sequence& s)
                {
                    return fusion::begin(s.get_data());
                }
            };
        };
    }
}}

#endif
