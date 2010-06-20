//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_FUNCTOR_HOLDER_APR_01_2007_0917AM)
#define BOOST_SPIRIT_FUNCTOR_HOLDER_APR_01_2007_0917AM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/xpressive/proto/proto.hpp>

namespace boost { namespace spirit 
{
    ///////////////////////////////////////////////////////////////////////////
    template <typename T, typename Functor>
    struct functor_holder
    {
        typedef Functor functor_type;
        T held;
    };

    template <typename T, typename Functor>
    struct make_functor_holder
      : proto::terminal<functor_holder<T, Functor> >
    {
    };

}}

#endif
