//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_SUPPORT_META_FUNCTION_HOLDER_SEP_03_2007_0302PM)
#define BOOST_SPIRIT_SUPPORT_META_FUNCTION_HOLDER_SEP_03_2007_0302PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/spirit/home/support/unused.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace boost { namespace spirit 
{
    namespace detail
    {
        template <typename Functor, typename ParameterMF>
        struct make_function_holder_base
        {
            typedef typename mpl::if_<
                is_same<Functor, ParameterMF>, unused_type, ParameterMF
            >::type type;
        };
    }

    template <typename Functor, typename ParameterMF>
    struct meta_function_holder 
      : spirit::detail::make_function_holder_base<Functor, ParameterMF>::type
    {
    private:
        typedef typename
            spirit::detail::make_function_holder_base<Functor, ParameterMF>::type
        base_type;

    public:
        meta_function_holder()
        {}

        meta_function_holder(ParameterMF const& mf)
          : base_type(mf)
        {}
    };
    
}}

#endif
