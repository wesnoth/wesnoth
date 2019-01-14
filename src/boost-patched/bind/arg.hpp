#ifdef __cplusplus

#include <boost/version.hpp>
#if (BOOST_VERSION >= 106000) && (BOOST_VERSION <= 106200)

#ifndef BOOST_BIND_ARG_HPP_INCLUDED
#define BOOST_BIND_ARG_HPP_INCLUDED

/*
** See https://svn.boost.org/trac/boost/ticket/12397
** Patch applied https://github.com/boostorg/bind/commit/3c56630b5400c43d1a4393d685a407e68a69ce9e
** GL 2016-10-12
*/

// MS compatible compilers support #pragma once

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

//
//  bind/arg.hpp
//
//  Copyright (c) 2002 Peter Dimov and Multi Media Ltd.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/libs/bind/bind.html for documentation.
//

#include <boost/config.hpp>
#include <boost/is_placeholder.hpp>

namespace boost
{

template< int I, int J > struct _arg_eq
{
};

template< int I > struct _arg_eq< I, I>
{
    typedef void type;
};

template< int I > struct arg
{
    BOOST_CONSTEXPR arg()
    {
    }

    template< class T > BOOST_CONSTEXPR arg( T const & /* t */, typename _arg_eq< I, is_placeholder<T>::value >::type * = 0 )
    {
    }
};

template< int I > BOOST_CONSTEXPR bool operator==( arg<I> const &, arg<I> const & )
{
    return true;
}

#if !defined( BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION )

template< int I > struct is_placeholder< arg<I>>
{
    enum _vt { value = I };
};

template< int I > struct is_placeholder< arg<I> (*) () >
{
    enum _vt { value = I };
};

#endif

} // namespace boost

#endif // #ifndef BOOST_BIND_ARG_HPP_INCLUDED

#endif // BOOST_VERSION checks

#endif // __cplusplus
