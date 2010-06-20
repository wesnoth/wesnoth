/*=============================================================================
    Copyright (c) 2001-2006 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying 
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(FUSION_AT_KEY_IMPL_05222005_0254)
#define FUSION_AT_KEY_IMPL_05222005_0254

#include <boost/fusion/support/detail/access.hpp>
#include <boost/type_traits/is_const.hpp>
#include <boost/mpl/identity.hpp>

namespace boost { namespace fusion
{
    struct map_tag;

    namespace extension
    {
        template <typename Tag>
        struct at_key_impl;

        template <>
        struct at_key_impl<map_tag>
        {
            template <typename Sequence, typename Key>
            struct apply 
            {
                typedef typename Sequence::template meta_at_impl<Key> element;
                
                typedef typename
                    mpl::eval_if<
                        is_const<Sequence>
                      , detail::cref_result<element>
                      , detail::ref_result<element>
                    >::type
                type;
    
                static type
                call(Sequence& m)
                {
                    return m.at_impl(mpl::identity<Key>());
                }
            };
        };
    }
}}

#endif
