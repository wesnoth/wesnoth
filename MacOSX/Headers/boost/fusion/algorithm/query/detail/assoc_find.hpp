/*=============================================================================
    Copyright (c) 2001-2006 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying 
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(FUSION_ASSOC_FIND_09242005_1133)
#define FUSION_ASSOC_FIND_09242005_1133

#include <boost/mpl/identity.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_const.hpp>

namespace boost { namespace fusion { namespace detail
{
    template <typename Sequence, typename Key>
    struct assoc_find
    {
        typedef typename 
            mpl::if_<
                is_const<Sequence>
              , typename Sequence::template meta_find_impl_const<Key>::type
              , typename Sequence::template meta_find_impl<Key>::type
            >::type 
        type;

        static type
        call(Sequence& s)
        {
            return s.find_impl(mpl::identity<Key>());
        }
    };
}}}

#endif
