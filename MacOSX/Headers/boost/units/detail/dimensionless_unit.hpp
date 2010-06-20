// Boost.Units - A C++ library for zero-overhead dimensional analysis and 
// unit/quantity manipulation and conversion
//
// Copyright (C) 2003-2008 Matthias Christian Schabel
// Copyright (C) 2007-2008 Steven Watanabe
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_UNITS_DETAIL_DIMENSIONLESS_UNIT_HPP
#define BOOST_UNITS_DETAIL_DIMENSIONLESS_UNIT_HPP

#include <boost/utility/enable_if.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/units/units_fwd.hpp>

namespace boost {
namespace units {

template<class T>
struct heterogeneous_system;

template<class T>
struct homogeneous_system;

template<class T1, class T2, class Scale>
struct heterogeneous_system_impl;

namespace detail {

template<class T>
struct is_dimensionless_system : boost::mpl::false_ {};

template<class T>
struct is_dimensionless_system<boost::units::homogeneous_system<T> > : boost::mpl::true_ {};

template<>
struct is_dimensionless_system<
   boost::units::heterogeneous_system<
       boost::units::heterogeneous_system_impl<
           boost::units::dimensionless_type,
           boost::units::dimensionless_type,
           boost::units::dimensionless_type
       >
   >
> : boost::mpl::true_ {};

#ifdef BOOST_MSVC

#define BOOST_UNITS_DIMENSIONLESS_UNIT(T)\
    boost::units::unit<\
        typename boost::enable_if<boost::units::detail::is_dimensionless_system<T>, boost::units::dimensionless_type>::type,\
        T\
    >

#define BOOST_UNITS_HETEROGENEOUS_DIMENSIONLESS_UNIT(T)\
    boost::units::unit<\
        typename boost::disable_if<boost::units::detail::is_dimensionless_system<T>, boost::units::dimensionless_type>::type,\
        T\
    >

#else

#define BOOST_UNITS_DIMENSIONLESS_UNIT(T)\
    boost::units::unit<\
        boost::units::dimensionless_type,\
        T,\
        typename boost::enable_if<boost::units::detail::is_dimensionless_system<T> >::type\
    >

#define BOOST_UNITS_HETEROGENEOUS_DIMENSIONLESS_UNIT(T)\
    boost::units::unit<\
        boost::units::dimensionless_type,\
        T,\
        typename boost::disable_if<boost::units::detail::is_dimensionless_system<T> >::type\
    >

#endif

}
}
}

#endif
