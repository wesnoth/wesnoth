/*=============================================================================
    Copyright (c) 2001-2006 Joel de Guzman
    Copyright (c) 2005-2006 Dan Marsden

    Distributed under the Boost Software License, Version 1.0. (See accompanying 
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_FUSION_TAG_OF_24122005_1722)
#define BOOST_FUSION_TAG_OF_24122005_1722

#include <boost/fusion/support/tag_of_fwd.hpp>

#include <boost/config/no_tr1/utility.hpp>

namespace boost { namespace fusion {

    struct std_pair_tag;

    namespace traits
    {
        template<typename T1, typename T2>
        struct tag_of<std::pair<T1, T2> >
        {
            typedef std_pair_tag type;
        };
    }
}}

#endif
