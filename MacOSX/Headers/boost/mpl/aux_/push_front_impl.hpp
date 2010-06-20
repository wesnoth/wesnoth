
#ifndef BOOST_MPL_AUX_PUSH_FRONT_IMPL_HPP_INCLUDED
#define BOOST_MPL_AUX_PUSH_FRONT_IMPL_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2008
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: push_front_impl.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-11 02:19:02 -0400 (Sat, 11 Oct 2008) $
// $Revision: 49267 $

#include <boost/mpl/push_front_fwd.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/mpl/aux_/has_type.hpp>
#include <boost/mpl/aux_/traits_lambda_spec.hpp>
#include <boost/mpl/aux_/config/forwarding.hpp>
#include <boost/mpl/aux_/config/static_constant.hpp>

#include <boost/type_traits/is_same.hpp>

namespace boost { namespace mpl {

template< typename Tag >
struct has_push_front_impl;

// agurt 05/feb/04: no default implementation; the stub definition is needed 
// to enable the default 'has_push_front' implementation below

template< typename Tag >
struct push_front_impl
{
    template< typename Sequence, typename T > struct apply
    {
        // should be instantiated only in the context of 'has_push_front_impl';
        // if you've got an assert here, you are requesting a 'push_front' 
        // specialization that doesn't exist.
        BOOST_MPL_ASSERT_MSG(
              ( boost::is_same< T, has_push_front_impl<T> >::value )
            , REQUESTED_PUSH_FRONT_SPECIALIZATION_FOR_SEQUENCE_DOES_NOT_EXIST
            , ( Sequence )
            );
    };
};

template< typename Tag >
struct has_push_front_impl
{
    template< typename Seq > struct apply
#if !defined(BOOST_MPL_CFG_NO_NESTED_FORWARDING)
        : aux::has_type< push_front< Seq, has_push_front_impl<Tag> > >
    {
#else
    {
        typedef aux::has_type< push_front< Seq, has_push_front_impl<Tag> > > type;
        BOOST_STATIC_CONSTANT(bool, value = 
              (aux::has_type< push_front< Seq, has_push_front_impl<Tag> > >::value)
            );
#endif
    };
};

BOOST_MPL_ALGORITM_TRAITS_LAMBDA_SPEC(2, push_front_impl)
BOOST_MPL_ALGORITM_TRAITS_LAMBDA_SPEC(1, has_push_front_impl)

}}

#endif // BOOST_MPL_AUX_PUSH_FRONT_IMPL_HPP_INCLUDED
