/*=============================================================================
    Copyright (c) 2001-2006 Joel de Guzman
    Copyright (c) 2005-2006 Dan Marsden

    Distributed under the Boost Software License, Version 1.0. (See accompanying 
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_FUSION_IS_VIEW_IMPL_27042006_2219)
#define BOOST_FUSION_IS_VIEW_IMPL_27042006_2219

#include <boost/mpl/bool.hpp>

namespace boost { namespace fusion 
{
    struct std_pair_tag;

    namespace extension
    {
        template<typename Tag>
        struct is_view_impl;

        template<>
        struct is_view_impl<std_pair_tag>
        {
            template<typename T>
            struct apply : mpl::false_
            {};
        };
    }
}}

#endif
