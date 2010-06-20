/*=============================================================================
    Copyright (c) 2001-2006 Joel de Guzman
    Copyright (c) 2005-2006 Dan Marsden

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_FUSION_CATEGORY_OF_IMPL_24122005_1731)
#define BOOST_FUSION_CATEGORY_OF_IMPL_24122005_1731

#include <boost/config/no_tr1/utility.hpp>

namespace boost { namespace fusion
{
    struct struct_tag;
    struct random_access_traversal_tag;

    namespace extension
    {
        template<typename T>
        struct category_of_impl;

        template<>
        struct category_of_impl<struct_tag>
        {
            template<typename T>
            struct apply
            {
                typedef random_access_traversal_tag type;
            };
        };
    }
}}

#endif
